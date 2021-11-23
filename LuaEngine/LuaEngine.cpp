#include <LuaEngine.h>
#include <type_traits>
#include <iostream>
extern "C" {
	#include <lua.h>
	#include <lauxlib.h>
	#include <lualib.h>
}


//Checks if result of lua expression is valid, asserts with error if it's not
bool LuaEngine::Check(int result) {
	if (result == LUA_OK) {
		return true;
	}
	else {
		try {
			if (lua_isstring(m_LuaState, -1)) {
				DebugPrint(lua_tostring(m_LuaState, -1));
			}
			else {
				return false;
			}

			return false;
		}
		catch (...) {
			Assert("Break"); 
			throw std::runtime_error("Error Message could not be accesed");
		}
	}
}

//Default Constructor, creates an empty function binding vector, initializes a new lua_State
LuaEngine::LuaEngine() : m_functions(std::make_unique<std::vector<Lua_Function_Binding>>())
{
	m_LuaState = luaL_newstate();
	luaL_openlibs(m_LuaState);
	DebugPrint("Stack is initialized at: " << lua_gettop(m_LuaState))
}

//Copy Constructor copies context name, creates a copy of internal function binding vector and Binds functions
//Note: lua State is still clean after copy if a script was run on the "ref" it needs to be ran on the copy manually.
LuaEngine::LuaEngine(const LuaEngine& ref) : m_ContextName(ref.m_ContextName)
{
	m_LuaState = luaL_newstate();
	luaL_openlibs(m_LuaState);
	DebugPrint("Stack is initialized at: " << lua_gettop(m_LuaState))
	m_functions = std::make_unique<std::vector<Lua_Function_Binding>>(*ref.m_functions);
	SetBindings(*m_functions);
}

//Copy assignment operator, closes the current lua state and clears function bindings, then does the same as copy constructor
LuaEngine& LuaEngine::operator=(const LuaEngine& ref)
{
	lua_close(m_LuaState);
	m_functions.reset();
	m_ContextName = ref.m_ContextName;
	m_functions = std::make_unique<std::vector<Lua_Function_Binding>>(*ref.m_functions);
	m_LuaState = luaL_newstate();
	luaL_openlibs(m_LuaState);
	DebugPrint("Stack is initialized at: " << lua_gettop(m_LuaState))
	SetBindings(*m_functions);
	return *this;
}
//Move Constructor copies ContextName, moves pointer to a lua state and pointer to function bindings, then clears ref to nullptr.
LuaEngine::LuaEngine(LuaEngine&& ref) noexcept : m_LuaState(ref.m_LuaState), m_functions(ref.m_functions.get()), m_ContextName(ref.m_ContextName)
{
	ref.m_LuaState = nullptr;
	ref.m_functions.release();
}

//Move assignment operator resets the current lua state and clears the function bindings, then does the same as Move constructor.
LuaEngine& LuaEngine::operator=(LuaEngine&& ref) noexcept
{
	lua_close(m_LuaState); // throw here could cause a crash !
	m_ContextName = ref.m_ContextName;
	m_LuaState = ref.m_LuaState;
	ref.m_LuaState = nullptr;
	m_functions.reset(ref.m_functions.get());
	ref.m_functions.release();
	return *this;
}

//Adds a singular function binding.
void LuaEngine::AddBinding(const LuaEngine_Function_Binding& binding)
{
	static Lua_Function_Binding termination = { NULL, NULL };
	if (!m_functions->empty() && ((*m_functions)[m_functions->size() - 1].func == NULL && (*m_functions)[m_functions->size() - 1].name == NULL)) {
		m_functions->pop_back();
	}
	int index = m_functions->size();
	m_functions->push_back({ binding.name,binding.function });
	SetBindings(*m_functions, index);

}

//Adds multiple function bindings.
void LuaEngine::AddBindings(const std::vector<LuaEngine_Function_Binding>& bindings)
{
	static Lua_Function_Binding termination = { NULL, NULL };
	if (!m_functions->empty() && ((*m_functions)[m_functions->size() - 1].func == NULL && (*m_functions)[m_functions->size() - 1].name == NULL)) {
		m_functions->pop_back();
	}
	int index = m_functions->size();
	for (auto& binding : bindings) {
		m_functions->push_back({ binding.name,binding.function });
	}

	SetBindings(*m_functions, index);

}
void LuaEngineProxy::Set_Field(const std::string& name, int index)
{
	lua_setfield(state, index, name.c_str());
}

void LuaEngineProxy::Get_Field(const std::string& name, int index)
{
	lua_getfield(state, index, name.c_str());
}

void LuaEngine::Create_Table(lua_State* L)
{
	lua_newtable(L);
}

void LuaEngine::RunString(const std::string& code)
{
	Check(luaL_dostring(m_LuaState, code.c_str()));
}

//Internal Call Creates or gets an existing Context table, the binds function from the binding vector from 
// start index. Used Internally by AddBindinds and Add Binding.
void LuaEngine::SetBindings(std::vector<Lua_Function_Binding>& bindings, int startindex) {
	static Lua_Function_Binding termination = { NULL, NULL };
	if (bindings.empty() || (bindings.size() - 1) < startindex) {
		DebugPrint("Invalid Binding array");
		Assert("Break"); 
		throw std::invalid_argument("Invalid binding array");
		return;
	}
	if (bindings[bindings.size() - 1].func != NULL || bindings[bindings.size() - 1].name != NULL) {
		bindings.push_back(termination);
 	}
	lua_getglobal(m_LuaState, m_ContextName);
	if (!lua_isnil(m_LuaState, -1) && lua_istable(m_LuaState, -1)) {
		luaL_setfuncs(m_LuaState, &bindings[startindex], 0);
		lua_pop(m_LuaState, 1);
	}
	else {
		lua_pop(m_LuaState, 1);
		lua_newtable(m_LuaState);
		luaL_setfuncs(m_LuaState, &bindings[startindex], 0);
		lua_setglobal(m_LuaState, m_ContextName);
	}
}

//Loads and runs a script from filepath, return true if succeeded false otherwise.
bool LuaEngine::LoadFromFile(const char* filepath)
{
	return Check(luaL_dofile(m_LuaState, filepath));
}

//LuaEngine destructor closes lua_State
LuaEngine::~LuaEngine()
{
	if (m_LuaState) {
		DebugPrint("Stack is destroyed at: " << lua_gettop(m_LuaState))
		lua_close(m_LuaState);
	}
}

//Pushes integer onto the stack
void LuaEngine::Set(lua_State* L, int number) {
	lua_pushinteger(L, number);
}

//pushes string onto the stack.
void LuaEngine::Set(lua_State* L, std::string str) {
	lua_pushstring(L, str.c_str());
}

//pushes float onto the stack.
void LuaEngine::Set(lua_State* L, float flt)
{
	lua_pushnumber(L, flt);
}

//Pushes object reference onto the stack as a table of void pointer and runtime type identifier
//Runtime type identifier can be used to maintain info on the type.
void LuaEngine::Set(lua_State* L, void* ptr, int id)
{
	lua_newtable(L);
	lua_pushinteger(L, id);
	lua_setfield(L, -2, "type_identifier");
	lua_pushlightuserdata(L, ptr);
	lua_setfield(L, -2, "ptr");
}

//static helper function for clearing lua stack, calls lua_pop internally.
void LuaEngine::clear_stack(lua_State *L,int num_elements)
{
	lua_pop(L, num_elements);
}

//static helper function for pushing function onto the stack and checking its validity. 
bool LuaEngine::LoadCall(LuaEngine* L, const char* func_name)
{
	lua_getglobal(L->GetState(), func_name);
	if (lua_isfunction(L->GetState(), -1)) {
		return true;
	}
	lua_pop(L->GetState(), 1);
	DebugPrint("Function with name " << func_name << " doesn't exist.");
	return false;

}

bool LuaEngine::LoadCall(LuaEngine* L, const char* table_name, const char* func_name)
{
	lua_getglobal(L->GetState(), table_name);
	if (lua_istable(L->GetState(), -1)) {
		
		lua_getfield(L->GetState(), -1,func_name);
		if (lua_isfunction(L->GetState(), -1)) {
			lua_getglobal(L->GetState(), table_name);
			return true;
		}
		lua_pop(L->GetState(), 1);
		DebugPrint("Function with name " << func_name << " doesn't exist.");
	}
	else {
		DebugPrint("Table with name " << table_name << " doesn't exist.");
	}
	return false;
}

//static helper function, for calling lua function, uses lua_pcall internally.
bool LuaEngine::Call_impl(LuaEngine* L, int args, int ret)
{
	return L->Check(lua_pcall(L->GetState(), args, ret, 0));
}

//static helper function, for getting pointer to lua_states extra space.
void* LuaEngine::GetExtraSpace(lua_State* L)
{
	return lua_getextraspace(L);
}

//static helper function, for getting integer from the stack.
void LuaEngine::Get(lua_State* L, int index, int* out) {
	if (lua_isinteger(L, index)) {
		*out = lua_tointeger(L, index);
		return;
	}
	Assert("Break"); 
	throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a number");

	*out = 0;
}

//static helper function, for getting string from the stack.
void LuaEngine::Get(lua_State* L, int index, std::string* str) {
	if (lua_isstring(L, index)) {
		*str = lua_tolstring(L, index, nullptr);
		return;
	}
	Assert("Break"); 
	throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a string");

	*str = std::string("Not Defined");
}

//static helper function, for getting float from the stack.
void LuaEngine::Get(lua_State* L, int index, float* out)
{
	if (lua_isnumber(L, index)) {
		*out = lua_tonumber(L, index);
		return;
	}
	Assert("Break"); 
	throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a float");

	*out = 0.0f;
}

//static helper function, for getting const char ptr from the stack.
//Note: lifetime of this string is bound to the lifetime on the stack, so it should be used scarcly.
void LuaEngine::Get(lua_State* L, int index, const char** str) {
	if (lua_isstring(L, index)) {
		*str = lua_tolstring(L, index, nullptr);
		return;
	}
	Assert("Break"); 
	throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a string");

	*str = nullptr;
}

//static helper function, for getting T ptr from the stack.
//Gets a table with a void pointer and a runtime identifier, then compares it with the runtime identifier of 
//the current type T, if types don't match an invalid argument exception is thrown.
//Only support type which can by runtime qualified(contain type_id_gen typedef) 
void LuaEngine::Get(lua_State* L, int index, void** ptr, int id)
{
	if (lua_istable(L,index)) {
		lua_pushstring(L, "type_identifier");
		lua_gettable(L, index);
		if (lua_isinteger(L, -1)) {
			if (lua_tointeger(L, -1) == id) {
				lua_pushstring(L, "ptr");
				lua_gettable(L, index);
				if (lua_islightuserdata(L, -1)) {
					*ptr = lua_touserdata(L, -1);
					return;
				}
			}
			else {
				Assert("Break");
				throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a pointer to a valid object.");
			}
		}
	}	
	Assert("Break");
	throw std::invalid_argument(std::string("Parameter ") + std::to_string(index) + " is not a Runtime identified pointer table.");
}

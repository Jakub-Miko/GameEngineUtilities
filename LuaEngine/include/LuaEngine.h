#include <string>
#include <tuple>
#include <memory>
#include <vector>
#include <TypeId.h>
#include <stdexcept>

// Debug code for assertions 
#ifndef NDEBUG
#include <iostream>

#ifndef Assert
#include <cassert>
#ifndef DISABLE_ASSERTS
#define Assert(msg) std::cout << msg << std::endl; assert(false);
#else 
#define Assert(msg)
#endif
#endif 

#ifndef Debug_Print
#define DebugPrint(msg) std::cout << msg << std::endl;
#endif
#else

#define Assert(msg)
#define DebugPrint(msg)

#endif

//Forward declarations in the case Engine wants to use lua_State directly.
struct lua_State;
struct luaL_Reg;

#pragma region BASE

//Default Class with support for pure functions.
//Supports easy function binding and calling.
class LuaEngine {
public:
	//Internal function binding format, its construction depends on Lua headers.
	using Lua_Function_Binding = luaL_Reg;

	//External function binding format, used when calling AddBindings and AddBinding without needing Lua headers.
	struct LuaEngine_Function_Binding {
		const char* name;
		int(*function)(lua_State*);
	};

public:
	
	//Default Constructor Creates Lua State and load libs.
	LuaEngine();

	//Copy Constructor copies context name, creates a copy of internal function binding vector and Binds functions
	//Note: lua State is still clean after copy if a script was run on the "ref" it needs to be ran on the copy manually.
	LuaEngine(const LuaEngine& ref);
	//Copy assignment operator, closes the current lua state and clears function bindings, then does the same as copy constructor
	LuaEngine& operator=(const LuaEngine& ref);

	//Move Constructor copies ContextName, moves pointer to a lua state and pointer to function bindings, then clears ref to nullptr.
	LuaEngine(LuaEngine&& ref) noexcept;

	//Move assignment operator resets the current lua state and clears the function bindings, then does the same as Move constructor.
	LuaEngine& operator=(LuaEngine&& ref) noexcept;
	
	//Gets internal lua_State for direct use, should be avoided to prevent coupling
	inline lua_State* GetState() const { return m_LuaState; }
	//Gets internal function bindings, should be avoided to prevent coupling, used in copy constriction and assignment.
	inline const std::vector<Lua_Function_Binding>& GetBindings() const { return *m_functions; }

	//Adds a singular function binding.
	void AddBinding(const LuaEngine_Function_Binding& binding);
	//Adds multiple function bindings.
	void AddBindings(const std::vector<LuaEngine_Function_Binding>& bindings);

	//Loads and runs a script from filepath, return true if succeeded false otherwise.
	bool LoadFromFile(const char* filepath);

	//template function for easy lua function calls from c++, pushes the function onto the stack, pushes arguments, 
	// calls functions, and pops and returns the return value if any. 
	//In the case of a failure while calling return default value and asserts.
	template<typename R = void, typename ... Args>
	R Call(const char* function_name, Args ... args) {
		if (LoadCall(this, function_name)) {
			
			(Set(m_LuaState, args),...); 
			
			if (Call_impl(this, sizeof...(args))) {
				
				if constexpr (!std::is_void_v<R>) {
					
					R ret_value;
					Get(m_LuaState, -1, &ret_value);
					clear_stack(m_LuaState, 1);
					return ret_value;

				} else {
					
					clear_stack(m_LuaState, 1);
					return;

				}
			}
			Assert("Break"); 
			throw std::runtime_error(std::string("Function ") + function_name + " failed.");
		}
		Assert("Break"); 
		throw std::invalid_argument(std::string("Function ") + function_name + " doesn't exist.");
	}

	//LuaEngine destructor closes lua_State
	~LuaEngine();

public:

	//Factory method for easy LuaEngineCreation
	static LuaEngine LoadEngineFromFile(
		const char* filepath,
		const std::vector<LuaEngine_Function_Binding>& bindings = std::vector<LuaEngine_Function_Binding>(),
		const char* BingingContextName = "Context")
	{
		LuaEngine engine;
		engine.m_ContextName = BingingContextName;
		engine.AddBindings(bindings);
		if (engine.LoadFromFile(filepath)) {
			return engine;
		}
		Assert("Break"); 
		throw std::invalid_argument("File could not be loaded");
	}


	//template function wrapper, for easy function binding, passes function pointer F to suitable Invoke_impl.
	template<auto F>
	static int Invoke(lua_State* L);

	//Simple Check for lua expression success return value, attemps to assert with error on failure.
	bool Check(int result);

protected:
	
	//Internal Call Creates or gets an existing Context table, the binds function from the binding vector from 
	// start index. Used Internally by AddBindinds and Add Binding.
	void SetBindings(std::vector<Lua_Function_Binding>& bindings, int startindex = 0);

	//static helper function for clearing lua stack, calls lua_pop internally.
	static void clear_stack(lua_State *L, int num_elements);

	//static helper function for pushing function onto the stack and checking its validity. 
	static bool LoadCall(LuaEngine* L, const char* func_name);

	//static helper function, for calling lua function, uses lua_pcall internally.
	static bool Call_impl(LuaEngine* L, int args, int ret = 1);

	//static helper function, for getting pointer to lua_states extra space.
	static void* GetExtraSpace(lua_State* L);

	//static helper function, for getting integer from the stack.
	static void Get(lua_State* L, int index, int* out);

	//static helper function, for getting string from the stack.
	static void Get(lua_State* L, int index, std::string* out);

	//static helper function, for getting float from the stack.
	static void Get(lua_State* L, int index, float* out);

	//static helper function, for getting const char ptr from the stack.
	//Note: lifetime of this string is bound to the lifetime on the stack, so it should be used scarcly.
	static void Get(lua_State* L, int index, const char** out);

	//This function is internally called by 'template<typename T> static void Get(lua_State*,int,T**)'
	static void Get(lua_State* L, int index, void** ptr, int id);
	
	//static helper function, for getting T ptr from the stack.
	//Gets a table with a void pointer and a runtime identifier, then compares it with the runtime identifier of 
	//the current type T, if types don't match an invalid argument exception is thrown.
	//Only support type which can by runtime qualified(contain type_id_gen typedef) 
	template<typename T, typename dummy = typename T::type_id_gen>
	static void Get(lua_State* L, int index, T** ptr) {
		Get(L, index, (void**)ptr, (int)TypeId<T>());
	}

	//Pushes integer onto the stack
	static void Set(lua_State* L, int number);

	//pushes string onto the stack.
	static void Set(lua_State* L, std::string str);
	
	//pushes float onto the stack.
	static void Set(lua_State* L, float flt);

	//pushes void ptr onto the stack.
	static void Set(lua_State* L, void* ptr, int id);

	//Pushes object reference onto the stack as a table of void pointer and runtime type identifier
	//Runtime type identifier can be used to maintain info on the type.
	template<typename T, typename dummy = typename T::type_id_gen>
	static void Set(lua_State* L, T* ref) {
		Set(L, (void*)ref, (int)TypeId<T>());
	}

	//recursive template which Iterates through the member types of an input tuple and sets its values from the stack,
	// used for fetching function parameters from the stack.
	template<int num = 0, typename First, typename ... Args, typename ... tupleargs>
	static void Invoke_Iterate(lua_State* L, std::tuple<tupleargs...>& tuple) {
		
		Get(L, num + 1, &std::get<num>(tuple));

		if constexpr ((num + 1) < sizeof...(tupleargs)) {

			Invoke_Iterate<num + 1, Args...>(L, tuple);

		}

		return;
	}

	//Invoke_impl used for calling parametrized pure functions from lua.
	template<typename R, typename ... Args>
	static int Invoke_impl(lua_State* L, R(*func)(Args...));

	//Invoke_impl used for calling non-parametrized pure functions from lua.
	template<typename R>
	static int Invoke_impl(lua_State* L, R(*func)());

private:
	//Internal Array of function bindings used mainly for purposes of copying.
	std::unique_ptr<std::vector<Lua_Function_Binding>> m_functions;

	//Internal lua_State
	lua_State* m_LuaState;
protected:
	//lua Context Table name, this is the table name uder which c++ functions are bound in lua.
	//Note: Needs to be owned by an object which has superset lifetime of LuaEngine. 
	const char* m_ContextName = "Context";
};

//Invoke_impl used for calling parametrized pure functions from lua.
template<typename R, typename ... Args>
int LuaEngine::Invoke_impl(lua_State* L, R(*func)(Args...)) {
	
	std::tuple<Args...> values;
	Invoke_Iterate<0, Args...>(L, values);

	if constexpr (!std::is_void_v<R>) {

		R result = std::apply(func, values);
		clear_stack(L, sizeof...(Args));
		Set(L, result);

	}
	else {

		std::apply(func, values);
		clear_stack(L, sizeof...(Args));

	}

	return 1;
}

//Invoke_impl used for calling non-parametrized pure functions from lua.
template<typename R>
int LuaEngine::Invoke_impl(lua_State* L, R(*func)()) {
	
	if constexpr (!std::is_void_v<R>) {

		R result = std::apply(func, std::tuple<>());
		Set(L, result);

	}
	else {

		std::apply(func, std::tuple<>());

	}

	return 1;
}

//template function wrapper, for easy function binding, passes function pointer F to suitable Invoke_impl.
template<auto F>
int LuaEngine::Invoke(lua_State* L) {
	return Invoke_impl(L, F);
}

#pragma endregion

//This functionality is not neccesary if you're not binding member functions.

#pragma region Extended


//template Extension of LuaEngine for binding member functions of a class,
// Note: members of only one class instance can be used for clarity and easy of implementation.
template<typename T>
class LuaEngineClass : public LuaEngine {
public:
	//type of the class owning the bound member functions.
	using class_type = T;
public:

	//Default constructor has no additional behavior
	LuaEngineClass() : LuaEngine() {

	}
	//Instance constructor sets the instance of the class
	LuaEngineClass(class_type* inst) : LuaEngine() {
		SetClassInstance(inst);
	}
	
	//copy constructor copies the instance of the class
	LuaEngineClass(const LuaEngineClass& copy) : LuaEngine(copy) {
		SetClassInstance(*static_cast<class_type**>(copy.GetExtraSpace(copy.GetState())));
	}

	//Move constructor copies the instance of the class implicitly when copying state, so no additional behavior is neccesary.
	LuaEngineClass(LuaEngineClass&& copy) : LuaEngine(std::move(copy)) {
		
	}

	//Copy assignment operator copies the instance of the class
	LuaEngineClass& operator=(const LuaEngineClass& copy) {
		
		LuaEngine::operator=(copy);
		SetClassInstance(*static_cast<class_type**>(copy.GetExtraSpace(copy.GetState())));
		return *this;

	}

	//Move assignment operator copies the instance of the class implicitly when copying state, so no additional behavior is neccesary.
	LuaEngineClass& operator=(LuaEngineClass&& copy) noexcept {
		
		LuaEngine::operator=(std::move(copy));
		return *this;

	}
	
	//Sets the instance of the class
	void SetClassInstance(class_type* instance) {
		
		*(static_cast<class_type**>(GetExtraSpace(GetState()))) = instance;

	}

	//Additional Invoke_impl, which is disambiguated as Invoke_impl_class and is called by InvokeClass,
	// used for non-const non-parametrized member functions
	template<typename R>
	static int Invoke_impl_class(lua_State* L, R(class_type::*func)()) {
		
		class_type* instance = *(static_cast<class_type**>(GetExtraSpace(L)));

		if constexpr (!std::is_void_v<R>) {

			R result = std::apply(func, std::tuple< class_type*>(instance));
			Set(L, result);

		}
		else {

			std::apply(func, std::tuple< class_type*>(instance));

		}
		return 1;
	}

	//Additional Invoke_impl, which is disambiguated as Invoke_impl_class and is called by InvokeClass,
	// used for non-const parametrized member functions
	template<typename R,typename ... Args>
	static int Invoke_impl_class(lua_State* L, R(class_type::*func)(Args...)) {
		class_type* instance = *(static_cast<class_type**>(GetExtraSpace(L)));
		std::tuple<Args...> values;
		Invoke_Iterate<0, Args...>(L, values);

		if constexpr (!std::is_void_v<R>) {

			R result = std::apply(func, std::tuple_cat(std::tuple< class_type*>(instance) , values));
			clear_stack(L, sizeof...(Args));
			Set(L, result);

		}
		else {

			std::apply(func, std::tuple_cat(std::tuple< class_type*>(instance), values));
			clear_stack(L, sizeof...(Args));

		}
		return 1;
	}

	//Additional Invoke_impl, which is disambiguated as Invoke_impl_class and is called by InvokeClass,
	// used for const non-parametrized member functions
	template<typename R>
	static int Invoke_impl_class(lua_State* L, R(class_type::* func)() const ) {
		class_type* instance = *(static_cast<class_type**>(GetExtraSpace(L)));

		if constexpr (!std::is_void_v<R>) {

			R result = std::apply(func, std::tuple< class_type*>(instance));
			Set(L, result);

		}
		else {

			std::apply(func, std::tuple< class_type*>(instance));

		}

		return 1;
	}

	//Additional Invoke_impl, which is disambiguated as Invoke_impl_class and is called by InvokeClass,
	// used for const parametrized member functions
	template<typename R, typename ... Args>
	static int Invoke_impl_class(lua_State* L, R(class_type::* func)(Args...) const) {

		class_type* instance = *(static_cast<class_type**>(GetExtraSpace(L)));
		std::tuple<Args...> values;
		Invoke_Iterate<0, Args...>(L, values);

		if constexpr (!std::is_void_v<R>) {

			R result = std::apply(func, std::tuple_cat(std::tuple< class_type*>(instance), values));
			clear_stack(L, sizeof...(Args));
			Set(L, result);

		}
		else {

			std::apply(func, std::tuple_cat(std::tuple< class_type*>(instance), values));
			clear_stack(L, sizeof...(Args));

		}

		return 1;
	}

	//Additional disambiguated version of Invoke, used for binding member functions.
	template<auto F>
	static int InvokeClass(lua_State* L) {
		return Invoke_impl_class(L, F);
	}

	static LuaEngineClass<class_type> LoadEngineClassFromFile(
		const char* filepath,
		class_type* instance = nullptr,
		const std::vector<LuaEngine_Function_Binding>& bindings = std::vector<LuaEngine_Function_Binding>(),
		const char* BingingContextName = "Context") 
	{
		LuaEngineClass<class_type> engine;
		engine.SetClassInstance(instance);
		engine.m_ContextName = BingingContextName;
		engine.AddBindings(bindings);
		if (engine.LoadFromFile(filepath)) {
			return engine;
		}
		Assert("Break"); 
		throw std::invalid_argument("File couldnt be loaded");
	}


};


#pragma endregion
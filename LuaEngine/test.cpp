#include <iostream>
#include <LuaEngine.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

int get_sum(int n1, int n2) {
	return n1 + n2;
}

float float_sum(float n1, float n2) {
	return n1 + n2;
}

std::string GetInput() {
	std::string input;
	std::getline(std::cin,input);
	return input;
}

void RecieveMessage(const char* msg) {
	std::cout << "Recieved Message: " << msg << std::endl;
}

void Test1(int argc, char* argv[]) {
	const char* filepath = "C:/Users/mainm/Desktop/GameEngine/Utility/LuaEngine/Script.lua";
	if (argc > 1 && argv[1]) {
		filepath = argv[1];
	}

	std::vector<LuaEngine::LuaEngine_Function_Binding> bindings = {
		{"CSum", LuaEngine::Invoke<get_sum>},
		{"SendMessage", LuaEngine::Invoke<RecieveMessage> },
		{"GetInput", LuaEngine::Invoke<GetInput> }
	};
	LuaEngine engine2 = LuaEngine::LoadEngineFromFile(filepath, bindings);

	LuaEngine engine1 = engine2;
	engine1.LoadFromFile(filepath);

	LuaEngine engine = std::move(engine1);

	std::cout << engine.Call<int>("add", 5, 3) << std::endl;
	std::cout << engine.Call<std::string>("concatenate", "5", "3") << std::endl;
}

class Entity {
private:
	int x, y;
public:
	using type_id_gen = SequentialIdGenerator;
	Entity(int x, int y) : x(x), y(y) {};
	Entity() : x(0),y(0) {}

	void Set_y(int y) {
		this->y = y;
	}

	void Set_x(int x) {
		this->x = x;
	}

	int Get_x() const {
		return x;
	}

	int Get_y() const {
		return y;
	}

	void copy_values(Entity* other) {
		x = other->x;
		y = other->y;
	}

};

template<>
class LuaEngineObjectDelegate<Entity> {
public:
	
	static void SetObject(LuaEngineProxy proxy, const Entity& value) {
		proxy.SetTableItem(value.Get_x(), "x");
		proxy.SetTableItem(value.Get_y(), "y");
	}

	static Entity GetObject(LuaEngineProxy proxy, int index = -1) {
		return Entity(proxy.GetTableField<int>("x", index), proxy.GetTableField<int>("y", index));
	}
};


class Entity2 {
private:
	int x, y;
public:
	using type_id_gen = SequentialIdGenerator;
	Entity2(int x, int y) : x(x), y(y) {};

	void Set_y(int y) {
		this->y = y;
	}

	void Set_x(int x) {
		this->x = x;
	}

	int Get_x() const {
		return x;
	}

	int Get_y() const {
		return y;
	}

	void copy_values(Entity2* other) {
		x = other->x;
		y = other->y;
	}

};

int main(int argc, char* argv[]) {
	{
		const char* filepath = "C:/Users/mainm/Desktop/GameEngine/Utility/LuaEngine/Script.lua";
		if (argc > 1 && argv[1]) {
			filepath = argv[1];
		}

		try {
			Entity entity_1(11554, 6);
			Entity second(1, 1);
			std::vector<int> asdas = { 5,54,4,6,415,5 };
			LuaEngineClass<Entity> engine = LuaEngineClass<Entity>::LoadEngineClassFromFile(filepath, &entity_1, {
				{"CSum", LuaEngine::Invoke<get_sum>},
				{"FloatSum", LuaEngine::Invoke<float_sum>},
				{"SendMessage", LuaEngine::Invoke<RecieveMessage> },
				{"GetInput", LuaEngine::Invoke<GetInput> },
				{"GetX" , LuaEngineClass<Entity>::InvokeClass<&Entity::Get_x> },
				{"GetY" , LuaEngineClass<Entity>::InvokeClass<&Entity::Get_y> },
				{"SetX" , LuaEngineClass<Entity>::InvokeClass<&Entity::Set_x> },
				{"SetY" , LuaEngineClass<Entity>::InvokeClass<&Entity::Set_y> },
				{"copy_values", LuaEngineClass<Entity>::InvokeClass<&Entity::copy_values>}
				}, "_G");
			std::cout << "Float is: " << engine.Call<float>("flt_sm", 5, 1.6f) << "\n";
			engine.Call("Test", &second);
			std::cout << entity_1.Get_x() << ", " << entity_1.Get_y() << ", " << sizeof(engine) << "\n\n\n";
			std::cout << engine.CallObject<int>("table1", "OnUpdate", 5, 6) << "\n";
			engine.CallObject("table2", "OnUpdate");

			int out;
			if (engine.TryCallObject<int>(&out,"table1", "OnUpdate", 5, 6))
			{
				std::cout << out << " Success\n";
			}
			else {
				std::cout << "Error\n";
			}

			if (engine.TryCallObject<void>(nullptr,"table1", "OnUpdate"))
			{
				std::cout << "Success\n";
			}
			else {
				std::cout << "Error\n";
			}

			std::cout << "-----------------------------------------------------------------\n";

			Entity ent_test = engine.Call<Entity>("ObjectTest", Entity(888, 91));
			std::cout << ent_test.Get_x() << ", " << ent_test.Get_y() << "\n";

		}
		catch (std::invalid_argument& e) {
			std::cout << "\nInvalid argument: " << e.what() << "\n";
		}
		catch (std::runtime_error& e) {
			std::cout << "\nRuntime Error: " << e.what() << "\n";
		}
		catch (...) {
			std::cout << "\nAn Error occured\n";
		}
	}
		_CrtDumpMemoryLeaks();
	void();
}

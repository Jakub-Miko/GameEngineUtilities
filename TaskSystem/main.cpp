#include <iostream> 
#include <TaskSystem.h>
#include <Profiler.h>
#include <tuple>

void TaskTest(int y) {
	PROFILE("asdasd");
	int asd = 0;
	for (int x = 0; x < 5000; x++) {
		asd += x*y;
		asd /= 2;
		asd %= 10;
	}
}


int main() {
	BEGIN_PROFILING("asdasd", "C:/Users/mainm/Desktop/GameEngine/Utility/TaskSystem/Profile_Result.json");

	int a = 5;

	
	//TaskDefinition* task2 = MakeTask([]() {TaskTest(1); });
	//TaskDefinition* task3 = MakeTask([a]() {TaskTest(a); });





	TaskSystem* system = new TaskSystem(TaskSystemProps{ 0 });
	system->Initialize();
	system->Run();

	int y = 8;

	
	for (int i = 0; i < 600; i++) {
		system->Submit([a]() {TaskTest(a); });
	}

	std::cin.get();
	delete system;
	std::cout << "End\n";
	END_PROFILING();

}
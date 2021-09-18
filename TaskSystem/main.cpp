#include <iostream> 
#include <TaskSystem.h>
#include <Profiler.h>

void TaskTest() {
	
	int asd = 0;
	for (int x = 0; x < 5000; x++) {
		asd += x;
		asd /= 2;
		asd %= 10;
	}
}


int main() {
	BEGIN_PROFILING("asdasd", "C:/Users/mainm/Desktop/GameEngine/Utility/TaskSystem/Profile_Result.json");

	TaskSystem* system = new TaskSystem(TaskSystemProps{ 0 });
	system->Initialize();
	system->Run();

	TaskDefinition task1(TaskTest);
	
	for (int i = 0; i < 600; i++) {
		system->Submit(task1);
	}

	std::cin.get();
	delete system;
	std::cout << "End\n";
	END_PROFILING();

}
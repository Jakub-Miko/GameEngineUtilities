#include <iostream> 
#include <TaskSystem.h>

#include <Profiler.h>
#include <tuple>
#include <thread>
#include <chrono>
#include <random>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void TaskTest(int milisecs) {
	PROFILE("asdasd");
}

void Sync() {
	std::cout << "Done\n";
}


int main() {
	{
		std::srand(521);

		BEGIN_PROFILING("asdasd", "C:/Users/mainm/Desktop/GameEngine/Utility/TaskSystem/Profile_Result.json");
		{
			PROFILE("main");
			int a = 5;

			std::atomic<int>* counter = new std::atomic<int>(0);

			PROFILE("TaskSystem Start");
			TaskSystem::Initialize();

			TaskSystem* system = TaskSystem::Get();
			system->Initialize(TaskSystemProps{ 1 });
			system->Run();
			std::vector<std::shared_ptr<TaskDefinition>> m_funcs;
			m_funcs.reserve(1200);
			PROFILE("TaskSystem Running");
			int y = 8;
			int step = 3;
			int modulo = 50;
			int bias = 10;
			int num = 0;
			{
				int time;
				PROFILE("TaskcCreation");
				for (int y = 0; y < 100; y++) {
					for (int i = 0; i < 10; i++) {
						time = ((num++ * step) % modulo) + bias;
						m_funcs.push_back(system->CreateTask([time]() {TaskTest(time); }));
					}
					m_funcs.push_back(system->CreateTask(Sync));
				}
			}
			{
				PROFILE("TaskSubmittion");
				system->Submit(m_funcs);
			}
			auto Task = system->CreateTask([](int i, int y) { std::cout << y << ", " << i << "\n"; },5,6);

			auto tasks = std::vector<std::shared_ptr<TaskDefinition>>{
				system->CreateTask([](int s) {std::cout << s << "\n"; },5),
				system->CreateTask([](int s) {std::cout << s << "\n"; },2),
				system->CreateTask([](int s) {std::cout << s << "\n"; },1),
				system->CreateTask([](int s) {std::cout << s << "\n"; },6),
				system->CreateTask([](int s) {std::cout << s << "\n"; },4)
			};

			system->Submit(tasks);
			PROFILE("TaskSystem Submitted");

			std::cin.get();
			delete counter;
			std::cout << "End\n";
		}
		TaskSystem::Shutdown();
		END_PROFILING();
	}
	_CrtDumpMemoryLeaks();
}
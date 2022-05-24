#include <iostream> 
#include <TaskSystem.h>
#include <Profiler.h>
#include <TaskSystemFence.h>
#include <tuple>
#include <thread>
#include <chrono>
#include <random>
#include <array>
#include "AsyncTaskDispatcher.h"
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

void SecondTask() {
	PROFILE("asdasd");
	std::this_thread::sleep_for(std::chrono::milliseconds(2));
}

void TaskTest(int milisecs) {
	PROFILE("submission task");
	TaskSystem* system = TaskSystem::Get();
	int y = 8;
	int step = 3;
	int modulo = 50;
	int bias = 10;
	int num = 0;
	int time;
	std::vector<std::shared_ptr<TaskDefinition>> m_funcs;
	m_funcs.reserve(200);

	for (int i = 0; i < 100; i++) {
		time = ((num++ * step) % modulo) + bias;
		m_funcs.push_back(system->CreateTask([]() {SecondTask(); }));
	}
	system->Submit(m_funcs);
}


void Sync() {
	std::cout << "Done\n";
}


int main() {
	{
		std::srand(521);

		
		{
			PROFILE("TaskSystem Start");
			ThreadManager::Init();
			TaskSystem::Initialize(TaskSystemProps{ 0 });

			TaskSystem* system = TaskSystem::Get();
			system->Run();

			auto task_return = system->CreateTask<int>([](int a) { PROFILE("RETURN"); return 2 * a; }, 5);
			system->Submit(task_return);
			auto return_value = task_return->GetFuture().GetValue();
			std::cout << return_value << "\n";


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
				PROFILE("TaskCreation");
				for (int y = 0; y < 100; y++) {
					m_funcs.push_back(system->CreateTask([time]() {TaskTest(time); }));
					m_funcs.push_back(system->CreateTask(Sync));
				}
			}
			{
				PROFILE("TaskSubmittion");
				system->Submit(m_funcs);
			}

			TaskSystemFence fence;
			auto task = [&fence, &system]() {
				fence.Signal(1); system->FlushLoop(); 
			};
			system->SetIdleTask(system->CreateTask(task));
			system->JoinTaskSystem([&fence]() -> bool {
				return fence.IsValue(1); 
				});

			
			PROFILE("FLUSH");
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
			std::cout << "End\n";
		}
		TaskSystem::Shutdown();
		ThreadManager::Shutdown();
	}
	_CrtDumpMemoryLeaks();
}
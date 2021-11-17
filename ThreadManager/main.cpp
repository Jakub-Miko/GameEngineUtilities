#include <iostream>
#include <ThreadManager.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>



void Function(int int1, int int2) {
	std::cout << "     " << int1 << int2 << "\n";
	std::cout << "     " << ThreadManager::GetCurrentThread()->GetId() << "\n";
}


int main() {
	{
		ThreadManager::Init();

		auto manager = ThreadManager::Get();

		std::cout << manager->GetMaxThreadCount() << "\n" << manager->GetAvailableThreadCount() << "\n";
		
		auto thread1 = manager->GetThread();
		thread1->RunThread(Function, 5, 6);
		std::cout << manager->GetAvailableThreadCount() << "\n";

		{
			auto thread1 = manager->GetThread();
			auto thread2 = manager->GetThread();
			auto thread3 = manager->GetThread();
			std::cout << manager->GetAvailableThreadCount() << "\n";
		}
		std::cout << manager->GetAvailableThreadCount() << "\n";


		std::cout << manager->GetAvailableThreadCount() << "\n";
		thread1->JoinThread();
		thread1.reset();
		ThreadManager::Shutdown();
	}
	_CrtDumpMemoryLeaks();
}
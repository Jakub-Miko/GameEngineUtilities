#include <iostream>
#include <ThreadManager.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>


int main() {
	{
		ThreadManager::Init();

		auto manager = ThreadManager::GetThreadManager();

		std::cout << manager->GetMaxThreadCount() << "\n" << manager->GetAvailableThreadCount() << "\n";
		
		auto thread1 = manager->GetThread();
		std::cout << manager->GetAvailableThreadCount() << "\n";

		{
			auto thread1 = manager->GetThread();
			auto thread2 = manager->GetThread();
			auto thread3 = manager->GetThread();
			std::cout << manager->GetAvailableThreadCount() << "\n";
		}
		std::cout << manager->GetAvailableThreadCount() << "\n";

		thread1.reset();

		std::cout << manager->GetAvailableThreadCount() << "\n";
		ThreadManager::Shutdown();
	}
	_CrtDumpMemoryLeaks();
}
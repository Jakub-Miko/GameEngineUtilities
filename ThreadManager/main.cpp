#include <iostream>
#include <ThreadManager.h>

#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

struct Entity {
	int x, y;
};

void Function(int int1, int int2) {
	std::cout << "     " << int1 << int2 << "\n";
	bool exists = ThreadManager::ThreadLocalDataExists<int>();
	if (!exists) {
		int* ptr = new int(5);
		ThreadManager::SetThreadLocalData<int>(ptr);
	}

	if (ThreadManager::ThreadLocalDataExists<int>()) {
		std::cout << "Exists" << "\n";
	}
	ThreadManager::SetThreadLocalData<Entity>(new Entity{ 5,10 });

	auto ent = ThreadManager::GetThreadLocalData<Entity>();
	std::cout << "------> " << *(ThreadManager::GetThreadLocalData<int>()) << "\n";
	std::cout << "------> " << ent->x << ", " << ent->y << "\n";
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
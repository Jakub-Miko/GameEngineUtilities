#include <TaskSystem.h>
#include <thread>
#include <vector>
#include <Profiler.h>
#include <memory>

TaskSystem* TaskSystem::instance = nullptr;	

TaskSystem::TaskSystem(TaskSystemProps props) : m_Props(props) {
	m_Queue.reset(new TaskQueue);
	m_Pool = new SynchronizedMultiPool<std::allocator<void>,true>(std::allocator<void>(), 32768);

	if (!m_Props.num_of_threads) { 
		m_Props.num_of_threads = std::thread::hardware_concurrency() - 1;
	}

	m_Threads = std::vector<std::thread*>(m_Props.num_of_threads);
}

void TaskSystem::Run()
{
	PROFILE("ThreadLaunch");
	for (auto& thread : m_Threads) {
		thread = new std::thread(ThreadLoop,m_Queue.get(),&m_runnning,m_Pool);
	}
}

void TaskSystem::Flush()
{
	for (auto& thread : m_Threads) {
		Submit(CreateTask([]() {}));
	}
	m_Queue->Flush();

}

TaskSystem::~TaskSystem()
{
	m_runnning = false;
	Flush();
	for (auto thread : m_Threads) {
		thread->join();
		delete thread;
	}
	m_Queue->Clear();
	delete m_Pool;
}


void TaskSystem::ThreadLoop(TaskQueue* queue, std::atomic<bool>* run,const std::pmr::memory_resource* pool) {
	bool running = true;
	while (running && run->load()) {
		PROFILE("Load");
		std::shared_ptr<TaskDefinition> task = queue->Pop();
		if (task) {
			task->Run();
		}
	}
}

void TaskSystem::DeleteTask(TaskDefinition* task,size_t size, size_t align)
{
	m_Pool->deallocate(task,size,align);
}


TaskSystem* TaskSystem::Get()
{
	return instance;
}

void TaskSystem::Initialize(TaskSystemProps props)
{
	PROFILE("Initialization");	
	if (!instance) {

		instance = new TaskSystem(props);

	}
}

void TaskSystem::Shutdown()
{

	if (instance) {
		delete instance;
	}

}


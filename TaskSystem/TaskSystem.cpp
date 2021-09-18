#include <TaskSystem.h>
#include <thread>
#include <vector>

void TaskSystem::Run()
{
	for (auto& thread : m_Threads) {
		thread = new std::thread(ThreadLoop,m_Queue.get(),&m_runnning);
	}
}

void TaskSystem::Flush()
{

}

TaskSystem::~TaskSystem()
{
	m_runnning = false;
	for (auto thread : m_Threads) {
		thread->join();
	}
}


void TaskSystem::ThreadLoop(TaskQueue* queue, std::atomic<bool>* run) {
	bool running = true;
	while (running && run->load()) {
		TaskDefinition* task = queue->Pop();
		if (task) {
			task->Run();
		}
	}
}


void TaskSystem::Initialize()
{
	m_Queue.reset(new TaskQueue);

	if (!m_Props.num_of_threads) {
		m_Props.num_of_threads = std::thread::hardware_concurrency() - 1;
	}

	m_Threads = std::vector<std::thread*>(m_Props.num_of_threads);


}


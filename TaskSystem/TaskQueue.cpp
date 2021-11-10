#include <TaskQueue.h>
#include <Profiler.h>

TaskQueue::TaskQueue(int num_of_threads)
	: m_Queue(), m_QueueMutex(), runnnig_threads(num_of_threads)
{
	
}

void TaskQueue::Push(std::shared_ptr<TaskDefinition> def)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_Queue.push(def);
	on_push.notify_one();
	lock.unlock();
}

std::shared_ptr<TaskDefinition> TaskQueue::Pop()
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	if (m_Queue.empty()) {
		--runnnig_threads;
		if (runnnig_threads == 0 && m_IdleTask) {
			PROFILE("IDLE");
			m_IdleTask->Run();
			m_IdleTask.reset();
		}
		on_push.wait(lock, [this]() {return !m_Queue.empty(); });
		++runnnig_threads;
	}

	std::shared_ptr<TaskDefinition> task = m_Queue.front();
	m_Queue.pop();
	return task;
}

void TaskQueue::SetIdleTask(std::shared_ptr<TaskDefinition> task)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_IdleTask = task;
}

void TaskQueue::Flush()
{
	on_push.notify_all();
}

void TaskQueue::Clear()
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	std::queue<std::shared_ptr<TaskDefinition>> empty;
	m_Queue.swap(empty);
}

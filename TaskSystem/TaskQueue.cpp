#include <TaskQueue.h>

TaskQueue::TaskQueue() 
	: m_Queue(), m_QueueMutex() {
	
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
		on_push.wait(lock, [this]() {return !m_Queue.empty(); });
	}

	std::shared_ptr<TaskDefinition> task = m_Queue.front();
	m_Queue.pop();
	return task;
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

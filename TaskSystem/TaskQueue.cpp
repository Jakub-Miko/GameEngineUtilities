#include <TaskQueue.h>

TaskQueue::TaskQueue() 
	: m_Queue(), m_QueueMutex() {
	
}

void TaskQueue::Push(TaskDefinition def)
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	m_Queue.push(def);
}

TaskDefinition* TaskQueue::Pop()
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);	
	if (m_Queue.empty())
		return nullptr;

	TaskDefinition* task = &m_Queue.front();
	m_Queue.pop();
	return task;
}

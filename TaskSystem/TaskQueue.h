#pragma once
#include <queue>
#include <Task.h>
#include <mutex>

class TaskQueue {
public:
	
	TaskQueue();

	void Push(TaskDefinition def);

	TaskDefinition* Pop();
	
private:
	std::queue<TaskDefinition> m_Queue;
	std::mutex m_QueueMutex;
};
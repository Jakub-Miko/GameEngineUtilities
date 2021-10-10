#pragma once
#include <queue>
#include <Task.h>
#include <mutex>
#include <condition_variable>

class TaskQueue {
public:
	
	TaskQueue();

	void Push(std::shared_ptr<TaskDefinition> def);

	std::shared_ptr<TaskDefinition> Pop();
	
	void Flush();

	void Clear();

private:
	std::queue<std::shared_ptr<TaskDefinition>> m_Queue;
	std::mutex m_QueueMutex;
	std::condition_variable on_push;
};
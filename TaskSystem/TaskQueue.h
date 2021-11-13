#pragma once
#include <queue>
#include <Task.h>
#include <mutex>
#include <condition_variable>

class TaskQueue {
public:
	
	TaskQueue(int num_of_threads);

	void Push(std::shared_ptr<TaskDefinition> def);

	std::shared_ptr<TaskDefinition> Pop();

	std::shared_ptr<TaskDefinition> PopExternal();

	void SetIdleTask(std::shared_ptr<TaskDefinition> task);
	
	void Flush();

	void Clear();

private:
	std::queue<std::shared_ptr<TaskDefinition>> m_Queue;
	std::mutex m_QueueMutex;
	std::condition_variable on_push;

	int runnnig_threads = 0;
	std::shared_ptr<TaskDefinition> m_IdleTask;

};
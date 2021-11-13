#pragma once
#include <queue>
#include <Task.h>
#include <mutex>
#include <condition_variable>

class TaskQueue {
public:
	
	TaskQueue(int num_of_threads);

	void Push(std::shared_ptr<TaskDefinition> def);

	std::shared_ptr<TaskDefinition> Pop(uint32_t& sync_var, uint32_t& reference_sync_var);

	void RegisterThread();

	void UnRegisterThread();

	void SetIdleTask(std::shared_ptr<TaskDefinition> task);

	void Flush(uint32_t& ref_var);

	void Clear();

private:
	std::queue<std::shared_ptr<TaskDefinition>> m_Queue;
	std::mutex m_QueueMutex;
	std::condition_variable on_push;

	int registered_threads = 0;
	int runnnig_threads = 0;
	std::shared_ptr<TaskDefinition> m_IdleTask;

};            
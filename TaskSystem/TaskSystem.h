#pragma once
#include <Task.h>
#include <TaskQueue.h>
#include <memory>
#include <atomic>

struct TaskSystemProps {
	int num_of_threads = 0;
};


class TaskSystem {
public:

	TaskSystem(TaskSystemProps props = TaskSystemProps()) : m_Props(props) { };

	void Initialize();

	static void ThreadLoop(TaskQueue* queue, std::atomic<bool>* run);

	void Submit(TaskDefinition task);

	void Run();

	~TaskSystem();

private:
	std::atomic<bool> m_runnning = true;
	std::unique_ptr<TaskQueue> m_Queue;
	std::vector<std::thread*> m_Threads;
	TaskSystemProps m_Props;
};
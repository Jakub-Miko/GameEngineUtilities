#pragma once
#include <Task.h>
#include <TaskQueue.h>
#include <memory>
#include <SynchronizedMultiPool.h>
#include <atomic>
#include <memory_resource>
#include <thread>

struct TaskSystemProps {
	int num_of_threads = 0;
};

class TaskSystem {
public:

	TaskSystem() = delete;
	
	TaskSystem(const TaskSystem& ref) = delete;
	TaskSystem(TaskSystem&& ref) = delete;

	TaskSystem& operator=(const TaskSystem& ref) = delete;
	TaskSystem& operator=(TaskSystem&& ref) = delete;

	static TaskSystem* Get();

	static void Initialize(TaskSystemProps props = TaskSystemProps());

	static void Shutdown();

	static void ThreadLoop(TaskQueue* queue, std::atomic<bool>* run, const std::pmr::memory_resource* pool);

	void DeleteTask(TaskDefinition* task, size_t size, size_t align = alignof(max_align_t));

	void Submit(std::shared_ptr<TaskDefinition> task) {
		m_Queue->Push(task);
	}

	void Submit(std::vector<std::shared_ptr<TaskDefinition>>& tasks) {
		for (auto task : tasks) {
			m_Queue->Push(task);
		}
	}

	template<typename T, typename ... Args>
	void Submit(T task, Args ... args)
	{
		Submit(CreateTask(task, args...));
	}


	template<typename T, typename ... Args>
	std::shared_ptr<TaskDefinition> CreateTask(T task, Args ... args)
	{
		std::pmr::polymorphic_allocator<Task<T,Args...>> alloc(m_Pool);
		return std::shared_ptr<TaskDefinition>(MakeTask(task, std::tuple<Args...>(args...), alloc), [](TaskDefinition* ptr) {
			TaskSystem::Get()->DeleteTask(ptr,sizeof(Task<T, Args...>),alignof(Task<T, Args...>));
		});
	}

	void Run();

	void Flush();

	~TaskSystem();



private:

	static TaskSystem* instance;
	TaskSystem(TaskSystemProps props);

	std::atomic<bool> m_runnning = true;
	std::unique_ptr<TaskQueue> m_Queue;
	std::vector<std::thread*> m_Threads;
	SynchronizedMultiPool<std::allocator<void>>* m_Pool;
	TaskSystemProps m_Props;
};
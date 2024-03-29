#pragma once
#include "Task.h"
#include <ThreadManager.h>
#include "TaskQueue.h"
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

	static void ThreadLoop(TaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool);

	void DeleteTask(TaskDefinition* task, size_t size, size_t align = alignof(max_align_t));

	template<typename R, typename T, typename ... Args>
	void Submit(std::shared_ptr<Task<R, T, Args...>> task) {
		m_Queue->Push(task);
	}

	void SetIdleTask(std::shared_ptr<TaskDefinition> task);

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

	const TaskSystemProps& GetProps() const {
		return m_Props;
	}

	template<typename R = void,typename T, typename ... Args>
	std::shared_ptr<Task<R, T, Args...>> CreateTask(T task, Args ... args)
	{
		std::pmr::polymorphic_allocator<Task<R,T,Args...>> alloc(m_Pool);
		return std::shared_ptr<Task<R, T, Args...>>(MakeTask<R>(task, std::tuple<Args...>(args...), alloc), [](TaskDefinition* ptr) {
			TaskSystem::Get()->DeleteTask(ptr,sizeof(Task<R,T, Args...>),alignof(Task<R,T, Args...>));
		});
	}

	void FlushDeallocations();

	template<typename F>
	auto JoinTaskSystem(F exit_condition_function) -> std::enable_if_t<std::is_same_v<std::invoke_result_t<F>,bool>,void>
	{
		JoinedThreadLoopInit();
		uint32_t sync = GetSyncNum();
		uint32_t& ref = GetSyncNum();
		while(!exit_condition_function() && m_runnning.load())
		{
			JoinedThreadLoopIteration(sync, ref);
		}
		JoinedThreadLoopShutdown();
	}

	template<typename F>
	void Run(F func)
	{
		for (auto& thread : m_Threads) {
			thread->RunThread(&StartThreadLoop<F>, func, m_Queue.get(), &m_runnning, m_Pool);
		}
	}

	template<typename F,typename FO>
	void Run(F func,FO func_out)
	{
		for (auto& thread : m_Threads) {
			thread->RunThread(&StartThreadLoop<F,FO>, func, func_out, m_Queue.get(), &m_runnning, m_Pool);
		}
	}

	void Run()
	{
		for (auto& thread : m_Threads) {
			thread->RunThread(&ThreadLoop, m_Queue.get(), &m_runnning, m_Pool);
		}
	}

	//Carefull flush happens only for task submitted before flush, if worker running workerthreads submit work after flush, a significant slowdown can occur, 
	//this can be avoided by submitting task from a single thread.
	//FLUSH CAN ONLY BE CALLED BY MAINTHREAD, OTHWERWISE IT'S UNDEFINED BEHAVIOR
	void Flush();

	void FlushLoop();

	~TaskSystem();



private:
	
	uint32_t& GetSyncNum() { return reference_sync_var; };

	static TaskSystem* instance;
	TaskSystem(TaskSystemProps props);

	void JoinedThreadLoopIteration(uint32_t& sync,uint32_t& ref);
	void JoinedThreadLoopInit();
	void JoinedThreadLoopShutdown();

	template<typename F>
	static void StartThreadLoop(F func, TaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool) {
		func();
		ThreadLoop(queue, run, pool);
	}

	template<typename F,typename FO>
	static void StartThreadLoop(F func_in, FO func_out, TaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool) {
		func_in();
		ThreadLoop(queue, run, pool);
		func_out();
	}

	std::atomic<bool> m_runnning = true;
	std::unique_ptr<TaskQueue> m_Queue;
	std::vector<std::shared_ptr<ThreadObject>> m_Threads;
	SynchronizedMultiPool<std::allocator<void>,true>* m_Pool;
	TaskSystemProps m_Props;

	uint32_t reference_sync_var = 0;

	std::mutex global_flush_mutex;
	std::mutex flush_mut;
	bool signaled;
	std::condition_variable flush_cond;

};
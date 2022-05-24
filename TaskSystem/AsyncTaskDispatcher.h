#pragma once 
#include <TaskSystem.h>
#include <SingleThreadedTaskQueue.h>

class AsyncTaskDispatcher {
public:

	AsyncTaskDispatcher();

	AsyncTaskDispatcher(const AsyncTaskDispatcher& ref) = delete;
	AsyncTaskDispatcher(AsyncTaskDispatcher&& ref) = delete;

	AsyncTaskDispatcher& operator=(const AsyncTaskDispatcher& ref) = delete;
	AsyncTaskDispatcher& operator=(AsyncTaskDispatcher&& ref) = delete;


	static void ThreadLoop(AsyncTaskDispatcher* dispather, SingleThreadedTaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool);

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

	template<typename R = void, typename T, typename ... Args>
	std::shared_ptr<Task<R, T, Args...>> CreateTask(T task, Args ... args)
	{
		std::pmr::polymorphic_allocator<Task<R, T, Args...>> alloc(m_Pool);
		return std::shared_ptr<Task<R, T, Args...>>(MakeTask<R>(task, std::tuple<Args...>(args...), alloc), [this](TaskDefinition* ptr) {
			DeleteTask(ptr, sizeof(Task<R, T, Args...>), alignof(Task<R, T, Args...>));
			});
	}

	void FlushDeallocations();


	template<typename F>
	void Run(F func)
	{
		
		m_Thread->RunThread(&StartThreadLoop<F>, this, func, m_Queue.get(), &m_runnning, m_Pool);

	}

	template<typename F, typename FO>
	void Run(F func, FO func_out)
	{
		
		m_Thread->RunThread(&StartThreadLoop<F, FO>, this, func, func_out, m_Queue.get(), &m_runnning, m_Pool);
		
	}

	void Run()
	{
		m_Thread->RunThread(&ThreadLoop, this, m_Queue.get(), &m_runnning, m_Pool);
	}

	//Carefull flush happens only for task submitted before flush, if worker running workerthreads submit work after flush, a significant slowdown can occur, 
	//this can be avoided by submitting task from a single thread.
	//FLUSH CAN ONLY BE CALLED BY MAINTHREAD, OTHWERWISE IT'S UNDEFINED BEHAVIOR
	void Flush();

	void FlushLoop();

	~AsyncTaskDispatcher();



private:

	uint32_t& GetSyncNum() { return reference_sync_var; };

	template<typename F>
	static void StartThreadLoop(AsyncTaskDispatcher* dispather, F func, SingleThreadedTaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool) {
		func();
		ThreadLoop(dispather, queue, run, pool);
	}

	template<typename F, typename FO>
	static void StartThreadLoop(AsyncTaskDispatcher* dispather, F func_in, FO func_out, SingleThreadedTaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool) {
		func_in();
		ThreadLoop(dispather, queue, run, pool);
		func_out();
	}

	std::atomic<bool> m_runnning = true;
	std::unique_ptr<SingleThreadedTaskQueue> m_Queue;
	std::shared_ptr<ThreadObject> m_Thread;
	SynchronizedMultiPool<std::allocator<void>, true>* m_Pool;

	uint32_t reference_sync_var = 0;

	std::mutex global_flush_mutex;
	std::mutex flush_mut;
	bool signaled;
	std::condition_variable flush_cond;

};


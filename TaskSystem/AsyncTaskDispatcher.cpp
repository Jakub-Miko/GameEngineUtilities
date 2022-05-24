#include <AsyncTaskDispatcher.h>
#include <thread>
#include <vector>
#include <Profiler.h>
#include <memory>



AsyncTaskDispatcher::AsyncTaskDispatcher() {
	m_Pool = new SynchronizedMultiPool<std::allocator<void>, true>(std::allocator<void>(), 32768);

	m_Queue.reset(new SingleThreadedTaskQueue);

	m_Thread = ThreadManager::Get()->GetThread();

}


void AsyncTaskDispatcher::FlushDeallocations()
{
	m_Pool->FlushDeallocations();
}

//Carefull flush happens only for task submitted before flush, if worker running workerthreads submit work after flush, a significant slowdown can occur, 
//this can be avoided by submitting task from a single thread.
//FLUSH CAN ONLY BE CALLED BY MAINTHREAD, OTHWERWISE IT'S UNDEFINED BEHAVIOR
void AsyncTaskDispatcher::Flush()
{
	std::lock_guard<std::mutex> lock_global(global_flush_mutex);
	signaled = false;
	auto task = [this]() {
		std::lock_guard<std::mutex> lock(flush_mut);
		signaled = true;
		flush_cond.notify_one();
	};

	m_Queue->SetIdleTask(CreateTask(task));
	std::unique_lock<std::mutex> lock(flush_mut);
	PROFILE("MainWait");
	flush_cond.wait(lock, [this]() { return signaled; });

}

void AsyncTaskDispatcher::FlushLoop()
{
	m_Queue->Flush(reference_sync_var);
	m_Pool->FlushDeallocations();
}

void AsyncTaskDispatcher::SetIdleTask(std::shared_ptr<TaskDefinition> task)
{
	m_Queue->SetIdleTask(task);
	m_Queue->Flush(reference_sync_var);
}


AsyncTaskDispatcher::~AsyncTaskDispatcher()
{
	m_runnning = false;
	FlushLoop();
	m_Thread->JoinThread();
	m_Thread.reset();
	m_Queue->Clear();
	delete m_Pool;
}

static void ThreadStartup(SynchronizedMultiPool<std::allocator<void>, true>* pool) {
	PROFILE("ThreadStartup");
	pool->InitializePools(std::this_thread::get_id());
}

void AsyncTaskDispatcher::ThreadLoop(AsyncTaskDispatcher* dispather, SingleThreadedTaskQueue* queue, std::atomic<bool>* run, SynchronizedMultiPool<std::allocator<void>, true>* pool) {
	ThreadStartup(pool);
	bool running = true;
	uint32_t& sync_ref = dispather->GetSyncNum();
	uint32_t sync_num = sync_ref;
	while (running && run->load()) {
		PROFILE("Load");
		std::shared_ptr<TaskDefinition> task = queue->Pop(sync_num, sync_ref);
		if (task) {
			task->Run();
		}
		pool->FlushDeallocations();
	}
}

void AsyncTaskDispatcher::DeleteTask(TaskDefinition* task, size_t size, size_t align)
{
	task->~TaskDefinition();
	m_Pool->deallocate(task, size, align);
}


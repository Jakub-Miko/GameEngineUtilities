#include <TaskSystem.h>
#include <thread>
#include <vector>
#include <Profiler.h>
#include <memory>

TaskSystem* TaskSystem::instance = nullptr;	

std::mutex startup_mutex;
std::condition_variable startup_cond;
int startup_thread_finished = 0;



TaskSystem::TaskSystem(TaskSystemProps props) : m_Props(props) {
	m_Pool = new SynchronizedMultiPool<std::allocator<void>,true>(std::allocator<void>(), 32768);

	if (!m_Props.num_of_threads) { 
		m_Props.num_of_threads = std::thread::hardware_concurrency() - 1;
	}
	m_Queue.reset(new TaskQueue(m_Props.num_of_threads));

	m_Threads = std::vector<std::thread*>(m_Props.num_of_threads);
}

void TaskSystem::JoinedThreadLoopIteration()
{
	PROFILE("Load");
	std::shared_ptr<TaskDefinition> task = m_Queue->PopExternal();
	if (task) {
		task->Run();
	}
	m_Pool->FlushDeallocations();
}

void TaskSystem::FlushDeallocations()
{
	m_Pool->FlushDeallocations();
}


void TaskSystem::Run()
{
	for (auto& thread : m_Threads) {
		thread = new std::thread(ThreadLoop,m_Queue.get(),&m_runnning,m_Pool);
	}
}


//Carefull flush happens only for task submitted before flush, if worker running workerthreads submit work after flush, a significant slowdown can occur, 
//this can be avoided by submitting task from a single thread.
//FLUSH CAN ONLY BE CALLED BY MAINTHREAD, OTHWERWISE IT'S UNDEFINED BEHAVIOR
void TaskSystem::Flush()
{
	std::lock_guard<std::mutex> lock_global(global_flush_mutex);
	signaled = false;
	auto task = [this]() {
		std::lock_guard<std::mutex> lock(flush_mut);
		signaled = true;
		flush_cond.notify_one();
	};

	m_Queue->SetIdleTask(CreateTask(task));
	Submit(CreateTask([]() {}));
	m_Queue->Flush();
	std::unique_lock<std::mutex> lock(flush_mut);
	PROFILE("MainWait");
	flush_cond.wait(lock, [this]() { return signaled; });

}

void TaskSystem::FlushLoop()
{
	for (auto& thread : m_Threads) {
		Submit(CreateTask([]() {}));
	}
	m_Queue->Flush();
	m_Pool->FlushDeallocations();
}

TaskSystem::~TaskSystem()
{
	m_runnning = false;
	FlushLoop();
	for (auto thread : m_Threads) {
		thread->join();
		delete thread;
	}
	m_Queue->Clear();
	startup_thread_finished = 0;
	delete m_Pool;
}

static void ThreadStartup(SynchronizedMultiPool<std::allocator<void>, true>* pool) {
	PROFILE("ThreadStartup");
	pool->InitializePools(std::this_thread::get_id());
	std::unique_lock<std::mutex> lock(startup_mutex);
	startup_thread_finished++;
	startup_cond.notify_all();
	startup_cond.wait(lock, []() {return startup_thread_finished >= TaskSystem::Get()->GetProps().num_of_threads; });
}

void TaskSystem::ThreadLoop(TaskQueue* queue, std::atomic<bool>* run,SynchronizedMultiPool<std::allocator<void>, true>* pool) {
	ThreadStartup(pool);
	bool running = true;
	while (running && run->load()) {
		PROFILE("Load");
		std::shared_ptr<TaskDefinition> task = queue->Pop();
		if (task) {
			task->Run();
		}
		pool->FlushDeallocations();
	}
}

void TaskSystem::DeleteTask(TaskDefinition* task,size_t size, size_t align)
{
	task->~TaskDefinition();
	m_Pool->deallocate(task,size,align);
}

void TaskSystem::SetIdleTask(std::shared_ptr<TaskDefinition> task)
{
	m_Queue->SetIdleTask(task);
}


TaskSystem* TaskSystem::Get()
{
	return instance;
}

void TaskSystem::Initialize(TaskSystemProps props)
{
	PROFILE("Initialization");	
	if (!instance) {

		instance = new TaskSystem(props);

	}
}

void TaskSystem::Shutdown()
{

	if (instance) {
		delete instance;
	}

}


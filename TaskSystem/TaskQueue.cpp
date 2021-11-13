#include <TaskQueue.h>
#include <Profiler.h>

TaskQueue::TaskQueue(int num_of_threads)
	: m_Queue(), m_QueueMutex(), runnnig_threads(num_of_threads),registered_threads(num_of_threads)
{
	
}

void TaskQueue::Push(std::shared_ptr<TaskDefinition> def)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_Queue.push(def);
	on_push.notify_one();
	lock.unlock();
}

std::shared_ptr<TaskDefinition> TaskQueue::Pop(uint32_t& sync_var, uint32_t& reference_sync_num)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	if (m_Queue.empty()) {
		--runnnig_threads;
		if (runnnig_threads == 0 && m_IdleTask) {
			PROFILE("IDLE");
			m_Queue.push(m_IdleTask);
			on_push.notify_one();
			m_IdleTask.reset();
		}
		on_push.wait(lock, [this, &sync_var, &reference_sync_num]() {return !m_Queue.empty() || sync_var != reference_sync_num; });
		++runnnig_threads;
		if (sync_var != reference_sync_num) {
			sync_var = reference_sync_num;
			return nullptr;
		}
		
	}

	std::shared_ptr<TaskDefinition> task = m_Queue.front();
	m_Queue.pop();
	return task;
}

void TaskQueue::RegisterThread()
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	runnnig_threads++;
	registered_threads++;
}

void TaskQueue::UnRegisterThread()
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	runnnig_threads--;
	registered_threads--;
}


void TaskQueue::SetIdleTask(std::shared_ptr<TaskDefinition> task)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_IdleTask = task;
}

void TaskQueue::Flush(uint32_t& ref_var)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	ref_var++;
	lock.unlock();
	on_push.notify_all();
}

void TaskQueue::Clear()
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	std::queue<std::shared_ptr<TaskDefinition>> empty;
	m_Queue.swap(empty);
}

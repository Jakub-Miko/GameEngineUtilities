#include <SingleThreadedTaskQueue.h>
#include <Profiler.h>

SingleThreadedTaskQueue::SingleThreadedTaskQueue()
{

}

void SingleThreadedTaskQueue::Push(std::shared_ptr<TaskDefinition> def)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_Queue.push(def);
	on_push.notify_one();
	lock.unlock();
}

std::shared_ptr<TaskDefinition> SingleThreadedTaskQueue::Pop(uint32_t& sync_var, uint32_t& reference_sync_num)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	if (m_Queue.empty()) {
		if (m_IdleTask) {
			PROFILE("IDLE");
			m_Queue.push(m_IdleTask);
			on_push.notify_one();
			m_IdleTask.reset();
		}
		on_push.wait(lock, [this, &sync_var, &reference_sync_num]() {return !m_Queue.empty() || sync_var != reference_sync_num; });
		if (sync_var != reference_sync_num) {
			sync_var = reference_sync_num;
			return nullptr;
		}

	}

	std::shared_ptr<TaskDefinition> task = m_Queue.front();
	m_Queue.pop();
	return task;
}


void SingleThreadedTaskQueue::SetIdleTask(std::shared_ptr<TaskDefinition> task)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	m_IdleTask = task;
}

void SingleThreadedTaskQueue::Flush(uint32_t& ref_var)
{
	std::unique_lock<std::mutex> lock(m_QueueMutex);
	ref_var++;
	lock.unlock();
	on_push.notify_all();
}

void SingleThreadedTaskQueue::Clear()
{
	std::lock_guard<std::mutex> lock(m_QueueMutex);
	std::queue<std::shared_ptr<TaskDefinition>> empty;
	m_Queue.swap(empty);
}

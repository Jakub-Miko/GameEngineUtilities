#include "TaskSystemFence.h"

TaskSystemFence::TaskSystemFence() : m_Mutex(), m_Cond(), counter(0)
{

}

void TaskSystemFence::Signal(uint32_t number)
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	counter = number;
	m_Cond.notify_all();
}

uint32_t TaskSystemFence::GetNumber()
{
	std::lock_guard<std::mutex> lock(m_Mutex);
	return counter;
}

bool TaskSystemFence::IsValue(uint32_t number)
{
	return GetNumber() >= number;
}

void TaskSystemFence::Wait(uint32_t number)
{
	std::unique_lock<std::mutex> lock(m_Mutex);
	m_Cond.wait(lock, [this, number]() {return counter >= number; });
}

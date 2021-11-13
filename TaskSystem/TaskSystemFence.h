#pragma once 
#include <mutex>
#include <cstddef>

class TaskSystemFence {
public:
	TaskSystemFence();

	void Signal(uint32_t number);

	uint32_t GetNumber();

	bool IsValue(uint32_t number);

	void Wait(uint32_t number);

private:
	std::mutex m_Mutex;
	std::condition_variable m_Cond;
	uint32_t counter;
};
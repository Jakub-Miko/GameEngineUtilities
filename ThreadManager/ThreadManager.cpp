#include "include/ThreadManager.h"
#include <thread>
#include <stdexcept>

ThreadManager* ThreadManager::instance = nullptr;
thread_local std::shared_ptr<ThreadObject> ThreadManager::current_thread = nullptr;


void ThreadManager::ResetThread(ThreadObject* ptr)
{
	ptr->ResetStateVariableList();
	
	ptr->SetState(nullptr);

	m_FreeThreads.push_back(ptr);
}

ThreadManager::ThreadManager() : m_Threads(), m_FreeThreads(), sync_mutex() {
	max_threads = (int)std::thread::hardware_concurrency();
	m_FreeThreads.reserve(max_threads);
	m_Threads.reserve(max_threads);
	for (int i = 0; i < max_threads; i++) {
		auto thread_obj = new ThreadObject(i + 1);
		m_Threads.push_back(thread_obj);
		m_FreeThreads.push_back(thread_obj);
	}
}

ThreadManager::~ThreadManager()
{
	for (auto thread : m_Threads) {
		delete thread;
	}
}

void ThreadManager::Init()
{
	if (!instance) {
		instance = new ThreadManager();
	}
}

void ThreadManager::Shutdown()
{
	if (instance) {
		delete instance;
	}
}

ThreadManager* ThreadManager::Get()
{
	return instance;
}

std::shared_ptr<ThreadObject> ThreadManager::GetThread()
{
	std::lock_guard<std::mutex> lock(sync_mutex);
	if (m_FreeThreads.empty()) {
		throw std::runtime_error("ThreadPool overflow");
	}
	
	auto thread = m_FreeThreads.back();
	m_FreeThreads.pop_back();

	auto ptr = std::shared_ptr<ThreadObject>(thread, [](ThreadObject* ptr) {
		ThreadManager::Get()->ResetThread(ptr);
		});

	ptr->SetState(ptr);
	return ptr;

}

std::shared_ptr<ThreadObject> ThreadManager::GetCurrentThread()
{
	if (current_thread) {
		return current_thread;
	}
	else {
		throw std::runtime_error("No ThreadObject is bound in the current thread");
	}
}

bool ThreadManager::IsValidThreadContext()
{
	return (bool)current_thread;
}

void ThreadObject::JoinThread()
{
	m_thread.join();
}

void ThreadObject::ResetStateVariableList()
{
	ThreadData.clear();
}

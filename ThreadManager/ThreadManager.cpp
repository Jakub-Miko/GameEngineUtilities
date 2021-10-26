#include "include/ThreadManager.h"
#include <thread>
#include <stdexcept>

ThreadManager* ThreadManager::instance = nullptr;

void ThreadManager::ResetThread(ThreadObject* ptr)
{
	m_FreeThreads.push_back(ptr);
}

ThreadManager::ThreadManager() : m_Threads(), m_FreeThreads() {
	max_threads = (int)std::thread::hardware_concurrency();
	m_FreeThreads.reserve(max_threads);
	m_Threads.reserve(max_threads);
	for (int i = 0; i < max_threads; i++) {
		auto thread_obj = new ThreadObject();
		thread_obj->thread_id = i + 1;
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

ThreadManager* ThreadManager::GetThreadManager()
{
	return instance;
}

std::shared_ptr<ThreadObject> ThreadManager::GetThread()
{
	if (m_FreeThreads.empty()) {
		throw std::runtime_error("ThreadPool overflow");
	}
	
	auto thread = m_FreeThreads.back();
	m_FreeThreads.pop_back();

	return std::shared_ptr<ThreadObject>(thread, [](ThreadObject* ptr) {
		ThreadManager::GetThreadManager()->ResetThread(ptr);
		});

}

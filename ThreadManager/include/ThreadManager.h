#pragma once
#include <vector>
#include <memory>

class ThreadManager;


class ThreadObject {
public:
	friend ThreadManager;

	int GetId() const {
		return thread_id;
	}

private:
	int thread_id;
};


class ThreadManager {
public:

	friend ThreadObject;

	ThreadManager(const ThreadManager& ref) = delete;

	ThreadManager(ThreadManager&& ref) = delete;

	ThreadManager& operator=(const ThreadManager& ref) = delete;

	ThreadManager& operator=(ThreadManager&& ref) = delete;

	static void Init();

	static void Shutdown();

	static ThreadManager* GetThreadManager();

	std::shared_ptr<ThreadObject> GetThread();

	int GetMaxThreadCount() const {
		return max_threads;
	}

	int GetAvailableThreadCount() const {
		return m_FreeThreads.size();
	}

private:
	
	void ResetThread(ThreadObject* ptr);

	ThreadManager();

	~ThreadManager();

private:

	static ThreadManager* instance;

private:

	std::vector<ThreadObject*> m_Threads;
	std::vector<ThreadObject*> m_FreeThreads;
	int max_threads;


};
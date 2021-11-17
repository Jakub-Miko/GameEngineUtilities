#pragma once
#include <vector>
#include <memory>
#include <thread>
#include <utility>
#include <mutex>

class ThreadManager;
class ThreadObject;

class ThreadObject_share_pointer_state {
public:
	ThreadObject_share_pointer_state(std::shared_ptr<ThreadObject> obj) : obj_ptr(obj) {}

protected:

	void SetState(std::shared_ptr<ThreadObject> obj)
	{
		obj_ptr = obj;
	}

	std::shared_ptr<ThreadObject> GetSharedFromThis() 
	{
		return std::shared_ptr<ThreadObject>(obj_ptr);
	}

private:
	std::weak_ptr<ThreadObject> obj_ptr;
};


class ThreadObject : public ThreadObject_share_pointer_state{
public:
	friend ThreadManager;

	int GetId() const {
		return thread_id;
	}


	template<typename F, typename ... Args>
	void RunThread(F func, Args ... args) {
		if (!m_thread.joinable()) {
			m_thread = std::thread(&ThreadObject::ThreadProcess<F,Args...>,this, func, args...);
		}
	}

	void JoinThread();

	~ThreadObject() {
		if (m_thread.joinable()) {
			m_thread.join();
		}
	}

private:
	ThreadObject(int id) : thread_id(id), m_thread(), ThreadObject_share_pointer_state(nullptr) {};

	template<typename F, typename ... Args>
	void ThreadProcess(F func, Args... args) {
		ThreadManager::current_thread = GetSharedFromThis();
		if (ThreadManager::current_thread) {
			func(args...);
			ThreadManager::current_thread = nullptr;
		}
		else {
			throw std::runtime_error("This ThreadObject is Invalid");
		}
	}

	std::thread m_thread;
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

	static ThreadManager* Get();

	std::shared_ptr<ThreadObject> GetThread();

	static std::shared_ptr<ThreadObject> GetCurrentThread();

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

	static thread_local std::shared_ptr<ThreadObject> current_thread;

	static ThreadManager* instance;

private:

	std::mutex sync_mutex;
	std::vector<ThreadObject*> m_Threads;
	std::vector<ThreadObject*> m_FreeThreads;
	int max_threads;


};
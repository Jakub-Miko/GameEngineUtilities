#pragma once
#include <vector>
#include <memory>
#include <stdexcept>
#include <thread>
#include <map>
#include <TypeId.h>
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
		ResetStateVariableList();

	}
	template<typename T>
	void SetStateValue(T* value) {
		ThreadData.insert_or_assign(TypeId<T, SequentialIdGenerator>(), value);
	}

	template<typename T>
	T* GetStateValue() {
		auto res = ThreadData.find(TypeId<T, SequentialIdGenerator>());
		if (res == ThreadData.end()) {
			throw std::runtime_error("No state value of this type could be found");
		}
		return reinterpret_cast<T*>(res->second);
	}

	template<typename T>
	bool StateValueExists() {
		return ThreadData.find(TypeId<T, SequentialIdGenerator>()) != ThreadData.end();
	}

private:
	ThreadObject(int id) : thread_id(id), m_thread(), ThreadObject_share_pointer_state(nullptr), ThreadData() {};

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

	void ResetStateVariableList();



	std::thread m_thread;
	int thread_id;

private:
	
	std::map<int,void*> ThreadData;

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

	template<typename T>
	static T* GetThreadLocalData() {
		return current_thread->GetStateValue<T>();
	}

	template<typename T>
	static void SetThreadLocalData(T* data) {
		current_thread->SetStateValue<T>(data);
	}

	template<typename T>
	static bool ThreadLocalDataExists() {
		return current_thread->StateValueExists<T>();
	}

	void JoinedThreadRegister(std::shared_ptr<ThreadObject>& thread);
	void JoinedThreadUnRegister();

	static bool IsValidThreadContext();

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
#pragma once
#include <future>
#include <utility>

template<typename T>
class Promise;

template<typename T>
class Future {
public:
	friend class Promise<T>;
	
	Future() : m_future() {

	}

	Future(const Future& ref) : m_future(ref.m_future) {

	}

	Future(Future&& ref) : m_future(std::move(ref.m_future)) {

	}

	Future& operator=(const Future& ref) noexcept {
		m_future = ref.m_future;
	}

	Future& operator=(Future&& ref) noexcept {
		m_future = std::move(ref.m_future);
	}

	~Future() {

	}

	T GetValue() const {
		return m_future.get();
	}

	void Wait() {
		m_future.wait();
	}

	bool valid() {
		return m_future.valid();
	}

private:
	Future(const std::shared_future<T>& future) : m_future(future) {}

	std::shared_future<T> m_future;

};

template<typename T>
class Promise {
public:
	Promise() : m_promise() {

	}

	Promise(const Promise& ref) : m_promise(ref.m_promise) {

	}

	Promise(Promise&& ref) : m_promise(std::move(ref.m_promise)) {

	}

	Promise& operator=(const Promise& ref) noexcept {
		m_promise = ref.m_promise;
	}

	Promise& operator=(Promise&& ref) noexcept {
		m_promise = std::move(ref.m_promise);
	}

	~Promise() {

	}

	Future<T> GetFuture() {
		return Future<T>(m_promise.get_future().share());
	}

	void SetValue(T&& value) {
		m_promise.set_value(std::forward<T>(value));
	}

	void SetException(std::exception_ptr&& exception) {
		m_promise.set_exception(std::forward<std::exception_ptr>(exception));
	}


private:
	std::promise<T> m_promise;

};

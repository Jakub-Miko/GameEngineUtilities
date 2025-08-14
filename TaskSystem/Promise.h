#pragma once
#include <future>
#include <utility>

template<typename T>
class Awaitable {
public:
	virtual T GetValue() const = 0;
	virtual bool IsAvailable() = 0;
	virtual void Wait() = 0;
	virtual ~Awaitable() {}
};

template<>
class Awaitable<void> {
public:
	virtual bool IsAvailable() = 0;
	virtual void Wait() = 0;
};


template<typename T>
class Promise;

template<typename T>
class Future : public Awaitable<T> {
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
		return *this;
	}

	Future& operator=(Future&& ref) noexcept {
		m_future = std::move(ref.m_future);
		return *this;
	}

	virtual ~Future() {

	}

	virtual T GetValue() const override {
		return m_future.get();
	}

	virtual void Wait() override {
		m_future.wait();
	}

	bool IsValid() const {
		return m_future.valid();
	}

	virtual bool IsAvailable() override {
		return m_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
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
		return *this;
	}

	Promise& operator=(Promise&& ref) noexcept {
		m_promise = std::move(ref.m_promise);
		return *this;
	}

	~Promise() {

	}

	Future<T> GetFuture() {
		return Future<T>(m_promise.get_future().share());
	}

	void SetValue(T&& value) {
		m_promise.set_value(std::forward<T>(value));
	}

	void SetException(std::exception_ptr exception) {
		m_promise.set_exception(exception);
	}


private:
	std::promise<T> m_promise;

};

template<>
class Future<void> : public Awaitable<void> {
public:
	friend class Promise<void>;

	Future() : m_future() {

	}

	Future(const Future& ref) : m_future(ref.m_future) {

	}

	Future(Future&& ref) : m_future(std::move(ref.m_future)) {

	}

	Future& operator=(const Future& ref) noexcept {
		m_future = ref.m_future;
		return *this;
	}

	Future& operator=(Future&& ref) noexcept {
		m_future = std::move(ref.m_future);
		return *this;
	}

	virtual ~Future() {

	}

	virtual void Wait() override {
		m_future.wait();
	}

	bool IsValid() const {
		return m_future.valid();
	}


	virtual bool IsAvailable() override {
		return m_future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready;
	}

private:
	Future(const std::shared_future<void>& future) : m_future(future) {}

	std::shared_future<void> m_future;

};

template<>
class Promise<void> {
public:
	Promise() : m_promise() {

	}

	Promise(Promise&& ref) : m_promise(std::move(ref.m_promise)) {

	}

	Promise& operator=(Promise&& ref) noexcept {
		m_promise = std::move(ref.m_promise);
		return *this;
	}

	~Promise() {

	}

	Future<void> GetFuture() {
		return Future<void>(m_promise.get_future().share());
	}

	void SetValue() {
		m_promise.set_value();
	}


	void SetException(std::exception_ptr exception) {
		m_promise.set_exception(exception);
	}


private:
	std::promise<void> m_promise;

};

template<typename T>
class CustomAwaitable : public Awaitable<T> {
public:	
	CustomAwaitable(std::function<void()> wait_func, std::function<bool()> is_available_func, std::function<T()> get_value_func) : wait_func(wait_func), is_available_func(is_available_func), get_value_func(get_value_func) {}

	virtual T GetValue() const override {
		return get_value_func();
	}

	virtual bool IsAvailable() override {
		return is_available_func();
	}

	virtual void Wait() override {
		wait_func();
	}

	virtual ~CustomAwaitable() {}
public:
	std::function<void()> wait_func;
	std::function<bool()> is_available_func;
	std::function<T()> get_value_func;
};

template<>
class CustomAwaitable<void> : public Awaitable<void> {
public:	
	CustomAwaitable(std::function<void()> wait_func, std::function<bool()> is_available_func) : wait_func(wait_func), is_available_func(is_available_func) {}

	virtual bool IsAvailable() override {
		return is_available_func();
	}

	virtual void Wait() override {
		wait_func();
	}

	virtual ~CustomAwaitable() {}
public:
	std::function<void()> wait_func;
	std::function<bool()> is_available_func;
};
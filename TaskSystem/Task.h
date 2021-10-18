#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <future>
#include <Profiler.h>
class TaskDefinition {
public:
	virtual void Run() = 0;
};



template<typename R, typename T, typename ... Args>
class Task : public TaskDefinition{
public:
	Task(T function, std::tuple<Args...> args) : function(function),data(args) {

	}

	Task(const Task<R,T,Args...>& ref) :function(ref.function), data(ref.data) {}

	virtual void Run() override {
		promise.set_value(std::apply(function,data));
	}

	std::shared_future<R> GetFuture() {
		return promise.get_future().share();
	}

private:
	T function;
	std::tuple<Args...> data;
	std::promise<R> promise;
};

template<typename R, typename T>
class Task<R, T> : public TaskDefinition {
public:
	Task(T function) : function(function) {

	}

	Task(const Task<R,T>& ref) :function(ref.function) {}


	virtual void Run() override {
		promise.set_value(std::invoke(function));
	}

	std::shared_future<R> GetFuture() {
		return promise.get_future().share();
	}

private:
	T function;
	std::promise<R> promise;
};

template<typename T, typename ... Args>
class Task<void,T,Args...> : public TaskDefinition {
public:
	Task(T function, std::tuple<Args...> args) : function(function), data(args) {

	}

	Task(const Task<void, T, Args...>& ref) :function(ref.function), data(ref.data) {}

	virtual void Run() override {
		std::apply(function, data);
	}
private:
	T function;
	std::tuple<Args...> data;
};

template<typename T>
class Task<void, T> : public TaskDefinition {
public:
	Task(T function) : function(function) {

	}

	Task(const Task<void,T>& ref) :function(ref.function) {}

	virtual void Run() override {
		std::invoke(function);
	}
private:
	T function;
};

template<typename R = void,typename T, typename ... Args, typename Allocator = std::allocator<Task<R,T, Args...>>>
auto MakeTask(T task, std::tuple<Args...> args, Allocator alloc = std::allocator<Task<R,T, Args...>>())
{
	if constexpr (std::is_void_v<R>) {
		if constexpr (sizeof...(Args) <= 0) {
			Task<void,T>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
			std::allocator_traits<Allocator>::construct(alloc, ptr, task);
			return ptr;
		}
		else {
			Task<void,T, Args...>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
			std::allocator_traits<Allocator>::construct(alloc, ptr, task, args);
			return ptr;
		}
	}
	else {
		if constexpr (sizeof...(Args) <= 0) {
			Task<R, T>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
			std::allocator_traits<Allocator>::construct(alloc, ptr, task);
			return ptr;
		}
		else {
			Task<R, T, Args...>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
			std::allocator_traits<Allocator>::construct(alloc, ptr, task, args);
			return ptr;
		}
	}
}

#pragma once
#include <memory>
#include <tuple>
#include <functional>
#include <Profiler.h>
class TaskDefinition {
public:
	virtual void Run() = 0;

};



template<typename T, typename ... Args>
class Task : public TaskDefinition{
public:
	Task(T function, std::tuple<Args...> args) : function(function),data(args) {

	}

	Task(const Task<T,Args...>& ref) :function(ref.function), data(ref.data) {}

	virtual void Run() override {
		std::apply(function, data);
	}
private:
	T function;
	std::tuple<Args...> data;
};

template<typename T>
class Task<T> : public TaskDefinition {
public:
	Task(T function) : function(function) {

	}

	Task(const Task<T>& ref) :function(ref.function) {}

	virtual void Run() override {
		std::invoke(function);
	}
private:
	T function;
	
};

template<typename T, typename ... Args, typename Allocator = std::allocator<Task<T, Args...>>>
TaskDefinition* MakeTask(T task, std::tuple<Args...> args, Allocator alloc = std::allocator<Task<T, Args...>>())
{
	if constexpr (sizeof...(Args) <= 0) {
		Task<T>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
		std::allocator_traits<Allocator>::construct(alloc, ptr, task);
		return ptr;
	}
	else {
		Task<T, Args...>* ptr = std::allocator_traits<Allocator>::allocate(alloc, 1);
		std::allocator_traits<Allocator>::construct(alloc, ptr, task, args);
		return ptr;
	}
}

#pragma once
#include <memory>
#include <tuple>

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


template<typename T,typename ... Args,typename Allocator = std::allocator<Task<T, Args...>>>
TaskDefinition* MakeTask(T task, std::tuple<Args...> args = std::tuple<>(), Allocator alloc = std::allocator<Task<T, Args...>>()) {
	
	Task<T, Args...>* ptr = std::allocator_traits<Allocator>::allocate(alloc,1);
	std::allocator_traits<Allocator>::construct(alloc, ptr, task , args);
	return ptr;
}
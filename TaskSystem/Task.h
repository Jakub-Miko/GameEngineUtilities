#pragma once

class TaskDefinition {
public:

	TaskDefinition(void(*function)()) : function(function) {}

	void Run() { function(); }
private:
	void(*function)();
};
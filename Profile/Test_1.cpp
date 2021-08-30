#include <iostream>
#include "Profiler.h"
#include <mutex>
#include <condition_variable>
#include <thread>

std::mutex mutex_1;
std::condition_variable cond_var;
int counter = 0;

void Work() {
	PROFILE_FUNCTION();
	while (counter < 100) {
		PROFILE("SECONDARY LOOP");
		
		std::unique_lock<std::mutex> mutex(mutex_1);
		{
			PROFILE("SECONDARY LOCK");
			cond_var.wait(mutex);
		}
		{
			PROFILE("SECONDARY IO");
			std::cout << "Im awake" << std::endl;
		}
	}
}


int main() {
	std::thread thread(Work);
	BEGIN_PROFILING("Profiler", "C:/Users/mainm/Desktop/GameEngine/Experimental/Profile/Profile.json");
	{
		PROFILE_FUNCTION();

		while (counter <= 100) {
			PROFILE("PRIMARY LOOP");
			{
				PROFILE("PRIMARY LOCK");
				std::lock_guard<std::mutex> guard(mutex_1);
				counter++;
			}
			{
				PROFILE("PRIMARY IO");
				std::cout << "Working!" << std::endl;
			}
			if ((counter % 4) == 0) cond_var.notify_one();

		}

		std::unique_lock<std::mutex> guard(mutex_1);
		cond_var.notify_one();
		guard.unlock();

		thread.join();
	}
	END_PROFILING();
	std::cin.get();
}
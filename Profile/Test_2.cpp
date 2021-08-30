#include <Profiler.h>

void main() {
	BEGIN_PROFILING("Profiler2", "C:/Users/mainm/Desktop/GameEngine/Experimental/Profile/Profile2.json");
	for (int i = 0; i < 20; i++) {
		PROFILE("Profile_Profiling");
		PROFILE("Profile Subject");
	}
	END_PROFILING();
}
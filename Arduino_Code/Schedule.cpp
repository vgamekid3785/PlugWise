#include "Schedule.h"

//Function to change all the member variables
void Schedule::create(const time_t new_time, const long new_inter, const unsigned int new_ticks, void (*function)()){
	hit_time = new_time;
	interval = new_inter;
	ticks = new_ticks;
	func = function;
}

//Function to check whether or not the function should be called
void Schedule::check_time(){
	//If current time is greater than start time && ticks > 0
	if (now() >= hit_time && ticks > 0){
		hit_time += interval*60;
		--ticks;
		func();
	}
}


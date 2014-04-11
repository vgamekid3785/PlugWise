#include "Schedule.h"

void Schedule::create(const time_t new_time, const long new_inter, const unsigned int new_ticks, void (*function)()){
	change_time(new_time);
	change_interval(new_inter);
	change_ticks(new_ticks);
	change_type(function);
}

void Schedule::check_time(){
	//If current time is greater than start time && ticks > 0
	if (now() > hit_time && ticks > 0){
		hit_time += interval*60;
		--ticks;
		func();
	}
}


#include <Time.h>

class Schedule {
public: 
//Constructors
Schedule(){
	double hit_time=0;
        double interval=0;
        double ticks = 0;
}
//~Schedule();

//Modifiers
void change_time(const time_t new_time) {hit_time = new_time;}
void change_interval(const long new_interval) {interval = new_interval;}
void change_ticks(const unsigned int new_ticks) {ticks = new_ticks;}
void change_type(void (*function)()) {func = function;}
void create(const time_t new_time, const long new_inter, const unsigned int new_ticks, void (*function)());

//If the next function should go, if so, decrements ticks, moves start time and returns 1;
void check_time();

//Representation
time_t hit_time;
long interval;
unsigned int ticks;
void (*func)();
};






//Functions
	// constructor
	// destructor
	
	// check time to turn on
	// reset based on new current time

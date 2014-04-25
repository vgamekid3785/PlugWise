#include <Time.h>

class Schedule {
public: 
//Constructors
Schedule(){
	time_t hit_time=0;
    long interval=0;
    unsigned int ticks = 0;
}

//Modifiers
void create(const time_t new_time, const long new_inter, const unsigned int new_ticks, void (*function)());

//If the next function should go, if so, decrements ticks, moves start time and returns 1;
void check_time();

//Representation
time_t hit_time;
long interval;
unsigned int ticks;
void (*func)();
};

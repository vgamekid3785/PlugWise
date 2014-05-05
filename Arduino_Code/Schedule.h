#include <Time.h>

class Schedule {
public: 
//Constructors
Schedule(){
	time_t hit_time=0;
    long interval=0;
    unsigned int ticks = 0;
}

//Create function to change the values in the class
void create(const time_t new_time, const long new_inter, const unsigned int new_ticks, void (*function)());

//If the next function should go, if so, decrements ticks, moves start time
void check_time();

//Representation
//Time that the function will be called
time_t hit_time;
//Interval for how often the function should be calles
long interval;
//Number of times the function should be called
unsigned int ticks;
//Function to be called
void (*func)();
};

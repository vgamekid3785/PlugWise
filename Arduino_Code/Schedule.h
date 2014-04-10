class Schedule {
public: 
//Constructors
Schedule();
~Schedule();

//Modifiers
void change_time(const time_t& new_time) {hit_time = new_time;}
void change_interval(const int& new_interval) {interval = new_interval;}
void change_ticks(const int& new_ticks) {ticks = new_ticks;}
void change_type(const int& new_type) {type = new_type;}
void create(const time_t& new_time, const int& new_inter, const int& new_ticks, const int& new_type);

//If the next function should go, if so, decrements ticks, moves start time and returns 1;
void check_time();

//Representation
time_t hit_time;
int interval;
int ticks;
int type;
};

void Schedule::create(const time_t& new_time, const int& new_inter, const int& new_ticks, const int& new_type){
	change_time(new_time);
	change_interval(new_inter);
	change_ticks(new_ticks);
	change_type(new_type);
}

Schedule::Schedule(){
hit_time,interval,ticks,type = 0;
}





//Functions
	// constructor
	// destructor
	
	// check time to turn on
	// reset based on new current time

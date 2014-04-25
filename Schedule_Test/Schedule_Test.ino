#include "Schedule.h"
#include <Time.h>

boolean first = true;
time_t var_time;
Schedule temp;

void print_time(time_t stime){
	Serial.println(second(stime));
	Serial.println(minute(stime));
	Serial.println(hour(stime));
	Serial.println(day(stime));
	Serial.println(month(stime));
	Serial.println(year(stime));
	Serial.println("End");
}

time_t serial_read_time(){
	tmElements_t temp;
	time_t timet;
    temp.Second = Serial.parseInt();
	temp.Minute = Serial.parseInt();
	temp.Hour = Serial.parseInt();
	temp.Day = Serial.parseInt();
	temp.Month = Serial.parseInt();
	temp.Year = Serial.parseInt();	
	timet = makeTime(temp);
	return timet;
}

void func(){
  Serial.println("Schedule Call");
}

Schedule parse_schedule(){
	//Need to figure out which function based on next ********
	int next = Serial.parseInt();
	//**Temporarily passing temp
	time_t start = serial_read_time();
	long interval = Serial.parseInt();
	time_t end = serial_read_time();
	Serial.flush();
	//This is the length of time that the schedule will be on for
	double length = (end - start)/60; //Return is in seconds
	//If timer, make ticks 1
    unsigned int ticks = 1;
	//Otherwise how many times the device will have to do the function. length of time of schedule / how often to do the action
	if (length) ticks = (int)ceil(length/interval);
	//Set the schedule to the parsed data from Serial, last pass is function
    Schedule return_schedule;
	return_schedule.create(start, interval, ticks, func);
    return return_schedule;
}

void setup(){
	Serial.begin(9600);
}

time_t start, end;
int type, between;
unsigned long before;
Schedule test_sch;

void loop(){
	//If the time is not set (initial power on), request the user to change the current time
	if (timeStatus()== timeNotSet){
		Serial.println("Request Sync Internal time");
		while(!Serial.available());
		setTime(serial_read_time());
		print_time(now());
		Serial.println("Update Frequency in Seconds");
		while(!Serial.available());
		between = Serial.parseInt()*1000;
		Serial.println(between);
		Serial.println("Schedule?");
		while(!Serial.available());
		test_sch = parse_schedule();
		Serial.println("Schedule will start at");
		print_time(test_sch.hit_time);
		Serial.write("For: ");
		Serial.println(test_sch.ticks);
		Serial.write(" times\n");
		before = millis(); 
        Serial.println("End parse\n");
	}
	if (millis() >= before + between){
		Serial.println("Current Time:");
		print_time(now());
		// Serial.println("millis:");
		// Serial.println(millis());
		// Serial.println("before before:");
		// Serial.println(before);
		before = millis();
		// Serial.println("before after:");
		// Serial.println(before);

	}
//        Serial.println("Times:");
//        Serial.println(before+between);
//	Serial.println(millis());
//        delay(3000);
	/*
	Serial.println(type);
	print_time(start);
	Serial.println(interval);
	print_time(end);
    Serial.println("---ticks---");
        
	double length = ((end - start)/60);
	int ticks = (int)ceil(length/interval);
	Serial.println(length);
	Serial.println(ticks);
	
	print_time(now());
	//Serial.println(test_sch.interval);
	Serial.println(test_sch.ticks);
	char a = '1';
	Serial.println("A");
	Serial.println((int)a);
	//Have to change schedule function in main code
	test_sch.check_time();
	delay(5000);
	if (test_sch.func == func) Serial.println("HIt");
	
	Serial.println("Current time:");
	print_time(now());
	Serial.println("-------------------");
    */
	test_sch.check_time();
	
}

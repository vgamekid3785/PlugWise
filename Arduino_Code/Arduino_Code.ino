#include <SoftwareSerial.h>  
#include <Time.h>
#include "Schedule.h"

//------------------------
//Pin Declarations 
//------------------------
#define bluetoothTx 11  // TX-O pin of bluetooth mate, Arduino D2
#define bluetoothRx 10  // RX-I pin of bluetooth mate, Arduino D3
#define power 9        	// Pin used to turn power on an off
#define led 13         	// Pin used to control safety LEDs
#define cmo 12         	// Pin used to read from the carbon monoxide detector
#define con 6         	// check pin if the BT module is paired
#define photor A6   	// Pin of photoresistor
#define swit 5    	   	// Pin of switch

//-----------------------
//Variable Declarations
//-----------------------
char val;                    	// Value received from bluetooth serial
boolean cmol = false;        	// Bool to make sure duplicate notifications are not pushed to device
float co_val = 500;				// Danger value for the carbon monoxide detector
unsigned int setting = 0;       // Value deciding whether or not to turn off power when no slave is detected
boolean powr = false;			// Whether or not power is flowing to the device
boolean conekt = false; 		// Is the android connected from bluetooth
boolean safety_auto = true;		// Are the safety lights set to automatically turn on
boolean ignore = false;			// Bool to ignore the conekt setting if push button is pushed
float min_value = 100.0;      	// Dark value for photoresistor to turn on safety lights
long last1;						// Values for checking if connected
long last0;
boolean debounce = false;		// Bool to make sure the device only changes once per push button click
Schedule blank;					// Blank schedule to keep in storage
Schedule actions[4] = {blank,blank,blank,blank}; //Arranged power off, on, safety off, on

//-----------------------
//--Function Prototypes--
//-----------------------
void change_power(const int& setting);						//Change power based on setting paseed in
void ambient_light_check(const unsigned int& min_value);	//Check the ambient light and turn the safety lights on if it is below the dark value
void carbon_monoxide_check(const boolean& connected);		//Check for carbon monoxide in the air
void parse_schedule(Schedule new_schedule);					//Scheduler parser from the bluetooth buffer
time_t bluetooth_read_time();								//Reads a time from the bluetooth and returns a type time_t
void main_power_on();										//Scheduling functions
void main_power_off();										//
void safety_lights_on();									//
void safety_lights_off();									//
void sync_time(); 											//Syncs the internal clock to the time given from the arduino

//Set the bluetooth to a software serial connection
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Initializations
void setup()
{
  Serial.begin(9600);             // Begin the serial monitor at 9600bps
  bluetooth.begin(9600);       	  // The Bluetooth Mate defaults to 115200bps
  pinMode(power, OUTPUT);         // Set power pin to output
  digitalWrite(power, LOW);       // Initialize power to off initially
  pinMode(led, OUTPUT);           // Set LED pin to output
  digitalWrite(led, LOW);         // Initialize the safety LEDs off initially
  pinMode(con,INPUT);             // Set the connection pin to input 
  pinMode(cmo, INPUT);			  // Carbon monoxide pin to input
  pinMode(swit, INPUT);			  // Switch pin to input
  pinMode(photor,INPUT);		  // Photoresistor pin to input
}

void sync_time(){
	Serial.println("Requesting Time");
	//Sync time from device and schedule 
	//time format second,hour,minute,day,month,year
	//Send request for time from device
	bluetooth.write("s");
	//Wait for response from device
	while(!bluetooth.available());
	setTime(bluetooth_read_time());
	bluetooth.println("Received time");
}

void loop()
{ 
  //Connection checking since coneckt pin cycles when disconnect and solid when connected
  if(digitalRead(con) == HIGH) {
	last1 = millis();
  }
  else {
    last0 = millis();
  }
  //If the device is connected, if the internal time is not set, request it, and also set connect == true
  if(last1 - last0 > 500) {
	if (timeStatus()== timeNotSet) sync_time(); 
	conekt = true;
  }
  else {
	conekt = false; 
  }

  if(digitalRead(swit)==HIGH){
	debounce = false;
  }
  //Manual override for a physical switch to change the state of the device
  if(digitalRead(swit)==LOW && !debounce){
	if (!conekt) ignore = true;
	debounce = true;
	if(powr == false){
		Serial.println("Power On (Switch)");
		powr = true;
		digitalWrite(power, HIGH);
	}
	else{
		powr = false;
		Serial.println("Power off (switch)");
		digitalWrite(power,LOW);
    }
  }
  //If the bluetooth sent any characters
  if(bluetooth.available())
  {
    //-------------------------------------------
    //---------------Receiving Data--------------
    //-------------------------------------------
    val = bluetooth.read();
    //----------------Immediate Commands---------------
    // if the value received is H turn on power to the socket
    if(val == 'H')
    {
        digitalWrite(power, HIGH);
        powr = true;
		ignore = false;
        Serial.println("Turned on power! (Command)");
    } 
    // if the value received is L turn off power to the socket
    else if(val == 'L')
    {
        digitalWrite(power, LOW);
        powr = false;
		ignore = false;
        Serial.println("Turned off power! (Command)");
    }

    // if the value received is S turn on the safety lights 
    else if(val == 'S')
    {
        digitalWrite(led, HIGH);
		safety_auto = false;
        Serial.println("Turned on safety lights!");
    } 
    // if the value received is U turn off the safety lights
    else if(val == 'U')
    {
        digitalWrite(led, LOW);
		safety_auto = false;
        Serial.println("Turned off safety lights!");
    } 
    //------------Safety Light Calibration---------- Command to calibrate the value for the photo resistor
    else if (val == 'C') 
    {
		Serial.println("Calibrating LED's");
		//Make sure that the LED's are on for the test
		if (!digitalRead(led)) digitalWrite(led,HIGH);
		min_value = 0;
		//Averaging 3 values to be sure
		min_value += analogRead(photor);
		delay(1000);
		min_value += analogRead(photor);
		delay(1000);
		min_value += analogRead(photor);
		min_value /= 3;
		//
		min_value -= 20;
		digitalWrite(led,LOW);
    }

    //--------------Setting Changes-----------------

    //--------------Safety Light Settings-----------  ****need to hardcode calibration into it.
    else if (val == '4'){
      safety_auto = true;
      Serial.println("Safety LED lights auto mode");
    }
    else if (val == '5'){
	  safety_auto = false;
      Serial.println("Safety LED lights manual mode");
    }

    //--------------Power Settings-------------------
    // if the value received is O change setting to 0, If the use wants the device to turn off when they go out of range
    else if(val == '1')
    {
        setting = 0;
		ignore = false;
        Serial.println("Changed setting to 0");
    }
    // if the value received is H change setting to 1, If the use wants the device to turn on when they go out of range
    else if(val == '2')
    {
        setting = 1;
		ignore = false;
        Serial.println("Changed setting to 1");
    }
    // if the value received is A change setting to 2, If the user wants the device to retain it's current power state when they go out of range
    else if(val == '3')
    {
      setting = 2;   
	  ignore = false;
      Serial.println("Changed setting to 2"); 
    }
	//-------------Scheduling-----------------------
	//Reading in a schedule from device
	else if (val = '(')
	{	
		Schedule new_schedule = parse_schedule();
		//Set correct action depending on schedule type
		if (new_schedule.func == main_on) actions[0] = new_schedule;
		if (new_schedule.func == main_off) actions[1] = new_schedule;
		if (new_schedule.func == safety_on) actions[2] = new_schedule;
		if (new_schedule.func == safety_off) actions[3] = new_schedule;
                //Store in EEPROM
	}
	//Remove a schedule, just sets ticks = 0;
	//Sent line is 'R#' with # replaced with a # 0 through 4 for which schedule to clear
	else if (val = 'R'){
		val = bluetooth.read();
		//Reading which schedule to clear
		actions[((int)val-48)].ticks = 0;
	}
    // delay before next command occurs 
    delay(50);
  }
  // Turn outlet on or off when Bluetooth disconnects depending on setting set by user. 
  if (!conekt) change_power(setting); 
  //Turn on or off the safety lights depending on if safety auto is on and it is darker than the minimum
  if (safety_auto) ambient_light_check(min_value);
  //CO check
  carbon_monoxide_check(conekt);
  //Scheduling checks 
  for (int i=0; i < 4; ++i){
	if (actions[i].ticks > 0) actions[i].check_time();
  }
  delay(500);
}
//Functions for Scheduling 
//=======================
//Main power options change out of range setting to constant to make sure the power stays on or off when it changes
//Safety lights auto is set to false so the safety lights stay on or off regardless of lighting conditions
void main_on(){
    if(!powr){
      digitalWrite(power, HIGH);
      powr = true;
      setting = 3;
      Serial.println("Turned on power! (Schedule)");
    }
}
void main_off(){
    if(powr){
      digitalWrite(power, LOW);
      powr = false;
      setting = 3;
      Serial.println("Turned off power! (Schedule)");
    }
}
void safety_on(){
  if(digitalRead(led) == LOW){
    digitalWrite(led, HIGH);
    safety_auto = false;
    Serial.println("Turned on safety lights! (Schedule)");
  }
}
void safety_off(){
  if(digitalRead(led) == HIGH){
    digitalWrite(led, LOW);
    safety_auto = false;
    Serial.println("Turned off safety lights! (Schedule)");
  }
}
//========================

//Prints out the time to the Serial for debugging
void print_time(time_t stime){
	Serial.println("Time_t has:");
	Serial.println(second(stime));
	Serial.println(minute(stime));
	Serial.println(hour(stime));
	Serial.println(day(stime));
	Serial.println(month(stime));
	Serial.println(year(stime));
	Serial.println("End Time_t --");
}

//Reads a time from bluetooth Serial
//year needs to be current date - 1970 
time_t bluetooth_read_time(){
	tmElements_t temp;
	temp.Second = bluetooth.parseInt();
	temp.Minute = bluetooth.parseInt();
	temp.Hour = bluetooth.parseInt();
	temp.Day = bluetooth.parseInt();
	temp.Month = bluetooth.parseInt();
	temp.Year = bluetooth.parseInt();	
    time_t timet;
    timet = makeTime(temp);
	return timet;
}

//-----------------------------------------------------------
//---------------------Scheduling----------------------------
//-----------------------------------------------------------
//Type is the function that the schedule will be performing
//Start and end are the start and end times
//Interval is how often the function should call
//Return is a type schedule that is the parsed schedule
Schedule parse_schedule(){
	int type = bluetooth.parseInt();
	time_t start = bluetooth_read_time();
	int interval = bluetooth.parseInt();
	time_t end = bluetooth_read_time();
	bluetooth.flush();
	//This is the length of time that the schedule will be on for
	double length = (end - start)/60; //Return is in seconds
	//If timer, make ticks 1, else ticks is number of times function is called
    unsigned int ticks = 1;
	//Otherwise how many times the device will have to do the function. length of time of schedule / how often to do the action
	if (length) ticks = (int)ceil(length/interval);
	//Set the schedule to the parsed data from Serial, last pass is function
	Schedule return_schedule; 
	//Changes function based on next value passed and returns the schedule
	if (type == 0) return_schedule.create(start, interval, ticks, main_on);
	else if (type == 1) return_schedule.create(start, interval, ticks, main_off);
	else if (type == 2) return_schedule.create(start, interval, ticks, safety_on);
	else return_schedule.create(start, interval, ticks, safety_off);
	return return_schedule;
}

//Function to change the state of the main power based on the passed setting 
void change_power(const int& set){
  //If the current setting is 0 (turn off out of range) or 1 (turn on out of range) 
  if (set < 2 && !ignore){
	//Turn off if out of range and power is on
    if (set == 0  && powr){
      digitalWrite(power, LOW);
      Serial.println("Turned off power! (Disconnect)");
      powr = false;
    }
	//Turn on if out of range and power is off
    else if (set == 1 && !powr){
      digitalWrite(power, HIGH);
      Serial.println("Turned on power! (Disconnect)");
      powr = true;
    }
  }
}

//-----------------------------------------------------------
//--------------------Photoresistor--------------------------
//-----------------------------------------------------------

void ambient_light_check(const unsigned int& min_value){
  //If the ambient light is less than the dark value and the LED's arn't already on, turn them on
  if (analogRead(photor) < min_value && !digitalRead(led)){
    Serial.println("Turned on LED's, darkness");
    digitalWrite(led,HIGH);
  }
  //If the ambient light is greater than the dark values and the LED's are on, turn them off
  else if (analogRead(photor) > min_value && digitalRead(led)){
    Serial.println("Turned off LED's, brightness");
    digitalWrite(led,LOW);
  }
}

//-----------------------------------------------------------
//----------------Carbon Monoxide Detection------------------
//-----------------------------------------------------------

//All the code for carbon monoxide detection (needs some tweaks)
//if carbon monoxide is detected, store that it is

void carbon_monoxide_check(const boolean& connected){
  // If the carbon monoxide detector is reporting a larger value than the danger value, send a notification to the bluetooth device
  if(analogRead(cmo) > co_val){ 
	//IF statement to make sure notification is not pushed multiple times
	if (connected && !cmol) bluetooth.println('1');
	cmol = true;
  } 
  //otherwise store that it is not detected
  else cmol = false;
}


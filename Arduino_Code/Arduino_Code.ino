#include <SoftwareSerial.h>  
#include <Time.h>
#include "Schedule.h"
//************
// Parse int may not work because i believe it returns long, in that case need to check if we have enough memory

//------------------------
//Pin Declarations 
//------------------------
#define bluetoothTx 11  // TX-O pin of bluetooth mate, Arduino D2
#define bluetoothRx 10  // RX-I pin of bluetooth mate, Arduino D3
#define power 9        // Pin used to turn power on an off
#define led 13         // Pin used to control safety LEDs
#define cmo 12         // Pin used to read from the carbon monoxide detector
#define con 6          // check if the BT module is paired
#define photor 5   
#define swit 7    

//-----------------------
//Variable Declarations
//-----------------------
char val;                     // Value received from bluetooth serial
float cmol = 0;         	// Value for carbon monoxide detection ** Value needs to be written ** 
unsigned int setting = 0;       // Value deciding whether or not to turn off power when no slave is detected
boolean powr = false;
boolean conekt = false;
boolean calibrated = false; //** needs to be written to flash ** 
boolean safety_auto = false;
float min_value = 512.0;      //Needs to be determined though testing
long last1;
long last0;
//time_t sync_time;             // Value pulled from device to sync internal clock
int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};
Schedule blank;
Schedule actions[4] = {blank,blank,blank,blank}; //Arranged power off, on, safety off, on

//-----------------------
//--Function Prototypes--
//-----------------------
void change_power(const int& setting);
void ambient_light_check(const unsigned int& min_value);
void carbon_monoxide_check(const boolean& connected);
void parse_schedule(Schedule new_schedule);
time_t bluetooth_read_time();
void main_power_on();
void main_power_off();
void safety_lights_on();
void safety_lights_off();

//Set these pins as a software serial connection
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Initializations
void setup()
{
  Serial.begin(9600);             // Begin the serial monitor at 9600bps
  bluetooth.begin(9600);        // The Bluetooth Mate defaults to 115200bps
  pinMode(power, OUTPUT);         // Set power pin to output
  digitalWrite(power, LOW);       // Initalize power to off initially
  pinMode(led, OUTPUT);           // Set LED pin to output
  digitalWrite(led, LOW);         // Initalize the safety LEDs off initially
  pinMode(con,INPUT);             // Set the connection pin to input 
  pinMode(cmo, INPUT);
  pinMode(swit, INPUT);


  //**Read schedule data from EEPROM**
}

void loop()
{
//  if (bluetooth.available())
//    Serial.write(bluetooth.read());
  
  if(digitalRead(con) == HIGH) {
    last1 = millis();
  }
  else {
    last0 = millis();
  }
 
  if(last1 - last0 > 500) {
    conekt = true;
  }
  else {
   conekt = false; 
  }

  //Manual override for a physical switch to change the state of the device
  if(digitalRead(swit)==HIGH){
      if(powr == false){
          powr = true;
          digitalWrite(power, HIGH);
      }
      else{
        powr = false;
        digitalWrite(power,LOW);
      }
  }
  
  // if (Serial.available()) bluetooth.write(Serial.read()); //For debug//

  //Sync time from device and schedule 
  //time format hour,minute,second,day,month,year
  //if (conekt && timeStatus()== timeNotSet){ 
  //Send request for time from device
	//bluetooth.write("sync request"); // can be changed arbitrarily 
	//while(!bluetooth.available());
	//setTime(bluetooth_read_time());
  // }


  if(bluetooth.available())  // If the bluetooth sent any characters
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
        Serial.println("Turned on power! (Command)");
    } 
    // if the value received is L turn off power to the socket
    else if(val == 'L')
    {
        digitalWrite(power, LOW);
        powr = false;
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
      //Make sure that the LED's are on for the test
      if (!digitalRead(led)) digitalWrite(led,HIGH);

      //Averaging 3 values to be sure ** We can change the delay or eliminate completely if need be **
      //Needs testing, may need to add some onto this number to prevent it from oscilating
      min_value += analogRead(photor);
      delay(50);
      min_value += analogRead(photor);
      delay(50);
      min_value += analogRead(photor);
      min_value /= 3;
      Serial.println("Calibration Successful");
	  calibrated = true;
      digitalWrite(led,LOW);
    }

    //--------------Setting Changes-----------------

    //--------------Safety Light Settings----------- ******************* don't let it set to auto without calibration, calibration needs to be written to flash***************
    else if (val == 4 && calibrated){
      safety_auto = true;
      Serial.println("Safety LED lights auto mode");
    }
    else if (val == 5){
	  safety_auto = false;
      Serial.println("Safety LED lights manual mode");
    }

    //--------------Power Settings-------------------
    // if the value received is O change setting to 0, If the use wants the device to turn off when they go out of range
    else if(val == '1')
    {
        setting = 0;
        Serial.println("Changed setting to 0");
    }
    // if the value received is H change setting to 1, If the use wants the device to turn on when they go out of range
    else if(val == '2')
    {
        setting = 1;
        Serial.println("Changed setting to 1");
    }
    // if the value received is A change setting to 2, If the user wants the device to retain it's current power state when they go out of range
    else if(val == '3')
    {
      setting = 2;   
      Serial.println("Changed setting to 2"); 
    }
	//-------------Scheduling-----------------------
	else if (val = '(')
	{
		Schedule new_schedule = parse_schedule();
        //Store in EEPROM
	}
	//Remove a schedule, just sets ticks = 0;
	else if (val = 'R'){
		val = bluetooth.read();
		actions[(int)val].ticks = 0;
	}



    // delay before next command occurs 
    delay(50);
  }

  // Turn outlet on or off when bluetooth disconnects depending on setting set by user. 
  else if (!conekt) change_power(setting); 
  //Turn on or off the safety lights depending on if safety auto is on and it is darker than the minimum
  else if (safety_auto) ambient_light_check(min_value);
  //CO check
  carbon_monoxide_check(conekt);
  //Scheduling checks 
  for (int i=0; i < 4; ++i){
	if (actions[i].ticks > 0) actions[i].check_time();
  }
  
}
//Functions for Scheduling 
//Main power options change out of range setting to constant when called by Scheduler
void main_on(){
	digitalWrite(power, HIGH);
    powr = true;
	setting = 3;
    Serial.println("Turned on power! (Schedule)");
}
void main_off(){
	digitalWrite(power, LOW);
    powr = false;
	setting = 3;
    Serial.println("Turned off power! (Schedule)");
}
void safety_on(){
	digitalWrite(led, HIGH);
	safety_auto = false;
    Serial.println("Turned on safety lights! (Schedule)");
}
void safety_off(){
	digitalWrite(led, LOW);
	safety_auto = false;
    Serial.println("Turned off safety lights! (Schedule)");
}

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
//For next, 0 = main on, 1 = main_off, 2 = safety_on, 3 = safety_off
Schedule parse_schedule(){
	int next = Serial.parseInt();
	time_t start = bluetooth_read_time();
	long interval = Serial.parseInt();
	time_t end = bluetooth_read_time();
	//This is the length of time that the schedule will be on for
	double length = (end - start)/60; //Return is in seconds
	//If timer, make ticks 1
    unsigned int ticks = 1;
	//Otherwise how many times the device will have to do the function. length of time of schedule / how often to do the action
	if (length) ticks = (int)ceil(length/interval);
	//Set the schedule to the parsed data from Serial, last pass is function
    Schedule return_schedule;
	//Changes function based on next value passed and returns the schedule
	if (next == 0) return_schedule.create(start, interval, ticks, main_on);
	else if (next == 1) return_schedule.create(start, interval, ticks, main_off);
	else if (next == 2) return_schedule.create(start, interval, ticks, safety_on);
	else return_schedule.create(start, interval, ticks, safety_off);
	return return_schedule;
}



//Function to change the state of the main power based on the passed setting 
void change_power(const int& set){
  if (set < 2){
    if (set == 0  && powr){
      digitalWrite(power, LOW);
      Serial.println("Turned off power! (Disconnect)");
    }
    else if (setting == 1 && !powr){
      digitalWrite(power, HIGH);
      Serial.println("Turned on power! (Disconnect)");
    }
    //Set 0 will write low and Set 1 will write high
    powr = set;
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
  
  // print out the value you read:
  //Serial.println(voltage);
  //Serial.println(sensorValue);
}

//-----------------------------------------------------------
//----------------Carbon Monoxide Detection------------------
//-----------------------------------------------------------


//All the code for carbon monoxide detection (needs some tweaks)
//if carbon monoxide is detected, store that it is

void carbon_monoxide_check(const boolean& connected){
  if(analogRead(cmo) > 512){ //512 is arbitrary 
	cmol = true;
	if (connected) bluetooth.println('1');
  } 
  //otherwise store that it is not detected
  else cmol = false;
}


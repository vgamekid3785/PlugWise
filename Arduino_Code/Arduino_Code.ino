#include <SoftwareSerial.h>  
#include <Time.h>
#include <TaskAction.h>
#include <Schedule.h>
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
float min_value = 0;      //Needs to be determined though testing
long last1;
long last0;
//time_t sync_time;             // Value pulled from device to sync internal clock
int days[12] = {31,28,31,30,31,30,31,31,30,31,30,31};

//-----------------------
//--Function Prototypes--
//-----------------------
void change_power(const int& setting);
void ambient_light_check(const unsigned int& min_value);
void carbon_monoxide_check(const boolean& connected);


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
  // pinMode(cmo, INPUT);
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
  
  // if (Serial.available()) bluetooth.write(Serial.read()); //For debug//

  //Sync time from device and schedule 
  //time format hour,minute,second,day,month,year
  //if (conekt && timeStatus()== timeNotSet){ //*******need to fix timeStatus and timeNotSet
  //Send request for time from device
	//bluetooth.write("sync request"); // can be changed arbitrarily 
	//while(!bluetooth.available());
	//**this may or may not work** Snyc time from device
	//sync_time(bluetooth.parseInt(),bluetooth.parseInt(),bluetooth.parseInt(),bluetooth.parseInt(), bluetooth.parseInt(), bluetooth.parseInt());
    //setTime(sync_time);
	
    //sync the current timing schedules as well 
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
    else if(val == 'S' && !safety_auto)
    {
        digitalWrite(led, HIGH);
        Serial.println("Turned on safety lights!");
    } 
    // if the value received is U turn off the safety lights
    else if(val == 'U' && !safety_auto)
    {
        digitalWrite(led, LOW);
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
		//schedule new_schedule;
		//parse_schedule(new_schedule);
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
  
  //---------------------------------------------
  //---------------Tasks Section-----------------
  //---------------------------------------------
  /*
    //Tasks are either to turn something on, off or both
    //Repeated or one time reflected in the schedule end / interval 
    //timer is essentially identical, except schedule start is current time + timer time and interval = 0;
  
    //In set up, initialize the three places in flash memory to tasks with NULL functions just so there's something there and we don't risk calling anything that doesn't exist

      //Settings for calendar events (task_action.h)
    
        //Receiving Tasks from bluetooth
        //tuple (type,schedule_start,interval, schedule end, device to change)

        types = off || on                       			  //Timer functionality will change the tuple (Android dev)
        schedule start = (minute,hour,day,month,year);        //Don't allow to be set in the past, set to current time + timer time both this and next on android end
        schedule end = (minute,hour,day,month,year);          //Don't allow to be set in the past, set to current time + timer time 
        interval = amount of time between function calls      //lowest user input is minutes, max is monthly (30 days, 43829 minutes), 0 if timer and set into different flash memory
        device = safety_lights || power;					  //Not implemented in final version for reason of photoresistor. Future functionality
        private ticks = floor(end-start / interval);

		//Function parses the schedule that was just sent by the bluetooth 
		schedule parse_schedule(schedule& new_schedule){
			int next = bluetooth.parseInt();
			//****************************************
			//If this DOES NOT eliminate the next non numeric char from the buffer this WILL NOT WORK
			//Another read will need to be implemented to clear that char
			//****************************************
			time_t start = bluetooth_read_time();
			int interval = bluetooth.parseInt();
			time_t end = bluetooth_read_time();
			//This is the length of time that the schedule will be on for
			double length = difftime(end,start)/60; //Return is in seconds
			//How many times the device will have to do the function. length of time of schedule / how often to do the action
			int ticks = length/interval;
			//Set the schedule to the parsed data from bluetooth, last pass is what type of function it is: on or off
			parsed.create(start, interval, ticks, next);
			return parsed; 
		}

        //options =
          //Schedule
          //(off,schedule_start,interval, schedule end, device)
          //(on,schedule_start,interval, schedule end, device)
          //(on+off,schedule_start_on, interval_on, schedule_end_on, schedule_end_on, schedule_start_off, Interval_off, schedule_end_off,device)
            //****on off translates to an on schedule and an off schedule although it is passed as one (can be passed as two if easier for android dev, probably best option **jake input**)****

          //Timer; 
          //(off,schedule_start, 0, schedule start, device)
          //(on,schedule_start, 0, schedule start, device)

        //3 places in flash memory
          //1 for schedule off
          //1 for schedule on
		  //Maybe safety light on and off implementation
          //1 for timer on/off ?maybe 2

        //When data is received, overwrite flash memory, don't change any parameters of the current task, prompt user to overwrite if there is currently a schedule. 
    //After writing the data to memory, set state of task to false so it doesn't immediately start. 
    // **Ask Jake** (Could change the constructor to default to false, seems like best solution)
    //task.tick() is only a check to see if it should call the function again based on the last time it was called, current time, and the interval
    
      //Running Tasks
    for task in schedule/timer
      if (current time > schedule start time && !task.GetCurrentState):
        task.Enable(); 
      task.tick()
    //----------
    //Misc notes
    //----------
    //If user wants to remove a schedule, change the function to NULL so nothing will ever be called if it gets it; 
  */
}



//Function to change the state of the main power based on the passed setting 
void change_power(const int& set){
  if (set < 2)
  {
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


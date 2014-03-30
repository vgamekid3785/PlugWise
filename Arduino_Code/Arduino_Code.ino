#include <SoftwareSerial.h>  
#include <Time.h>

//------------------------
//Pin Declarations 
//------------------------
#define bluetoothTx 11  // TX-O pin of bluetooth mate, Arduino D2
#define bluetoothRx 10  // RX-I pin of bluetooth mate, Arduino D3
#define power 9        // Pin used to turn power on an off
#define led 13         // Pin used to control safety LEDs
//#define cmo 12         // Pin used to read from the carbon monoxide detector
#define con 6          // check if the BT module is paired

//-----------------------
//Variable Declarations
//-----------------------
char val;                     // Value received from bluetooth serial
boolean cmol = false;         // Value for carbon monoxide detection
unsigned int setting = 0;       // Value deciding whether or not to turn off power when no slave is detected
boolean powr = false;
boolean conekt = false;
long last1;
long last0;
//time_t sync_time;             // Value pulled from device to sync internal clock

void change_power(const int& setting);

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
  if (conekt){
    //Send request for time from device
    //sync_time = time from device
    //setTime(sync_time);
    //sync the current timing schedules as well 
  }


  if(bluetooth.available())  // If the bluetooth sent any characters
  {
    val = bluetooth.read();
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
        Serial.println("Turned on safety lights!");
    } 
    // if the value received is U turn off the safety lights
    else if(val == 'U')
    {
        digitalWrite(led, LOW);
        Serial.println("Turned off safety lights!");
    } 
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

    //Tasks Section
    /*
      //Tasks are either to turn something on, off or both
      //Repeated or one time reflected in the schedule end / interval 
      //timer is essentially identical, except schedule start is current time + timer time and interval = 0;
	  
	  //In set up, initialize the three places in flash memory to tasks with NULL functions just so there's something there and we don't risk calling anything that doesn't exist

      //Settings for calendar events (task_action.h)
	  
        //Receiving Tasks from bluetooth
        //tuple (type,schedule_start,interval, schedule end, device to change)

        types = off || on || on/off                           //Timer functionality will change the tuple (Android dev)
        schedule start = (minute,hour,day,month,year);        //Don't allow to be set in the past, set to current time + timer time 
        schedule end = (minute,hour,day,month,year);          //Don't allow to be set in the past, set to current time + timer time
        interval = amount of time between function calls      //lowest user input is minutes, max is monthly (30 days, 43829 minutes), 0 if timer and set into different flash memory
        device = safety_lights || power;
        
        private ticks = floor(end-start / interval);

        //How to initiate task 
        Taskaction task(function, interval, ticks);

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

    // delay before next command occurs 
    delay(50);
  }

  // Turn outlet on or off when bluetooth disconnects depending on setting set by user. 
  else if (!conekt) change_power(setting);

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
    powr = set;
  }
}



//-----------------------------------------------------------
//--------------------Photoresistor--------------------------
//-----------------------------------------------------------

/*
int sensorValue = analogRead(12);
// Convert the analog reading (which goes from 0 - 1023) to a voltage (0 - 5V):
float voltage = sensorValue * (5.0 / 1023.0);
// print out the value you read:
Serial.println(voltage);
Serial.println(sensorValue);
delay(500);
*/

//-----------------------------------------------------------
//----------------Carbon Monoxide Detection------------------
//-----------------------------------------------------------
/*
//All the code for carbon monoxide detection (needs some tweaks)
if(Serial.available())  // If stuff was typed in the serial monitor
{
  //if the board has detected carbon monoxide, tell the serial connection
  if(cmol == true){
      bluetooth.println('1');
  }
  // Send any characters the Serial monitor prints to the bluetooth
  bluetooth.println((char)Serial.read());
  //delay before next command occurs
  delay(50);
  //if carbon monoxide is detected, store that it is
  if(digitalRead(cmo) == HIGH){
    cmol = true;
  } 
  //otherwise store that it is not detected
  else {
    cmol = false;
  }
}
*/
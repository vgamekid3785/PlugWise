
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
    //Optional: send query to user to verify time.
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

    //Settings for calendar events (task_action.h)
      //Receive Data type tuple
      //(off,schedule start((minute,hour,day,month,year), interval(in minutes, max is monthly), schedule end(minute,hour,day,month,year))
      //(on,time schedule start((minute,hour,day,month,year), interval(in minutes, max is monthly), schedule end(minute,hour,day,month,year))
      //(on+off, schedule start((minute,hour,day,month,year), interval_tostayon(in minutes, max is monthly),interval_tostayoff(in minutes, max is monthly) schedule end(minute,hour,day,month,year))
      //(least common, maybe not implement) (off+on, schedule start((minute,hour,day,month,year), interval_tostayoff(in minutes, max is monthly),interval_tostayon(in minutes, max is monthly) schedule end(minute,hour,day,month,year)))
      //when confirming schedule, cross check all other time schedules to make sure of no conflicts.

    // delay before next command occurs 
    delay(50);
  }

  // Turn outlet on or off when bluetooth disconnects depending on setting set by user. 
  else if (!conekt && setting < 2) 
  {
    if (setting == 0  && powr){
      digitalWrite(power, LOW);
      Serial.println("Turned off power! (Disconnect)");
    }
    else if (setting == 1 && !powr){
      digitalWrite(power, HIGH);
      Serial.println("Turned on power! (Disconnect)");
    }
    powr = setting;
  }

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
  }
  

  //if carbon monoxide is detected, store that it is
  if(digitalRead(cmo) == HIGH){
    cmol = true;
  } 
  //otherwise store that it is not detected
  else {
    cmol = false;
  }

  */
  // and loop forever and ever!
}

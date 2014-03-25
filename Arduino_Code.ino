
#include <SoftwareSerial.h>  
#include <Time.h>

//--------------------
//Pin Assignments
//--------------------

#define bluetoothTx 11  // TX-O pin of bluetooth mate, Arduino D2
#define bluetoothRx 10  // RX-I pin of bluetooth mate, Arduino D3
#define power 9        // Pin used to turn power on an off
#define led 13         // Pin used to control safety LEDs
#define cmo 12         // Pin used to read from the carbon monoxide detector
#define con 6          // check if the BT module is paired


//---------------------
//Variable Definitions
//---------------------

char val;                     // Value received from bluetooth serial
boolean cmol = false;         // Value for carbon monoxide detection
boolean setting = true;       // Value deciding whether or not to turn off power when no slave is detected
boolean powr = false;
boolean conekt = false;
long last1;
long last0;
time_t sync_time;             // Value pulled from device to sync internal clock

//Declare a serial connection over these pins
SoftwareSerial bluetooth(bluetoothTx, bluetoothRx);

//Initialization
void setup()
{
  Serial.begin(9600);             // Begin the serial monitor at 9600bps
  bluetooth.begin(9600);        // The Bluetooth Mate defaults to 115200bps
  //  bluetooth.print("$");           // Print three times individually
  //  bluetooth.print("$");
  //  bluetooth.print("$");           // Enter command mode
  //  delay(100);                     // Short delay, wait for the Mate to send back CMD
  //  bluetooth.println("U,9600,N");  // Temporarily Change the baudrate to 9600, no parity. 115200 can be too fast at times for NewSoftSerial to relay the data reliably
  //  bluetooth.begin(9600);          // Start bluetooth serial at 9600
  pinMode(power, OUTPUT);         // Set power pin to output
  digitalWrite(power, LOW);       // Initialize power to off
  pinMode(led, OUTPUT);           // Set led pin to output
  digitalWrite(led, LOW);         // Initialize LED to off
  pinMode(con,INPUT);             // Set 
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

  //Sync time from device
  if (conekt){
    //Send request for time from device
    //sync_time = time from device
    // 


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
    // if the value received is O turn the setting off, If the user wants the device to remain on when they go out of range
    else if(val == 'O')
    {
      setting = false;    
    }
    // if the value received is C turn the setting on, If the use wants the device to turn off when they go out of range
    else if(val == 'C')
    {
        setting = true;
    }
    // delay before next command occurs 
    delay(50);
  }

  // Turn outlet on or off when bluetooth disconnects depending on setting set by user. 
  else if (!conekt) 
  {
    if (!setting){
      digitalWrite(power, LOW);
      Serial.println("Turned off power! (Disconnect)");
    }
    else {
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

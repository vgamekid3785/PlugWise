#include <SoftwareSerial.h>  
#include <Time.h>
#include <TaskAction.h>

int LedPin = 13; //Whatever you want 
int Inverval = 3000; //3 seconds just to be sure, arbitrary
int count = 0;
bool LedState = false;

//Testing to see how ".tick()" works. Essentially if it calls delay then function, function then delay, or if it calls function and then calls it again after time has passed (not delay(x))

TaskAction task(blinkLed, Inverval, INFINITE_TICKS);

void blinkLed()
{
  LedState = !LedState;
  digitalWrite(LedPin, LedState);
}

void setup()
{
  pinMode(LedPin, OUTPUT);
  Serial.begin(9600);
}

void loop()
{
  if (count == 0) delay(15000); //Give you some time to open the serial connection
  Serial.println("first");
  task.tick();
  Serial.println("second");
  count += 1;
}
#define photor A5   
#define led 13

int min_value = 0;


void setup(){
	Serial.begin(9600);
        pinMode(photor,INPUT);
	pinMode(led,OUTPUT);
}

void loop(){
        while(!Serial.available());
        char a = Serial.read();
        if (a == 'H') digitalWrite(led,HIGH);
        if (a == 'L') digitalWrite(led,LOW);
        Serial.println(digitalRead(led));
        //Serial.println(voltage);
        //Serial.println(analogRead(photor));
	//ambient_light_check(min_value);
}


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
  
}

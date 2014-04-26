#define swit 9
#define power 8

boolean powr = false;
boolean debounce = false;

void setup(){
	Serial.begin(9600);
        pinMode(9,INPUT);
        pinMode(8,OUTPUT);
}

void loop(){
        Serial.println(digitalRead(swit));
	if(digitalRead(swit)==HIGH){
		debounce = false;
	}
	//Manual override for a physical switch to change the state of the device
	if(digitalRead(swit)== LOW && !debounce){
	  debounce = true;
	  if(powr == false){
		  Serial.println("Changing power\n%%%%%%%55\n%%%%%%%%%%%%%%\n%%%%%%%%%%%%%");
		  powr = true;
		  digitalWrite(power, HIGH);
	  }
	  else{
		powr = false;
		digitalWrite(power,LOW);
	  }
	}
}

/*
void loop(){
        Serial.println(analogRead(swit));
	if(analogRead(swit)>0){
		debounce = false;
	}
	//Manual override for a physical switch to change the state of the device
	if(analogRead(swit)==0 && !debounce){
	  debounce = true;
	  if(powr == false){
		  Serial.println("Changing power\n%%%%%%%55\n%%%%%%%%%%%%%%\n%%%%%%%%%%%%%");
		  powr = true;
		  digitalWrite(power, HIGH);
	  }
	  else{
		powr = false;
		digitalWrite(power,LOW);
	  }
	}
delay(250);
}
*/

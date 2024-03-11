//Pin assignment for inputs
int te1_pin = A0;
int te2_pin = A1;
int diode = 0;//143 if diode is used


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(3,OUTPUT);
  pinMode(2,OUTPUT);
  pinMode(A0,INPUT);
  pinMode(A1,INPUT);

}

void loop() {
    int te1 = diode + analogRead(te1_pin);
    int te2=analogRead(te2_pin);

    float te2Volts = voltConverter(te2);

    if (te2Volts < 0.5 || te2Volts > 4.5){
      digitalLow();
    }
    else {
      float output = deviationCheck(te1,te2);
      if (output < 10) {
        digitalHigh();
      }
      else {
        digitalLow();
      }
  }

}

float voltConverter(int x){
  float converted = (abs(x)*5/1023);
  return converted;
}

float deviationCheck(int x, int y){
  float dev = (abs(x-y)*100/1023);
  return dev;
}

void digitalLow(){
  digitalWrite(2,LOW);
  digitalWrite(3,LOW);
}

void digitalHigh(){
  digitalWrite(2,HIGH);
  digitalWrite(3,HIGH);
}




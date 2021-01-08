#include "MQ135.h"
void setup (){
Serial.begin (9600);
}
void loop() {
MQ135 gasSensor = MQ135(A0); // Sensör pini A0 analog pine bağlanır. 
float rzero = gasSensor.getRZero();
Serial.println (rzero);
delay(1000);
}

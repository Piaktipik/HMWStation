

boolean loopRxMode = false;
char dataIn;

void setup() {
  Serial.begin(9600);
  Serial1.begin(19200);
}

void loop() {
  if (Serial.available()) {      // If anything comes in Serial (USB),
    // we check if the last data sended were the LOOP comand to activate the decoder
    
    Serial1.write(Serial.read());   // read it and send it out Serial1 (pins 0 & 1)
  }

  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
    dataIn = Serial1.read();
    Serial.write(dataIn);   // read it and send it out Serial (USB)
    Serial.println();
    Serial.println(dataIn);
    Serial.print(dataIn,DEC);
  }
}

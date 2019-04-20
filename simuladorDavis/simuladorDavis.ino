void setup() {
  // put your setup code here, to run once:
  Serial.begin(19200);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Serial.available() > 1) {
    // Enviamos Simulacion pa quete Davis
    Serial.println("LOO�&�s ����������������?�����������x �������s�c ");
    Serial.flush();
    while (Serial.available() > 1) {
      Serial.read();
    }
  }
  delay(100);
}

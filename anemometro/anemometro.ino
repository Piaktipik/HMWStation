// Prueba Anemometro 


// Pin 3 conectado a anemometro -> pinAnemometro:
int pinAnemometro = 3;

// Tiempo entre pulsos del anemometro usado para calcular la velocidad del tiempo medida
unsigned long tAnemometro = 0;
float periodoAne = 0;

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  // make the pinAnemometro's pin an input:
  pinMode(pinAnemometro, INPUT);

  //Activamos interrupcion
  attachInterrupt(digitalPinToInterrupt(pinAnemometro), anemometro, FALLING);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input pin:
  //int buttonState = digitalRead(pinAnemometro);
  // print out the state of the button:
  Serial.println(periodoAne);
  delay(1);        // delay in between reads for stability
}

void anemometro() {
  periodoAne = float(millis()-tAnemometro);
  tAnemometro = millis();
}


// ---------------------- Variables ----------------------
boolean loopRxMode = false;
char dataIn;
char dato[110];
char datosE[99];
unsigned long TiempoSolicitud1 = 0;
unsigned long TiempoSolicitud2 = 0;
byte contDato = 0;
unsigned long wTimeOut1 = 0;
boolean debug = false;

// Variables decodificacion datos

// ------------------------------ Confuguraciones Iniciales ------------------------------
void setup() {
  Serial.begin(115200);
  Serial1.begin(19200);
}

// ------------------------------ Ciclo Inifinito ------------------------------
void loop() {

  // Verificamos datos enviados por Serial
  revisarSerial();

  // Si llega algun dato por serial de la consola Davis, se procesa
  if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)

    if (loopRxMode) {
      capturarPaqueteTest();
    } else {
      capturarPaqueteLoop();
    }

  } // Fin datos recividos Davis

  // Solicitud Datos:
  if (TiempoSolicitud2 + 1000 < millis()) {
    TiempoSolicitud2 = millis();
    Serial.println();
    Serial.print("Contador: ");
    Serial.print(contDato);
    Serial.println(" Dato: ");
    int j = 0;
    for (int i = 0; i < contDato; i++) {
      // llenamos datos 
      if(i>2){
        datosE[j] = dato[i];
        j++;
      }
      //Serial.print(dato[i]);
    }
    // mostramos vector datos trama Loop
    for (int i = 0; i < j; i++) {
      Serial.print(datosE[i]);
    }

    Serial.println();
    Serial.println("--------------------------------------------");
    contDato = 0;

    if (debug)Serial.println(" C8 ");

    // Se solicita nuevo paquete
    Serial1.print(char(10));
    Serial1.print(char(10));
    delay(100);
    Serial1.println("LOOP 1");
  }

}

// ------------------------------ SubFunciones  ------------------------------
inline void revisarSerial() {
  if (Serial.available() > 1) {
    // Cambiamos modo de visualizacion
    dataIn = Serial.read();
    if (dataIn == 'v') {
      debug = !debug;
    } else {
      // Se reenvia cualquier dato que se envie por serial
      Serial1.write(dataIn);   // read it and send it out Serial1 (pins 0 & 1)
    }
  }
}

inline void capturarPaqueteTest() {

  if (debug)Serial.print(" C1 ");
  dataIn = Serial1.read();

  // el primer caracter del paquete es char(13) y el ultimo char(10)
  if (dataIn == char(13)) {
    Serial.println();
    Serial.println("Inicio paquete Detectado: ");
    delay(100); // esperamos que lleguen el resto de paquetes
    wTimeOut1 = millis() + 20000; // Se da un tiempo de 1 segundo de espera

    while (Serial1.available() && (wTimeOut1 > millis())) {
      if (debug)Serial.print(" C2 ");
      dataIn = Serial1.read();
      // verificamos que el caracter que llego no sea el de fin
      if (dataIn == char(10)) {
        loopRxMode = false;
        if (debug)Serial.print(" C3 ");
        break;
      }
      // guardamos cada dato que llego
      dato[contDato] = dataIn;
      contDato++;
      if (debug)Serial.print(" C4 ");
      delay(100);
    }
    // mostramos datos recogido:
    Serial.println();
    Serial.print("ConteoDatos: ");
    Serial.print(contDato);
    Serial.print(" Dato: ");
    for (int i = 0; i < contDato; i++) {
      Serial.print(dato[i]);
    }
    contDato = 0;
    Serial.println();
    if (debug)Serial.print(" C5 ");
  }

  // Solicitud Datos:
  //  if (TiempoSolicitud1 + 2000 < millis()) {
  //    TiempoSolicitud1 = millis();
  //    if(debug)Serial.println(" C7 ");
  //    Serial1.println("TEST");
  //    loopRxMode = true;
  //  }
}

inline void capturarPaqueteLoop() {
  if (debug)Serial.print(" C6 ");
  if (debug)Serial.println();
  if (debug)Serial.print("Datos: ");

  // Se lee dato serial
  dataIn = Serial1.read();
  // se almacena en vector
  dato[contDato] = dataIn;
  contDato++;

  // Se imprime info por consola
  if (debug)Serial.print(contDato);
  if (debug)Serial.print(", ");
  Serial.write(dataIn);   // read it and send it out Serial (USB)
  if (debug)Serial.print(", ");
  if (debug)Serial.print(dataIn, DEC);

}

inline void procesarDatos(){
  
}

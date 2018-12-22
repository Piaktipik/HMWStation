
// ---------------------- Variables ----------------------
boolean loopRxMode = false;
char dataIn;
char dato[110];
byte datosE[99];
unsigned long TiempoSolicitud1 = 0;
unsigned long TiempoSolicitud2 = 0;
byte contDato = 0;
unsigned long wTimeOut1 = 0;
boolean debug = false;

// Variables decodificacion datos
byte tam;
byte Doff[45] = {0, 1, 2, 3, 4, 5, 7, 9, 11, 12, 14, 15, 16, 18, 25, 29, 33, 34, 41, 43, 44, 46, 48, 50, 52, 54, 56, 58, 60, 62, 66, 70, 71, 72, 74, 82, 86, 87, 89, 90, 91, 93, 95, 96, 97};
byte Dsize[45] = {1, 1, 1, 1, 1, 2, 2, 2, 1, 2, 1, 1, 2, 7, 4, 4, 1, 7, 2, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 4, 4, 1, 1, 2, 8, 4, 1, 2, 1, 1, 2, 2, 1, 1, 2};
float Val[45];
float DFact[45];
String DUnits[45];
bool DSave[45];

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
      if (i > 2) {
        datosE[j] = byte(dato[i]);
        Serial.print(j);
        Serial.print(", ");
        Serial.print(datosE[j]);   // read it and send it out Serial (USB)
        Serial.print(", ");
        Serial.println(dato[i], DEC);
        j++;
      }
      //Serial.print(dato[i]);
    }
    // mostramos vector datos trama Loop
    for (int i = 0; i < j; i++) {
      // Se imprime info por consola
      Serial.print(datosE[i]);   // read it and send it out Serial (USB)
    }

    Serial.println();
    Serial.println("--------------------------------------------");

    // se procesan los datos
    procesarDatos();

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

int mix2bytes(byte pos) {
  return datosE[pos + 1] * 256 + datosE[pos];
}

inline void procesarDatos() {
  tam = 44;
  for (int i = 0; i <= tam; i++) {
    byte pos = Doff[i];
    switch (pos) {
      case 3:
        //"Bar Trend";
        Val[i] = signed(datosE[pos]);
        break;
      case 4:
        //"Packet Type";
        Val[i] = datosE[pos];
        break;
      case 5:
        //"Next Record";
        Val[i] = mix2bytes(pos);
        break;
      case 7:
        //"Barometer";
        Val[i] = mix2bytes(pos);
        break;
      case 9:
        //"Inside Temperature";
        Val[i] = mix2bytes(pos);
        break;
      case 11:
        //"Inside Humidity";
        Val[i] = datosE[pos];
        break;
      case 12:
        //"Outside Temperature";
        Val[i] = mix2bytes(pos);
        break;
      case 14:
        //"Wind Speed";
        Val[i] = datosE[pos];
        break;
      case 15:
        //"10 Min Avg Wind Speed";
        Val[i] = datosE[pos];
        break;
      case 16:
        //"Wind Direction";
        Val[i] = mix2bytes(pos);
        break;
      case 18:
        //"Extra Temperatures";
        Val[i] = datosE[pos]; //soporta hasta 7 temperaturas - no implementado
        break;
      case 25:
        //"Soil Temperatures";
        Val[i] = datosE[pos]; //soporta hasta 4 temperaturas - no implementado
        break;
      case 29:
        //"Leaf Temperatures";
        Val[i] = datosE[pos]; //soporta hasta 4 temperaturas - no implementado
        break;
      case 33:
        //"Outside Humidity";
        Val[i] = datosE[pos];
        break;
      case 34:
        //"Extra Humidties";
        Val[i] = datosE[pos]; //soporta hasta 7 humedades - no implementado
        break;
      case 41:
        //"Rain Rate";
        Val[i] = mix2bytes(pos);
        break;
      case 43:
        //"UV";
        Val[i] = datosE[pos];
        break;
      case 44:
        //"Solar Radiation";
        Val[i] = mix2bytes(pos);
        break;
      case 46:
        //"Storm Rain";
        Val[i] = mix2bytes(pos);
        break;
      case 48:
        //"Start Date of current Storm";
        // funcion fecha 2 bytes ->
        // Bit 15 to bit 12 is the month
        // bit 11 to bit 7 is the day
        // bit 6 to bit 0 is the year offseted by 2000.
        break;
      case 50:
        //"Day Rain";
        Val[i] = mix2bytes(pos);
        break;
      case 52:
        //"Month Rain";
        Val[i] = mix2bytes(pos);
        break;
      case 54:
        //"Year Rain";
        Val[i] = mix2bytes(pos);
        break;
      case 56:
        //"Day ET";
        Val[i] = mix2bytes(pos);
        break;
      case 58:
        //"Month ET";
        Val[i] = mix2bytes(pos);
        break;
      case 60:
        //"Year ET";
        Val[i] = mix2bytes(pos);
        break;
      case 62:
        //"Soil Moistures";
        Val[i] = datosE[pos]; //soporta hasta 4 - no implementado
        break;
      case 66:
        //"Leaf Wetnesses";
        Val[i] = datosE[pos]; //soporta hasta 4 - no implementado
        break;
      case 70:
        //"Inside Alarms";
        Val[i] = datosE[pos];
        break;
      case 71:
        //"Rain Alarms";
        Val[i] = datosE[pos];
        break;
      case 72:
        //"Outside Alarms ";
        Val[i] = mix2bytes(pos);
        break;
      case 74:
        //"Extra Temp/Hum Alarms";
        Val[i] = datosE[pos];  //soporta hasta 8 - no implementado
        break;
      case 82:
        //"Soil & Leaf Alarms";
        Val[i] = datosE[pos];  //soporta hasta 4 - no implementado
        break;
      case 86:
        //"Transmitter Battery Status";
        Val[i] = datosE[pos];
        break;
      case 87:
        //"Console Battery Voltage";
        Val[i] = mix2bytes(pos); // Voltage = ((Data * 300)/512)/100.0 - no implementado
        break;
      case 89:
        //"Forecast Icons";
        Val[i] = datosE[pos];
        break;
      case 90:
        //"Forecast Rule number";
        Val[i] = datosE[pos];
        break;
      case 91:
        //"Time of Sunrise";
        Val[i] = mix2bytes(pos); // The time is stored as hour * 100 + min.
        break;
      case 93:
        //"Time of Sunset";
        Val[i] = mix2bytes(pos); // The time is stored as hour * 100 + min.
        break;
      case 95:
        //"<LF> = 0x0A";
        Val[i] = datosE[pos];
        break;
      case 96:
        //"<CR> = 0x0D";
        Val[i] = datosE[pos];
        break;
      case 97:
        //"CRC";
        Val[i] = mix2bytes(pos); // CRC - Verificar!
        break;

      default:
        //"";
        break;
    }
  }


  //Mostramos los datos procesados
  for (int i = 0; i <= tam; i++) {
    Serial.println();
    Serial.print(DField(Doff[i]));
    Serial.print("[");
    Serial.print(Doff[i]);
    Serial.print(",");
    Serial.print(Dsize[i]);
    Serial.print("-");
    Serial.print(DSave[i]);
    Serial.print("]: ");
    Serial.print(Val[i]);
    Serial.print(" ");
    Serial.print(DUnits[i]);
    Serial.print(" [");
    Serial.print(DFact[i]);
    Serial.print("]");
  }
  Serial.println();
}

String DField(int pos) {
  switch (pos) {
    case 0:
      return "L";
      break;
    case 1:
      return "O";
      break;
    case 2:
      return "O";
      break;
    case 3:
      return "Bar Trend";
      break;
    case 4:
      return "Packet Type";
      break;
    case 5:
      return "Next Record";
      break;
    case 7:
      return "Barometer";
      break;
    case 9:
      return "Inside Temperature";
      break;
    case 11:
      return "Inside Humidity";
      break;
    case 12:
      return "Outside Temperature";
      break;
    case 14:
      return "Wind Speed";
      break;
    case 15:
      return "10 Min Avg Wind Speed";
      break;
    case 16:
      return "Wind Direction";
      break;
    case 18:
      return "Extra Temperatures";
      break;
    case 25:
      return "Soil Temperatures";
      break;
    case 29:
      return "Leaf Temperatures";
      break;
    case 33:
      return "Outside Humidity";
      break;
    case 34:
      return "Extra Humidties";
      break;
    case 41:
      return "Rain Rate";
      break;
    case 43:
      return "UV";
      break;
    case 44:
      return "Solar Radiation";
      break;
    case 46:
      return "Storm Rain";
      break;
    case 48:
      return "Start Date of current Storm";
      break;
    case 50:
      return "Day Rain";
      break;
    case 52:
      return "Month Rain";
      break;
    case 54:
      return "Year Rain";
      break;
    case 56:
      return "Day ET";
      break;
    case 58:
      return "Month ET";
      break;
    case 60:
      return "Year ET";
      break;
    case 62:
      return "Soil Moistures";
      break;
    case 66:
      return "Leaf Wetnesses";
      break;
    case 70:
      return "Inside Alarms";
      break;
    case 71:
      return "Rain Alarms";
      break;
    case 72:
      return "Outside Alarms ";
      break;
    case 74:
      return "Extra Temp/Hum Alarms";
      break;
    case 82:
      return "Soil & Leaf Alarms";
      break;
    case 86:
      return "Transmitter Battery Status";
      break;
    case 87:
      return "Console Battery Voltage";
      break;
    case 89:
      return "Forecast Icons";
      break;
    case 90:
      return "Forecast Rule number";
      break;
    case 91:
      return "Time of Sunrise";
      break;
    case 93:
      return "Time of Sunset";
      break;
    case 95:
      return "<LF> = 0x0A";
      break;
    case 96:
      return "<CR> = 0x0D";
      break;
    case 97:
      return "CRC";
      break;

    default:
      return "";
      break;
  }
}


// ------------------------------------- Librerias -------------------------------------
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
//#include "Wire.h"
#include "SparkFunIMU.h"
#include "SparkFunLSM303C.h"
#include "LSM303CTypes.h"
// libreria SD
#include <SPI.h>
#include <SD.h>
// libreria GPS
#include "gps.h"
// librerias LCD



// ------------------------------------- Parametros -------------------------------------
#define LedSD_PIN 13
//////////////////////////// BAROMETRO
#define PBase 1023.4 // Baseline presure

//////////////////////////// SD
#define chipSelect 38

//////////////////////////// GPS
#define GPS_BAUDRATE  9600
#define gpsForceOn_PIN 25
#define gpsRESET_PIN 24
#define GPS_timeout 2000
#define DEBUG_GPS
static const uint32_t VALID_POS_TIMEOUT = 2000;  // ms


////////////////////////////  Gases
/* MQ-7 Carbon Monoxide Sensor Circuit with Arduino */
const int AOUTpin=0;//the AOUT pin of the CO sensor goes into analog pin A0 of the arduino
const int DOUTpin=48;//the DOUT pin of the CO sensor goes into digital pin D8 of the arduino
//const int ledPin=13;//the anode of the LED connects to digital pin D13 of the arduino

////////////////////////////  Anemometro
// Pin 3 conectado a anemometro -> pinAnemometro:
int pinAnemometro = 3;

////////////////////////////  LCD



// ------------------------------------- Variables -------------------------------------
int sensor = 2;
int temp, humedad;
int gota = 0;
boolean datosLimpios = true;
boolean meteoroPI = true;

//////////////////////////// BAROMETRO
char status;
double T, P, p0, a;
unsigned long tiempoA = 0;

//////////////////////////// SD
File dataFile;
bool sd_ok = false;

////////////////////////////  Gases
int value;

////////////////////////////  Anemometro
int dirAnemometro = 0;
// Tiempo entre pulsos del anemometro usado para calcular la velocidad del tiempo medida
unsigned long tAnemometro = 0;
float periodoAne = 0;

//////////////////////////// LCD

// ------------------------------------- Declaraciones de clases  -------------------------------------
DHT dht (sensor, DHT11);
SFE_BMP180 pressure;
LSM303C myIMU;

// ------------------------------------- Configuraciones -------------------------------------
void setup () {
  Serial.begin (9600);

  dht.begin();

  // Initialize the sensor (it is important to get calibration values stored on the device).
  if (pressure.begin())
    Serial.println("BMP180 init success");
  else
  {
    Serial.println("BMP180 init fail\n\n");
    //while (1); // Pause forever.
  }

  // inicializacion IMU
  if (myIMU.begin() != IMU_SUCCESS)
  {
    Serial.println("Failed setup.");
    //while (1);
  }

  // inicializacion SD
  Serial.print("Initializing SD card...");
  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    //while (1);
  }

  ///////////////////// Setup GPS
  Serial1.begin(GPS_BAUDRATE);
  Serial.print("Setup Gps...");
  gps_setup();
  Serial.println(" OK.");
  // Se reseteo del GPS
  pinMode(gpsRESET_PIN, OUTPUT);
  digitalWrite(gpsRESET_PIN, LOW);
  delay(50);
  pinMode(gpsRESET_PIN, INPUT);


  ///////////////////// Setup Gases
  pinMode(DOUTpin, INPUT);//sets the pin as an input to the arduino
  //pinMode(ledPin, OUTPUT);//sets the pin as an output of the arduino

  ///////////////////// Setup Anemometro
  // make the pinAnemometro's pin an input:
  pinMode(pinAnemometro, INPUT);
  // Activamos interrupcion
  attachInterrupt(digitalPinToInterrupt(pinAnemometro), anemometro, FALLING);
  
  ///////////////////// Setup LCD





  Serial.println("initialization done.");
}

// ------------------------------------- Ciclo -------------------------------------
void loop() {
  String datos = "";

  ///////////////////////////////////////////////////// Se lee el GPS
  read_gps();
  // Datos GPS
  datos += String(gps_time);
  datos += ","; datos += String(gps_lat * 10000);
  datos += ","; datos += String(gps_lon * 10000);
  datos += ","; datos += String(gps_altitude);

  datos += ","; datos += String(millis());

  humedad = dht.readHumidity();
  temp = dht.readTemperature();
  datos += ","; datos += String(temp);
  datos += ","; datos += String(humedad);

  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
      // Print out the measurement:
      datos += ","; datos += String(T);

      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);

        status = pressure.getPressure(P, T);
        if (status != 0) {
          datos += ","; datos += String(P);

          a = pressure.altitude(P, PBase);
          datos += ","; datos += String(a);


        }
        else if (!datosLimpios) {
          Serial.println("error retrieving pressure measurement\n");
        }
      }
      else if (!datosLimpios) {
        Serial.println("error starting pressure measurement\n");
      }
    }
    else if (!datosLimpios) {
      Serial.println("error retrieving temperature measurement\n");
    }
  }
  else if (!datosLimpios) {
    Serial.println("error starting temperature measurement\n");
  }


  gota = analogRead(A1);
  datos += ","; datos += String(gota);

  // Get all parameters
  datos += ","; datos += String(myIMU.readAccelX());
  datos += ","; datos += String(myIMU.readAccelY());
  datos += ","; datos += String(myIMU.readAccelZ());
  datos += ","; datos += String(myIMU.readMagX());
  datos += ","; datos += String(myIMU.readMagY());
  datos += ","; datos += String(myIMU.readMagZ());
  datos += ","; datos += String(myIMU.readTempC());

  //////////////////////////////// Se lee el Anemometro
  dirAnemometro = analogRead(A2);
  //Serial.println(dirAnemometro);
  datos += ","; datos += String(dirAnemometro);
  datos += ","; datos += String(periodoAne);

  //////////////////////////////// Se lee sensor Gases
  value = analogRead(AOUTpin); //reads the analaog value from the CO sensor's AOUT pin
  datos += ","; datos += String(value);
  //Serial.print("CO value: ");
  //Serial.println(value);//prints the CO value
  
  
  /////////////////////// Fin captura datos ///////////////////////

  // Cambiamos modo de visualizacion
  if (Serial.available() > 1) {
    if (Serial.read() == 'v') {
      datosLimpios = !datosLimpios;
    }
  }
  if (meteoroPI) {
    Serial.print('T');
    Serial.println(datos);
  }
  else {
    if (datosLimpios) {
      Serial.print("T");
      Serial.print(gps_time);
      Serial.print("A");
      Serial.print(gps_lat);
      Serial.print("O");
      Serial.print(gps_lon);
      Serial.print("H");
      Serial.print(gps_altitude);

      Serial.print("C");
      Serial.print(millis());

      Serial.print("T");
      Serial.print(temp);
      Serial.print("H");
      Serial.print(humedad);

      Serial.print("T");
      Serial.print(T, 2);

      Serial.print("P");
      Serial.print(P, 2);

      Serial.print("A");
      Serial.print(a, 0);

      Serial.print("G");
      Serial.print(gota);

      Serial.print("X");
      Serial.print(myIMU.readAccelX(), 4);
      Serial.print("Y");
      Serial.print(myIMU.readAccelY(), 4);
      Serial.print("Z");
      Serial.print(myIMU.readAccelZ(), 4);

      Serial.print("X");
      Serial.print(myIMU.readMagX(), 4);
      Serial.print("Y");
      Serial.print(myIMU.readMagY(), 4);
      Serial.print("Z");
      Serial.print(myIMU.readMagZ(), 4);

      Serial.print("T");
      Serial.println(myIMU.readTempC(), 4);

      Serial.print("D");
      Serial.println(dirAnemometro); // Direccion Anemometro

      Serial.print("A");
      Serial.println(periodoAne); // Velocidad Anemometro
      
      Serial.print("C");
      Serial.println(value); // prints the CO value

    } else {
      Serial.println("GPS");
      Serial.print("Tiempo: ");
      Serial.print(gps_time);
      Serial.print(" Latitud: ");
      Serial.print(gps_lat * 10000);
      Serial.print(" Longitud: ");
      Serial.print(gps_lon * 10000);
      Serial.print(" Altitud: ");
      Serial.print(gps_altitude);

      Serial.println("Tiempo");
      Serial.println(millis());

      Serial.println("DHT11" );
      Serial.print(" Temperatura: " );
      Serial.print(temp);
      Serial.print(" Humedad: ");
      Serial.print(humedad);
      Serial.println("%");

      Serial.println("Barometro");
      Serial.print(" Temperatura: ");
      Serial.print(T, 2);

      Serial.print(" Presion Absoluta: ");
      Serial.print(P, 2);
      Serial.print("mb");

      Serial.print(" Altitud: ");
      Serial.print(a, 0);
      Serial.println("m");

      Serial.println(" gota: ");
      Serial.println(gota);

      Serial.println(" IMU: ");
      Serial.print(" AX: ");
      Serial.print(myIMU.readAccelX(), 4);
      Serial.print(" AY: ");
      Serial.print(myIMU.readAccelY(), 4);
      Serial.print(" AZ: ");
      Serial.print(myIMU.readAccelZ(), 4);

      Serial.print(" MX: ");
      Serial.print(myIMU.readMagX(), 4);
      Serial.print(" MY: ");
      Serial.print(myIMU.readMagY(), 4);
      Serial.print(" MZ: ");
      Serial.print(myIMU.readMagZ(), 4);

      Serial.print(" Tem3: ");
      Serial.println(myIMU.readTempC(), 4);

      Serial.print("Direccion Anemometro: ");
      Serial.println(dirAnemometro); 

      Serial.print("Vel Anemometro");
      Serial.println(periodoAne); // Velocidad Anemometro
      
      Serial.print("CO value: ");
      Serial.println(value);//prints the CO value
      
    }
  }

  guardarStringSD(datos, "d");

  // Mostramos informacion por LCD

  // Esperamos a que pase un segundo para iniciar nueva captura:
  while (tiempoA + 999 >= millis()) {}
  tiempoA = millis();
} // fin loop

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

void anemometro() {
  periodoAne = float(millis()-tAnemometro);
  tAnemometro = millis();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void iniciarSd() {
  pinMode(chipSelect, OUTPUT);
  Serial.print(" ini.. ");
  if (SD.begin(chipSelect)) {
    Serial.println("OK ");
    sd_ok = true;

  } else {
    Serial.println("Err ");
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void read_gps() {
  // Get a valid position from the GPS
  int valid_pos = 0;
  uint32_t timeout = millis();
  char lecturagps;
  if (!meteoroPI)Serial.println("RGPS");
  gps_reset_parser();

  do {
    if (Serial1.available()) {
      lecturagps = Serial1.read();

      if (!meteoroPI)Serial.print(lecturagps);

      valid_pos = gps_decode(lecturagps);
    }
  } while ( (millis() - timeout < VALID_POS_TIMEOUT) && ! valid_pos) ;

}


//////////////////////////////////////////////////////////////////////////////////////////////////////
void guardarStringSD(String Datos, String NombreA) {
  String nombre = NombreA;
  nombre += String(gps_date);
  nombre += ".csv";
  if (sd_ok) {
    digitalWrite(LedSD_PIN, HIGH);
    // se guarda en la SD
    dataFile = SD.open(nombre, FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
      dataFile.println(Datos);
      dataFile.flush();
      dataFile.close();
      // led indicador guardado en SD

      // print to the serial port too:
      //Serial.print(datos);
      if (!meteoroPI) {
        Serial.print("On: ");
        Serial.print(nombre);
        Serial.print(": ");
        Serial.print(Datos);
      }
      //pitar(50);   //Pitido indicando que se guardo correctamente la informacion
    }
    // if the file isn't open, pop up an error:
    else {
      if (!meteoroPI) {
        Serial.print("error opening:");
        Serial.println(nombre);
        dataFile.close();
        sd_ok =  false;
      }
    }

  } else {
    iniciarSd(); // si sd no esta ok (sd_ok) tratamos de iniciarla nuevamente
  }
  digitalWrite(LedSD_PIN, LOW); // gurdado SD finalizado
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

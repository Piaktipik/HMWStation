
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

// ------------------------------------- Variables -------------------------------------
int sensor = 2;
int temp, humedad;
int gota = 0;
boolean datosLimpios = true;

//////////////////////////// BAROMETRO
char status;
double T, P, p0, a;
unsigned long tiempoA = 0;

//////////////////////////// SD
File dataFile;
bool sd_ok = false;

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
  if (datosLimpios) {
    Serial.print("T");
    Serial.print(gps_time);
    Serial.print("A");
    Serial.print(gps_lat);
    Serial.print("O");
    Serial.print(gps_lon);
    Serial.print("H");
    Serial.print(gps_altitude);
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
  }

  datos += ","; datos += String(millis());
  if (datosLimpios) {
    Serial.print("C");
    Serial.print(millis());
  } else {
    Serial.println("Tiempo");
    Serial.println(millis());
  }

  humedad = dht.readHumidity();
  temp = dht.readTemperature();
  datos += ","; datos += String(temp);
  datos += ","; datos += String(humedad);
  if (datosLimpios) {
    Serial.print("T");
    Serial.print(temp);
    Serial.print("H");
    Serial.print(humedad);
  } else {
    Serial.println("DHT11" );
    Serial.print(" Temperatura: " );
    Serial.print(temp);
    Serial.print(" Humedad: ");
    Serial.print(humedad);
    Serial.println("%");
  }

  status = pressure.startTemperature();
  if (status != 0) {
    delay(status);
    status = pressure.getTemperature(T);
    if (status != 0) {
      // Print out the measurement:
      datos += ","; datos += String(T);
      if (datosLimpios) {
        Serial.print("T");
        Serial.print(T, 2);
      } else {
        Serial.println("Barometro");
        Serial.print(" Temperatura: ");
        Serial.print(T, 2);
      }

      status = pressure.startPressure(3);
      if (status != 0) {
        delay(status);

        status = pressure.getPressure(P, T);
        if (status != 0) {
          datos += ","; datos += String(P);
          if (datosLimpios) {
            Serial.print("P");
            Serial.print(P, 2);
          } else {
            Serial.print(" Presion Absoluta: ");
            Serial.print(P, 2);
            Serial.print("mb");
          }

          a = pressure.altitude(P, PBase);
          datos += ","; datos += String(a);
          if (datosLimpios) {
            Serial.print("A");
            Serial.print(a, 0);
          } else {
            Serial.print(" Altitud: ");
            Serial.print(a, 0);
            Serial.println("m");
          }

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
  if (datosLimpios) {
    Serial.print("G");
    Serial.print(gota);
  } else {
    Serial.println(" gota: ");
    Serial.println(gota);
  }

  // Get all parameters
  datos += ","; datos += String(myIMU.readAccelX());
  datos += ","; datos += String(myIMU.readAccelY());
  datos += ","; datos += String(myIMU.readAccelZ());
  datos += ","; datos += String(myIMU.readMagX());
  datos += ","; datos += String(myIMU.readMagY());
  datos += ","; datos += String(myIMU.readMagZ());
  datos += ","; datos += String(myIMU.readTempC());

  if (datosLimpios) {
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
  } else {
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
  }

  // Cambiamos modo de visualizacion
  if (Serial.available() > 1) {
    if (Serial.read() == 'v') {
      datosLimpios = !datosLimpios;
    }
  }

  guardarStringSD(datos, "d");
  // Esperamos a que pase un segundo para iniciar nueva captura:
  while (tiempoA + 999 >= millis()) {}
  tiempoA = millis();
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
  Serial.println("RGPS");
  gps_reset_parser();

  do {
    if (Serial1.available()) {
      lecturagps = Serial1.read();
      Serial.print(lecturagps);

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
      Serial.print("On: ");
      Serial.print(nombre);
      Serial.print(": ");
      Serial.print(Datos);
      //pitar(50);   //Pitido indicando que se guardo correctamente la informacion
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.print("error opening:");
      Serial.println(nombre);
      dataFile.close();
      sd_ok =  false;
    }

  } else {
    iniciarSd(); // si sd no esta ok (sd_ok) tratamos de iniciarla nuevamente
  }
  digitalWrite(LedSD_PIN, LOW); // gurdado SD finalizado
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

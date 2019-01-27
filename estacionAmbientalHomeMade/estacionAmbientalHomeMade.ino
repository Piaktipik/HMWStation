
// ------------------------------------- Librerias -------------------------------------
#include <DHT.h>
#include <SFE_BMP180.h>
#include <Wire.h>
//#include "Wire.h"
#include "SparkFunIMU.h"
#include "SparkFunLSM303C.h"
#include "LSM303CTypes.h"
// libreria SD


// ------------------------------------- Parametros -------------------------------------
#define ALTITUDE 1651.0 // Altitude of SparkFun's HQ in Boulder, CO. in meters

// ------------------------------------- Variables -------------------------------------
int sensor = 2;
int temp, humedad;
int gota = 0;
boolean datosLimpios = true;
char status;
double T, P, p0, a;
double PBase = 1023.4; //Baseline presure
unsigned long tiempoA = 0;
double baseline; // baseline pressure

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

    // Get the baseline pressure:
//    baseline = getPressure();
//    
//    Serial.print("baseline pressure: ");
//    Serial.print(baseline);
//    Serial.println(" mb"); 
  else
  {
    // Oops, something went wrong, this is usually a connection problem,
    // see the comments at the top of this sketch for the proper connections.

    Serial.println("BMP180 init fail\n\n");
    //while (1); // Pause forever.
  }

  // inicializacion IMU
  if (myIMU.begin() != IMU_SUCCESS)
  {
    Serial.println("Failed setup.");
    //while (1);
  }

}

// ------------------------------------- Ciclo -------------------------------------
void loop() {
  if (datosLimpios) {
    Serial.print("C");
    Serial.print(millis());
  } else {
    Serial.println("Tiempo");
    Serial.println(millis());
  }

  humedad = dht.readHumidity();
  temp = dht.readTemperature();

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
          if (datosLimpios) {
            Serial.print("P");
            Serial.print(P, 2);
          } else {
            Serial.print(" Presion Absoluta: ");
            Serial.print(P, 2);
            Serial.print("mb");
          }

          a = pressure.altitude(P, PBase);
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
  if (datosLimpios) {
    Serial.print("G");
    Serial.print(gota);
  } else {
    Serial.println(" gota: ");
    Serial.println(gota);
  }

  // Get all parameters
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

  // Esperamos a que pase un segundo para iniciar nueva captura:
  while (tiempoA + 999 >= millis()) {}
  tiempoA = millis();
}

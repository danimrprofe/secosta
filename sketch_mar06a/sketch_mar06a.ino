/******************************************************************************
 EQUIP BAROGRAF llegeix a la vegada temps, temperatura i pressió atmosfèrica i ho escriu en una uSD

 components:
 1) Arduino uno
 2) Velleman vma202 datalogger
 
 3) adafruit bme 680
 Es tracta d'un sensor d'humitat, temperatura i pressió

 CODI PREPARAT PER A LA UNIVERSITAT DE LES ILLES BALEARS DINS EL PROJECTE 
 VENOM I DISTRIBUIT ALS INSTITUTS DE BATXILLERAT PARTICIPANTS. 

 Data última revisió 051219 
 joan@tremar.cat

 CONNEXIONS

 adafruit bme680                                            ARDUINO
1-Vin                                                    3.3V
2-3Vo                                                    --- 
3-GND                                                    GND
4-SCK                                                    SCL
5-SDO                                                    ---
6-SDI                                                    SDA
7-CS                                                     --

 ******************************************************************************/


//////////////////////////////////////////////////////////////////////
// COSES DE LA TARJA SD
/////////////////////////////////////////////////////////////////////

#include <SPI.h>
#include <SD.h> 

// set up variables using the SD utility library functions:
Sd2Card card;
SdVolume volume;
SdFile root;

const int chipSelect = 10; //CHIP SELECT 8 PER LA MICRO SD SHIFT DE SPARKFUN; ALTRES ADAFRUIT 10; VELLEMAN VMA202 10


// MAR_ACXX.dat, fins a 99.
#define LOG_FILE_PREFIX "BAROM" 
#define MAX_LOG_FILES 100 
#define LOG_FILE_SUFFIX "dat" 
#define MIDA_MAX 500000 // en bytes, canviar pel que convingui. 500000 són aprox 1 setmana a 1mn
char logFileName[13]; // Char string to store the log file name

#define CAPSALERA1 "barograf arduino venom 01"
#define CAPSALERA2 "data,hora,press(mb), temp (ºC),num_mostres"

String dades=""; //variable on ficarem les dades
File fitxer_dades;

////////////////////////////////////////////////
//COSES DEL RELLOTGE
///////////////////////////////////////////////
#include <Wire.h> //comunicacio i2c
#include "RTClib.h" //pel rellotge ds3107 de la velleman vm202
//RTC_DS1307 rtc;
RTC_PCF8523 rtc;  
DateTime ara;

/*
////////////////////////////////////
//COMUNICACIO SERIE
//////////////////////////////////
#include <SoftwareSerial.h>

#define TXpin 7   //Sortida d'aquest arduino
#define RXpin 6  //entrada a aquest arduino
SoftwareSerial marea_SS = SoftwareSerial(RXpin, TXpin);

*/

/////////////////////////////////////////////////////////////////
//COSES DELS SENSORS
//////////////////////////////////////////////////////////////////////
//BAROMETRE BMP280
#include "Adafruit_BMP280.h"


Adafruit_BMP280 bmp; // I2C

void setup()
{

      Serial.begin(9600);  //el port serie de l'arduino

      //////////////////////////////////////////////////////////////////////////////
      // COSES DEL RELLOTGE
      ///////////////////////////////////////////////////////////////////////////////
      Wire.begin();
      rtc.begin(); // inicialitza el rellotge
      
      ////////////////////////////////////////////////////////////////////////////
      // UN PETIT DIÀLEG PER QUAN ES CONNECTA UN PORT SÈRIE
      ///////////////////////////////////////////////////////////////////////////
      Serial.flush();
/*
      ara=rtc.now();
      Serial.print(ara.year(), DEC);
      Serial.print('/');
      Serial.print(ara.month(), DEC);
      Serial.print('/');
      Serial.print(ara.day(), DEC);
      Serial.print('-');
      Serial.print(ara.hour(), DEC);
      Serial.print(':');
      Serial.print(ara.minute(), DEC);
      Serial.print(':');
      Serial.print(ara.second(), DEC);
      Serial.println();
*/
      posa_hora();//Serial.flush();
/*
      ara=rtc.now();
      Serial.print(ara.year(), DEC);
      Serial.print('/');
      Serial.print(ara.month(), DEC);
      Serial.print('/');
      Serial.print(ara.day(), DEC);
      Serial.print('-');
      Serial.print(ara.hour(), DEC);
      Serial.print(':');
      Serial.print(ara.minute(), DEC);
      Serial.print(':');
      Serial.print(ara.second(), DEC);
      Serial.println();
*/  
    ////////////////////////////////////////////////////////////////////////////
    // CREO EL FITXER DE DADES 
    ///////////////////////////////////////////////////////////////////////////
      //MIRO SI LA TARJA SD HI ÉS
      if (!SD.begin(chipSelect)) {
        //Serial.println(F("ERROR TARJA SD"));
        return;
      }

     updateFileName(); // creem un nou fitxer i escrivim la capçalera
     File logFile = SD.open(logFileName, FILE_WRITE); // obrim el fitxer de dades, si ho fem bé escrivim les dades i el tanquem
     if(logFile) {
              logFile.println(logFileName);
              logFile.println(CAPSALERA1);
              logFile.println(CAPSALERA2);
              }
     logFile.close();

     ////////////////////////////////////////////////////////////////////////
     //POSO A PUNT EL SENSOR
     ///////////////////////////////////////////////////////////////////////

     if (!bmp.begin()) {
         Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
      while (1);
      }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

      
} //DEL SETUP


////////////////////////////////////////////////////////////////
//LOOOOOOP
///////////////////////////////////////////////////////////////

void loop()
{

    double pres, temp;
    int num_mostres=0;int jaerahora=HIGH;
    char any[4];char mes[3];char dia[3];char hora[3];char minut[3];char segon[3];
    String data="";
    String aux_str="";
    int i,minut_inici,minut_actual;
    int aa;int mm; int dd; int hh; int mn; int sg;
    //unsigned long temps,inici,actual;

    dades=""; //buido la variable que conté les dades abans de començar a llegir
    Serial.flush();

    //inici=millis(); 
    ara=rtc.now();
    minut_inici=ara.minute();
    //Serial.println(minut_inici);
/*
      Serial.print(ara.year(), DEC);
      Serial.print('/');
      Serial.print(ara.month(), DEC);
      Serial.print('/');
      Serial.print(ara.day(), DEC);
      Serial.print('-');
      Serial.print(ara.hour(), DEC);
      Serial.print(':');
      Serial.print(ara.minute(), DEC);
      Serial.print(':');
      Serial.print(ara.second(), DEC);
      Serial.println();
*/

  ///////////////////////////////////////////////
  //LLEGIM EL SENSOR 
  //////////////////////////////////////////////////
  int mostres = 0;
      while (mostres<10){                                //Anirà mesurant mentre l'alarma no faci canviar el flag jaerahora

          Serial.print("T: ");
          Serial.print(bmp.readTemperature());
          Serial.print("ºC  P: ");
          Serial.print(bmp.readPressure()/100);
          Serial.print("mbar Altitud: ");
          Serial.print(bmp.readAltitude(1009.00));
          Serial.println(" metres");
          temp+=bmp.readTemperature();
          pres+=bmp.readPressure()/100;//Serial.println(bmp.readPressure());
                    
          
          num_mostres ++; 
          delay(1000);   
          //LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF); //adormim per estalvi energia
          ara=rtc.now();
          
          mostres++;
          minut_actual=ara.minute();
          //Serial.println(minut_actual);
          //actual=millis();temps=actual-inici;
          if(minut_actual!=minut_inici) jaerahora = LOW; //i finalment comprovo si ja es hora d'escriure a la SD. guardo 35ms per esciure.
          //if(temps>=60000-35) jaerahora = LOW; //i finalment comprovo si ja es hora d'escriure a la SD. guardo 35ms per esciure.
      }
        
/////////////////////////////////////////////////////////
//EN SORTIR DEL BUCLE DE MESURA, ESCRIC LES DADES A LA SD, I SI EL COMPTADOR M'HO INDOCA HO ENVIO PEL PORT SERIE
//////////////////////////////////////////////////////////
      Serial.println();
      Serial.println(F("Ha sonat l'alarma, escric a la SD"));
      ara=rtc.now();
      temp/=num_mostres;//promitjo les mesures fetes
      pres/=num_mostres;
      

      aux_str=""; //buido la variable que conté les dades abans escriure de nou
      data="";

      String any_str=String(ara.year());
      //sprintf(any, "%04d", ara.year());
      sprintf(mes, "%02d", ara.month());sprintf(dia, "%02d", ara.day());sprintf(hora, "%02d", ara.hour());sprintf(minut, "%02d", ara.minute());sprintf(segon, "%02d", ara.second());
      data=(String(any_str)+String(mes)+String(dia)+String(hora)+String(minut)+String(segon));

      aux_str.concat(data);aux_str.concat(',');
      aux_str.concat(String(pres,2));aux_str.concat(',');
      aux_str.concat(String(temp,2));aux_str.concat(',');
      aux_str.concat(String(num_mostres));
    
      Serial.println(aux_str);              //escric al port serie hard per control
      data="";
      
     ////////////////////////////////////////
     //ESCRIC SD
     /////////////////////////////////////// 
     
     //primer miro la mida del fitxer, que no es passi
      unsigned long mida;
      File logFile = SD.open(logFileName, FILE_WRITE); // obrim el fitxer de dades, 
      if(logFile) {
        mida=logFile.size();
        //Serial.print(F("Mida del fitxer : "));Serial.println(mida);
      }
      logFile.close();
      if (mida > MIDA_MAX) {    
        updateFileName(); // si es passa, creem un nou fitxer i escrivim la capçalera
        File logFile = SD.open(logFileName, FILE_WRITE); // obrim el fitxer de dades, si ho fem bé escrivim la capçalera i el tanquem
        if(logFile) {
              logFile.println(logFileName);
              logFile.println(CAPSALERA1);
              logFile.println(CAPSALERA2);
              }
        logFile.close();
      }
    
        //i escrivim normalment
      logFile = SD.open(logFileName, FILE_WRITE); // obrim el fitxer de dades,       
     if(logFile) {logFile.println(aux_str);//Serial.println(F("Ole"));
     }
      logFile.close();
      delay(100);

/*
      /////////////////////////////////////////////////////////////////////////
      //ENVIO PEL PORT SERIE,
      //////////////////////////////////////////////////////////////////////
      // envio una cosa com ara: $BAROM,20190925114400,0.00,56


              Serial.println(F("ho envio pel port serie"));
              data.concat("$BAROM,");
              data.concat(aux_str);
              marea_SS.listen();
              marea_SS.println(data);   //escric al port serie soft per a l'arduino que hi estigui connectat
              marea_SS.flush();
              Serial.println(data);
              data="";
*/

} //DEL LOOP


// updateFileName() - Looks through the log files already present on a card,
// and creates a new file with an incremented file index.
void updateFileName()
{
  int i = 0;
  for (; i < MAX_LOG_FILES; i++)
  {
    memset(logFileName, 0, strlen(logFileName)); // Clear logFileName string
    // Set logFileName to "gpsbarXX.dat":
    sprintf(logFileName, "%s%d.%s", LOG_FILE_PREFIX, i, LOG_FILE_SUFFIX);
    if (!SD.exists(logFileName)) // If a file doesn't exist
    {
      break; // Break out of this loop. We found our index
    }
    else // Otherwise:
    {
 //     Serial.print(logFileName);
 //     Serial.println(" exists"); // Print a debug statement
    }
  }
  Serial.print(F("File name: "));
  Serial.println(logFileName); // Debug print the file name
}

  
  
void posa_hora() {
  int aa;int mm; int dd; int hh; int mn; int sg; 
  int c;
 
  if(Serial){
  Serial.println(F("H posa l'hora. tens 10s:"));
  }
  unsigned long ms=millis();
  while(!Serial.available() && (millis()-ms)<10000){delay(1);} //espero la comanda durant 10"
  while(Serial.available()) {
    c=Serial.read(); 
  }
  if (c=='H'||c=='h'){
  Serial.print(F("aaaa:"));  Serial.flush(); 
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    aa=Serial.parseInt();Serial.println(aa);  }
  Serial.print(F("mm:"));
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    mm=Serial.parseInt();Serial.println(mm);  }
  Serial.print(F("dd:"));
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    dd=Serial.parseInt();Serial.println(dd);  }
  Serial.print(F("hh:"));
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    hh=Serial.parseInt();Serial.println(hh);  }
  Serial.print(F("mn:"));
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    mn=Serial.parseInt();Serial.println(mn);  }
  Serial.print(F("sg:"));
  while(!Serial.available()){delay(1);} //espero la comanda
  while(Serial.available()) {    sg=Serial.parseInt();Serial.println(sg);  }
  rtc.adjust(DateTime(aa, mm, dd, hh, mn, sg));  
  } // de posar a l'hora
  else{}
}

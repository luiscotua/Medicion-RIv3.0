/*
* Project voyager-sensors
* Description: Read multiple sensors
* Author: Julio César
* Date: Apr 2018
* Edited: Alejandra Pérez, Reynaldo López, Luis Cotua y Jorge Causil
* Date: Nov 2018
*/

//Librerias a utilizar, por IDE particle
#include "Particle.h"
#include "Adafruit_HTU21DF.h"
#include "Adafruit_SSD1306.h"
#include "Adafruit_GFX.h"
#include "Adafruit_DHT.h"


//Definir Constante
#define OLED_RESET -1
#define LOG_PERIOD 15000
#define MAX_PERIOD 60000
#define RGB_A A1
#define RGB_B A2
#define RGB_C A3

//Variables
unsigned long counts;
unsigned long cpm;
unsigned int multiplier;
unsigned long previousMillis;
int cell_bars = 0;
char jsonString[200];
int sample = 0;
int ReadUVintensityPin = A2;

Adafruit_HTU21DF htu = Adafruit_HTU21DF();
Adafruit_SSD1306 display(OLED_RESET);


SYSTEM_THREAD(ENABLED);

STARTUP(RGB.mirrorTo(A4, A5, A7));

//Inicializar función setup
void setup() {
    
    //Sensor UV
    pinMode(ReadUVintensityPin, INPUT);

    beginDisplay();
    delay(1000);
    beginHTUSensor();
    delay(1000);
    beginGeigerSensor();
    delay(1000);

}

//Funcion contador Geiger
void setGeigerTubeImpulse(){
 counts++;
}

void beginGeigerSensor(){
 counts = 0;
 cpm = 0;
 multiplier = MAX_PERIOD / LOG_PERIOD;
 pinMode(D5, INPUT);
 attachInterrupt(D5, setGeigerTubeImpulse, FALLING);
}

//Sensor Tem-Hum
void beginHTUSensor(){
  htu.begin();
}

//Pantalla OLED
void beginDisplay(){
 display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
 display.display();
 display.clearDisplay();
 display.setTextSize(3);
 display.setCursor(0,0);
 display.setTextColor(WHITE);
 display.println("VOYAGER");
 display.setTextSize(3);
 display.setCursor(27,30);
 display.setTextColor(BLACK,WHITE);
 display.println(" V3 ");
 display.display();
 delay(8000);
 display.clearDisplay();
}


void collectData(){
 unsigned long currentMillis = millis();
 if(currentMillis - previousMillis > LOG_PERIOD){

     ++sample;
     previousMillis = currentMillis;
     cpm = counts * multiplier;
     counts = 0;

     float sv0 = cpm * 0.00884;
     float sv1 = cpm * 0.00662;


     int uvLevel = averageAnalogRead(ReadUVintensityPin);
     float outputVoltage = 5.0 * uvLevel/1024;
     float uvIntensity = mapfloat(outputVoltage, 0.99, 2.9, 0.0, 15.0);
     float rel_hum = htu.readHumidity();
     float temp = htu.readTemperature();


     display.setTextSize(1.5);
     display.setCursor(0,0);
     display.setTextColor(WHITE);
     display.println("sv0: " + String(sv0) + " µSv/h" );

     display.setTextSize(1.5);
     display.setCursor(0,12);
     display.setTextColor(WHITE);
     display.println("CPM: " + String(cpm));


     display.setTextSize(1.5);
     display.setCursor(0,22);
     display.setTextColor(WHITE);
     display.println("HUM: " + String(rel_hum) + " % ");

     display.setTextSize(1.5);
     display.setCursor(0,32);
     display.setTextColor(WHITE);
     display.println("UV: " + String(uvIntensity) + " mW/cm^2");

     display.setTextSize(1.5);
     display.setCursor(0,42);
     display.setTextColor(WHITE);
     display.println("TEMP: " + String(temp) + " °C" );
     display.display();
     delay(2000);
     display.clearDisplay();
     sprintf(jsonString,"{\"sample\": %d, \"cpm\": %d, \"sv0\": %f, \"uv\": %f, \"hum\": %f, \"temp\": %f, \"time\": %d}", sample, cpm, sv0, uvIntensity, rel_hum, temp, Time.local());
     Particle.publish("onData", jsonString);
 }
}

//Takes an average of readings on a given pin
//Returns the average
int averageAnalogRead(int pinToRead)
{
 byte numberOfReadings = 8;
 unsigned int runningValue = 0;

 for(int x = 0 ; x < numberOfReadings ; x++)
   runningValue += analogRead(pinToRead);
 runningValue /= numberOfReadings;

 return(runningValue);

}

//The Arduino Map function but for floats
//From: http://forum.arduino.cc/index.php?topic=3922.0
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {


 collectData();

}

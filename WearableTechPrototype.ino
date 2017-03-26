// I2C
#include <Wire.h>
// Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET 4

// Boold Pulse Oximeter
#include "MAX30105.h"
#include "spo2_algorithm.h"


// Air Pressure, Humidity, Temperature Sensor
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#define SEALEVELPRESSURE_HPA (1013.25)

//UV indexer
#include <Adafruit_SI1145.h>
// Light sensor/Luxmeter 

// Lux
#include <SparkFunTSL2561.h>

// claim devices
Adafruit_SSD1306 display(OLED_RESET);
Adafruit_BME280 bme280; // I2C


SFE_TSL2561 luxmeter;// SparkFun Elextronics
boolean gain = false;
unsigned int ms;
unsigned char time = 2;

Adafruit_SI1145 uv = Adafruit_SI1145();
float UVindex = 0;
//MAX30105 MAX30102
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 particleSensor;
const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred
float beatsPerMinute;
int beatAvg;

// variables
  float temperature;
  float pressure;
  float altitude;
  float humidity;
  double lux;

void setup() {
  Serial1.begin(9600);
  Serial.begin(9600);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);// initialize with the I2C Address
  bme280.begin(); // initialize Air pressure, Humidity, Temperature sensor
  luxmeter.begin();
  luxmeter.setTiming(gain,time,ms);
  luxmeter.setPowerUp();
  uv.begin();
  particleSensor.begin(Wire, I2C_SPEED_FAST);
  particleSensor.setup(); //Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A);
  
  display.display();
  delay(2000);
  display.clearDisplay();
}

void loop() {
  delay(ms);
  unsigned int data0;
  unsigned int data1;
  
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  temperature = bme280.readTemperature();
  pressure = bme280.readPressure() / 100.0f;
  altitude = bme280.readAltitude(SEALEVELPRESSURE_HPA);
  humidity = bme280.readHumidity();
  
  display.print("Temperature: "); display.print(temperature);
  display.println(" C");
  display.print("Pressure: ");display.print(pressure);
  display.println(" hPa");  
  display.print("Alt.: ");display.print(altitude);
  display.println(" m");  
  display.print("Humidity: ");display.print(humidity);
  display.println(" %");
  if (luxmeter.getData(data0,data1)){
    
    boolean good;
    good = luxmeter.getLux(gain,ms,data0,data1,lux);
    display.print("lux: ");display.println(lux);
    
  }

  UVindex = uv.readUV();
  UVindex /= 100;
  display.print("UV: "); display.println(UVindex);

  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }
  if(irValue < 50000)
  {
    display.print("BPM: ");display.println("No Finger?");
  }
  else
  {
    if(beatAvg == 0)
    {
      display.print("BPM: ");display.println("Detecting...");
    }
    else
    {
     display.print("BPM: ");display.println(beatAvg);
    }
  }
  
  display.display();
  delay(1);
  display.clearDisplay();
  String message = "{temperature: " + String(temperature) + "\n, pressure: " + String(pressure) + "\n, altitude: "+ String(altitude) + "\n, humidity: " + String(humidity)+ "\n, lux:" + String(lux) + "\n, UVindex: "+ String(UVindex) +"}\n\n";
  Serial1.write(message.c_str());
  
}

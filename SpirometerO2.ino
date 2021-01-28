/*
 * This is the software developed for the  3D printed spirometer. It is not to 
 * be used for any human health related measurements...mostly for dogs.
 * 
 * 
 */
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
#define BLYNK_PRINT Serial
#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads;    
double  calibrationv; //used to store calibrated value
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=106;//When sensor is healthy and new it reads 106 on high
double timeChecker = 0.0;

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
char auth[] = "QF0VrkZ0PFlyAZAX6mpQhr0ak2A3Gwdl";//this is where your token from Blynk


// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "Palm2704";//your wifi network
char pass[] = "9073456071  ";//your password

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h


const int analogInPin = 33;  //change this if you want to use another analog pin--works on this model of esp32
float area_1 = 0.000531; //these are the areas taken carefully from the 3D printed venturi 2M before constriction
float area_2 = 0.000201;// this is area within the venturi
float rho = 1.225; //Demsity of air in kg/m3;
float dt = 0;
int logCount = 0;
float timerBreath;
float secondsBreath;
float minuteTotal;
int newBreath = 0;
float TimerNow = 0.0;
int totalBreath = 0;
float massFlow = 0;
float volFlow = 0;
float maxFlow = 0.0;
float volumeTotal = 0;
float sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
float voltage = 0.0;
float Pa = 0.0;
float fev1 = 0.0;
float vol[30]; //this is the total volume that stored with time...
float volSec[30];//this is the the liter/sec messured
int snatch = 0;
int fundex = 0;
 double modr;//Variable to hold mod value in
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  minuteTotal = millis();
  //Blynk.begin(auth, ssid, pass);
  delay(2000);
  for(int p = 0;p < 30; p++){
    vol[p] = 0; //clears out matrix
    volSec[p] = 0;
  }
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
  calibrationv=calibrate();
  Serial.print("calibrate: ");
  Serial.println(calibrationv);
   if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange))
   {
    sensorcheck=1;
     //current_function=1;//Sensor needs to be calibrated
     need_calibrating();//print need calibrating message
   } 
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
 tft.drawCentreString("BLOW.....",125,55,4);
}
void need_calibrating(){
   Serial.println("sensor out of range");
  
}
int calibrate(){
  int16_t adc0=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
  
  result=adc0/20;
  return result;
  timeChecker = millis();
}
void loop() {
 //Blynk.run(); 
  if((millis()- minuteTotal) > 60000){
    minuteTotal = millis();
    //Serial.print( "Breaths per minute:");
    //Serial.println(totalBreath);
   // logfile.print("                                        ");
    //logfile.println(totalBreath);
    totalBreath = 0;
  }
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
 
//Serial.print("max sens value");
//Serial.println(sensorValue);
 
  voltage =( 3.3 * (( sensorValue )/4095));//the usual voltage conversion for 12 bit esp32
  
  Pa = (190 * voltage/3.3) - 31;  //equation from sensor data sheet
     //Serial.print("pa");
     //Serial.println(Pa);
  if(( Pa > 1) || (fundex >5 && fundex <28)) {   //checks to see if your breathing out and if your halfway through a blow
    if (newBreath < 1) {
      timerBreath = millis();
      fev1 = 0.0;
      snatch = 0;
      fundex = 0;
      if(volumeTotal > 100){
      totalBreath = totalBreath + 1; //counts breath
      //Serial.print("VolumeTotal= ");
      //Serial.println(volumeTotal);
      }
      
      volumeTotal = 0;
      newBreath = 1;
      
     
    }
  
    if(!(snatch%5)){
      vol[fundex] = volumeTotal;
      volSec[fundex] = volFlow;
      fundex++;
      
    }
  snatch++;
  // change the analog out value:
  //analogWrite(analogOutPin, outputValue);
  if((millis() - timerBreath >= 1000) && (fev1 == 0)) fev1 = volumeTotal;
massFlow = 1000*sqrt((abs(Pa)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2))))); //Bernoulli equation
volFlow = massFlow/rho; //volumetric flow of air
if(volFlow > maxFlow) maxFlow = volFlow;

volumeTotal = volFlow * (millis() - TimerNow) + volumeTotal;//integrates volumes over units of time to get total volume
  // print the results to the Serial Monitor:
  //Serial.print("sensor = ");
  //Serial.print(sensorValue);
  Serial.print("VolumeTotal= ");
 Serial.println(volumeTotal);
 
  //Serial.print("\t pressure = ");
  //Serial.println(Pa);

 
  } else if(newBreath){    //if your done figure out the results
    newBreath = 0;
    //Serial.print("time for breath");
    if(volumeTotal > 200){
    secondsBreath = (millis() - timerBreath)/1000;
    Serial.print("FEV1  ");
    Serial.println(fev1);
    Serial.print("FVC  ");
    Serial.print(volumeTotal);
    Serial.print("FV1/FVC");
    Serial.println(fev1/volumeTotal);
    Serial.print("Duration");
    Serial.println(timerBreath/1000);
    Serial.print("MaxFlow");
    Serial.println(maxFlow);
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("FEV1",32,20,4);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("FVC",32,80,4);
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.drawString("888888",70,5,7);
    tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
    tft.drawNumber(fev1,70,5,7);
    tft.setTextColor(TFT_BLACK, TFT_BLACK);
    tft.drawString("888888",70,70,7);
    tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
    tft.drawNumber(volumeTotal,70,70,7);
    //Serial.println(secondsBreath);
    delay(5000);
    tft.fillScreen(TFT_BLACK);
   printer();
   dataDump();
   O2dump();
  // if(Blynk.connected())blynkPrint();
   
 }
    for(int p = 0;p < 30; p++){
    vol[p] = 0;
    volSec[p] = 0;
  }
   tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
 tft.drawCentreString("BLOW.....",125,55,4);
    maxFlow = 0;
  
  }
  
  TimerNow = millis();
  delay(20);
}
void dataDump(){
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("FEV1:",32,5,4);
    tft.drawCentreString("FEVC:",32,27,4);
    tft.drawCentreString("DURATION:",67,49,4);
    tft.drawCentreString("FEV1/FEVC:",70,71,4);
    tft.drawCentreString("MAXFLOW:",63,93,4);
   
    tft.setTextColor(0xFBE0, TFT_BLACK); // Orange
    tft.drawNumber(fev1,100,5,4);
     tft.drawNumber(volumeTotal,100,27,4);
      tft.drawNumber(timerBreath/1000,160,49,4);
       tft.drawNumber((fev1/volumeTotal) * 100,160,71,4);
        tft.drawNumber(maxFlow,160,93,4);
     delay(3000);
   
    
}
void printer(){     //Draws nice little graph
   int x = 1;
  for(int p = 0; p< 30; p++){
    Serial.print("vol");
    Serial.println(vol[p]);
    Serial.print("volspeed");
    Serial.println(volSec[p]);
     int x = map(vol[p],0,5000,0,20);
   
    linearMeter(x,10,p*8,5,10,3,25,1);
    
  }
  delay(3000);
}
void blynkPrint(){             //connects to Blynk
   tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
 tft.drawCentreString("BLYNK......",125,55,4);
 Blynk.virtualWrite(V1,timerBreath/1000);
 Blynk.virtualWrite(V2, fev1/volumeTotal);
 Blynk.virtualWrite(V3, volumeTotal);
 Blynk.virtualWrite(V4, fev1);
  
  
  for(int i = 0; i < 20; i++){
   Blynk.virtualWrite(V5, vol[i]);
    Blynk.virtualWrite(V6, volSec[i]);
   delay(500);
  }
  
}
void O2dump(){
   double modr;//Variable to hold mod value in
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
      
      
      currentmv = adc0/20;
      calibratev=calibrationv;
      result=(currentmv/calibratev)*20.9;
  Serial.print("O2%:  ");
  Serial.println(result);
}

void linearMeter(int val, int x, int y, int w, int h, int g, int n, byte s)       //Does the graphing
{
  tft.setRotation(0);
  // Variable to save "value" text colour from scheme and set default
  int colour = TFT_BLUE;
  // Draw n colour blocks
  for (int b = 1; b <= n; b++) {
    if (val > 0 && b <= val) { // Fill in coloured blocks
      switch (s) {
        case 0: colour = TFT_RED; break; // Fixed colour
        case 1: colour = TFT_GREEN; break; // Fixed colour
        case 2: colour = TFT_BLUE; break; // Fixed colour
        //case 3: colour = rainbowColor(map(b, 0, n, 127,   0)); break; // Blue to red
        //case 4: colour = rainbowColor(map(b, 0, n,  63,   0)); break; // Green to red
        //case 5: colour = rainbowColor(map(b, 0, n,   0,  63)); break; // Red to green
        //case 6: colour = rainbowColor(map(b, 0, n,   0, 159)); break; // Rainbow (red to violet)
      }
      tft.fillRect(x + b*(w+g), y, w, h, colour);
    }
    else // Fill in blank segments
    {
      tft.fillRect(x + b*(w+g), y, w, h, TFT_DARKGREY);
    }
  }
}

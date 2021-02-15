
#include "DFRobot_OxygenSensor.h"
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
const int  buttonPin1 = 0; 
const int  buttonPin2 = 35; 
#define COLLECT_NUMBER    10             // collect number, the collection range is 1-100.
#define Oxygen_IICAddress ADDRESS_3

DFRobot_OxygenSensor Oxygen;
#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;
int wtTotal = 0;
int buttonPushCounter1 = 0;   // counter for the number of button presses
int buttonState1 = 1;         // current state of the button
int lastButtonState1 = 0;  
int buttonPushCounter2 = 0;   // counter for the number of button presses
int buttonState2 = 1;         // current state of the button
int lastButtonState2 = 0;  
   

// These constants won't change. They're used to give names to the pins used:
const int analogInPin = 33;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to
float area_1 = 0.000531;
float area_2 = 0.000201;
float rho = 1.225; //Demsity of air in kg/m3;
float dt = 0;
float timerBreath;
float secondsBreath;
float minuteTotal;
int newBreath = 0;
float TimerNow = 0.0;
int totalBreath = 0;
float massFlow = 0;
float volFlow = 0;
float volumeTotal = 0;
float volumeMinute = 0;
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
float voltage = 0.0;
float lastTemp = 0.0;
float lastCotwo = 0.0;
float lastOtwo = 0.0;
float vo2Max = 0.0;
float vo2MaxMax = 0.0;
float Pa = 0.0;
const int numReadings = 5;
float oReadings[numReadings];
float coReadings[numReadings];

int oreadIndex = 0;
int coreadIndex = 0;
float oTotal = 0;
float coTotal = 0;
float oAverage = 0;
float coAverage = 0;
void setup() {
  Serial.begin(115200);
  Wire.begin();
  while(!Oxygen.begin(Oxygen_IICAddress)) {
    Serial.println("I2c device number error !");
    delay(1000);
  }
  Serial.println("I2c connect success !");
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }
  
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  tft.fillScreen(TFT_RED);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.drawCentreString("Enter Weight in LBS",120,12,4);
  tft.setCursor(180, 115, 4);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.println("UP");
  
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    oReadings[thisReading] = 0;
  }
    for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    coReadings[thisReading] = 0;
  
}

while(buttonPushCounter2 < 3)wtRead();
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_GREEN, TFT_RED);   
  tft.drawCentreString("VO2  MAX",120,60,4);
  tft.setCursor(160, 115, 4);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.println("GO!");
  minuteTotal = millis();
}

void loop() {
  if(! digitalRead(buttonPin1)){
    totalBreath = 0;
    volumeMinute = 0;
  }
  if(! digitalRead(buttonPin2)){
    screenMax();
  }
  if(totalBreath == 0)minuteTotal = millis();              
  if((millis()- minuteTotal) > 30000){
    minuteTotal = millis();
    //Serial.print( "Breaths per minute:");   
    //Serial.println(totalBreath);
    //Serial.print("Volume Exhaled One Minute: ");
    //Serial.println(volumeMinute);
    totalBreath = 0;
    goFigure();   
    volumeMinute = 0;
  }
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:
 
  voltage =( 3.3 * (( sensorValue )/4095.0));//the usual voltage conversion for 12 bit esp32
  //Serial.print("sensor value");
  //Serial.println(voltage);
  Pa = (190.0 * voltage/3.3) - 31.0;  //equation from sensor data sheet
//Serial.print("Pa");
  //Serial.println(Pa);
  if( Pa > 5 ) {
    if (newBreath < 1) {
      timerBreath = millis();

 if(volumeTotal > 300){
      totalBreath = totalBreath + 1;
      //Serial.print("Breathvolume:");
      //Serial.print(volumeTotal);
      //Serial.print("     TotalBreaths: ");
      //Serial.println(totalBreath);
     
      }
      volumeMinute = volumeMinute + volumeTotal;
      volumeTotal = 0;
      
      newBreath = 1;
      
     
    }
massFlow = 1000*sqrt((abs(Pa)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2)))));
volFlow = massFlow/rho; //volumetric flow of air
volumeTotal = volFlow * (millis() - TimerNow) + volumeTotal;
  
  } else if(newBreath){
    newBreath = 0;
   
    secondsBreath = (millis() - timerBreath)/1000;
    if(secondsBreath > 0.1){
    //Serial.print("time for breath");
    //Serial.println(secondsBreath);
    O2dump();
    CO2dump();
    screen();
    }
    
  }
  TimerNow = millis();
  delay(20);
}
void CO2dump(){
  if (airSensor.dataAvailable())
  {
    //Serial.print("co2(ppm):");
    //Serial.print(airSensor.getCO2());
lastCotwo = airSensor.getCO2();
    //Serial.print(" temp(C):");
    //Serial.print(airSensor.getTemperature(), 1);
lastTemp = airSensor.getTemperature();
    //Serial.print(" humidity(%):");
    //Serial.print(airSensor.getHumidity(), 1);
    //Serial.println();
    // subtract the last reading:
  coTotal = coTotal - coReadings[coreadIndex];
  // read from the sensor:
  coReadings[coreadIndex] = lastCotwo;
  // add the reading to the total:
  coTotal = coTotal + coReadings[coreadIndex];
  // advance to the next position in the array:
  coreadIndex = coreadIndex + 1;

  // if we're at the end of the array...
  if (coreadIndex >= numReadings) {
    // ...wrap around to the beginning:
    coreadIndex = 0;
  }

  // calculate the average:
 coAverage = coTotal / numReadings;
 lastCotwo = coAverage;
  // send it to the computer as ASCII digits
  }
  else
    Serial.println("Waiting for new data");

   delay(500);
}
void O2dump(){
  float oxygenData = Oxygen.ReadOxygenData(COLLECT_NUMBER);
  //Serial.print(" Oxygen concentration is ");
  //Serial.print(oxygenData);
  //Serial.println(" %vol");
 // subtract the last reading:x                                                    
  oTotal = oTotal - oReadings[oreadIndex];
  // read from the sensor:
  oReadings[oreadIndex] = oxygenData;
  // add the reading to the total:
  oTotal = oTotal + oReadings[oreadIndex];
  // advance to the next position in the array:
  oreadIndex = oreadIndex + 1;

  // if we're at the end of the array...
  if (oreadIndex >= numReadings) {
    // ...wrap around to the beginning:
    oreadIndex = 0;
  }

  // calculate the average:
  oAverage = oTotal / numReadings;
  // send it to the computer as ASCII digits
  
  lastOtwo = oAverage;
}
void goFigure(){
  float percentN2exp;
  float co2;
  volumeMinute = volumeMinute * 2.0;
  volumeMinute = volumeMinute/1000.0; //gives liters of air VE
  //Serial.print("liters/min uncorrected");
  //Serial.print(volumeMinute);
  co2 = lastCotwo/10000.0;
  //lastOtwo = 17.5;
  //Serial.print("CO2  ");
  //Serial.print(co2);
  //Serial.print("02  ");
  //Serial.println(lastOtwo);
  percentN2exp = (100.0 - (co2 + lastOtwo));
  //Serial.print("%N  ");    
  //Serial.println(percentN2exp);
  volumeMinute = volumeMinute * (273/(273 + lastTemp)) * ((760.0 - 25.2)/760);
  //Serial.print("liters/min corrected  ");
  //Serial.print(volumeMinute);
  vo2Max = volumeMinute * (((percentN2exp/100.0) * 0.265) - (lastOtwo/100.0));
  //Serial.print(lastOtwo/100.0);
  //Serial.print("This: ");
  //Serial.print((percentN2exp/100.0 * 0.265) - (lastOtwo/100.0));
  //Serial.print("VO2Max!  ");
  //Serial.print(vo2Max);
  vo2Max = (vo2Max * 1000.0)/(float(wtTotal)/2.2);
  //Serial.print("VO2Max!!!ml/kg:  ");
  //Serial.print(vo2Max);
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.drawCentreString("VO2Max= ",60,10,4);
  tft.setTextColor(TFT_RED, TFT_RED);
  tft.drawString("888888",70,40,7);
  tft.setTextColor(TFT_WHITE, TFT_RED); // Orange
  //tft.drawNumber(vo2Max,100,40,7);
  tft.setCursor(70, 40, 7);
  
  if(vo2Max > vo2MaxMax) vo2MaxMax = vo2Max;
   tft.println(vo2MaxMax);
   tft.setCursor(160, 115, 4);
   tft.setTextColor(TFT_GREEN, TFT_RED);
   tft.println("RESET"); 
   
   
  
  
  
}
void wtRead(){
  buttonState1 = digitalRead(buttonPin1);
  if (buttonState1 != lastButtonState1) {
    // if the state has changed, increment the counter
    if (buttonState1 == LOW) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter1++;
      //Serial.println("on");
      //Serial.print("number of button pushes: ");
      //Serial.println(buttonPushCounter1);
      if(buttonPushCounter1 == 10)buttonPushCounter1 = 0;
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  lastButtonState1 = buttonState1;
  buttonState2 = digitalRead(buttonPin2);
  if (buttonState2 != lastButtonState2) {
    // if the state has changed, increment the counter
    if (buttonState2 == LOW) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter2++;
      wtTotal = wtTotal + 1000/pow(10, buttonPushCounter2) * buttonPushCounter1;
      buttonPushCounter1  = 0;
      
      if(buttonPushCounter2 == 3){
        //Serial.print("total wt = ");
        //Serial.print(wtTotal);
      }
    } else {
      // if the current state is LOW then the button went from on to off:
     // Serial.println("off");
    }
    // Delay a little bit to avoid bouncing
    delay(50);
  }
  lastButtonState2 = buttonState2;
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  if(buttonPushCounter2 < 3){
  int counter = 30 + (40 * buttonPushCounter2);
  tft.drawNumber(buttonPushCounter1,counter,40,7);
  }
}
void screen(){
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("TotalVol",50,10,4);
  tft.setTextColor(TFT_BLACK, TFT_BLACK);
  tft.drawString("888888",40,80,7);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Orange
  //tft.drawNumber(vo2Max,40,80,7);
  tft.setCursor(100, 40, 7);
  int puff = volumeMinute;
 
   tft.println(puff); 
   tft.setCursor(160, 115, 4);
   tft.setTextColor(TFT_GREEN, TFT_BLACK);
   tft.println("RESET");
   tft.setCursor(20, 115, 4);
   int  timeNow = (millis()- minuteTotal)/1000;
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
   tft.println(timeNow);
}
void screenMax(){
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.drawCentreString("VO2Max= ",60,10,4);
  tft.setTextColor(TFT_RED, TFT_RED);
  tft.drawString("888888",70,40,7);
  tft.setTextColor(TFT_WHITE, TFT_RED); // Orange
  //tft.drawNumber(vo2Max,100,40,7);
  tft.setCursor(70, 40, 7);
  
  if(vo2Max > vo2MaxMax) vo2MaxMax = vo2Max;
  
   tft.println(vo2MaxMax);
   tft.setCursor(160, 115, 4);
   tft.setTextColor(TFT_GREEN, TFT_RED);
   tft.println("RESET"); 
}

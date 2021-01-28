



#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads;    
double  calibrationv; //used to store calibrated value
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=106;//When sensor is healthy and new it reads 106 on high
double timeChecker = 0.0;
void setup(void) 
{
 Serial.begin(115200);
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
void loop(){
  
    double modr;//Variable to hold mod value in
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
    if(millis() - timeChecker >2000){
     timeChecker = millis();
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
}

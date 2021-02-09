






const int  buttonPin1 = 0; 
const int  buttonPin2 = 35; 
#include <TFT_eSPI.h> // Graphics and font library for ST7735 driver chip
#include <SPI.h>
int wtTotal = 0;
int buttonPushCounter1 = 0;   // counter for the number of button presses
int buttonState1 = 1;         // current state of the button
int lastButtonState1 = 0;  
int buttonPushCounter2 = 0;   // counter for the number of button presses
int buttonState2 = 1;         // current state of the button
int lastButtonState2 = 0;  
TFT_eSPI tft = TFT_eSPI();  // Invoke library, pins defined in User_Setup.h
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_RED);
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  //digitalWrite(buttonPin1, HIGH);
  //digitalWrite(buttonPin2, HIGH);
  tft.setTextColor(TFT_GREEN, TFT_RED);
  tft.drawCentreString("Enter Weight in LBS",120,12,4);
  
}

void loop() {
 // Serial.println(digitalRead(buttonPin1));
  
  buttonState1 = digitalRead(buttonPin1);
  if (buttonState1 != lastButtonState1) {
    // if the state has changed, increment the counter
    if (buttonState1 == LOW) {
      // if the current state is HIGH then the button went from off to on:
      buttonPushCounter1++;
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter1);
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
      Serial.println("on");
      Serial.print("number of button pushes: ");
      Serial.println(buttonPushCounter2);
      if(buttonPushCounter2 == 3){
        Serial.print("total wt = ");
        Serial.print(wtTotal);
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
  //tft.drawNumber(3,110,40,7);
  //tft.drawNumber(4,150,40,7);
  // put your main code here, to run repeatedly:
 /* 
  tft.fillScreen(TFT_RED);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("FEV1",32,20,4);
  tft.setTextColor(TFT_RED, TFT_RED);
  tft.drawString("888888",70,5,7);
  tft.setTextColor(TFT_WHITE, TFT_RED); // Orange
  tft.drawNumber(vo2Max,70,5,7);
  */
}

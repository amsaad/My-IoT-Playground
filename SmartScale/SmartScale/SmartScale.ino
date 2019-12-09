#include <TM1637Display.h>
#include <HX711_ADC.h>

#define CLK 2
#define DIO 4

int DOUTPIN = 10;
int SCKPIN = 11;
int DisplayPIN = 5;
int LEDPIN = 3;
int BUTTONPIN = 7;

float MinWeight = 16.2f;
float WeightThreshold = 1.0f;
float calValue = 2025;
long stabilisingtime = 2000;

float currentWeight = 0.0;
int rapidlyBlink = 2; //time to delay on led blink
int checkWeightInterv = 5000; //Check each X miliseconds
unsigned long lastDebounceTime = 0;
unsigned long debounceDelay = 50;
int lastButtonState = LOW;
int buttonState;
unsigned long lastTimeSensorCheck = 0;
const uint8_t SEG_REDY[] = {
  SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,           // H
  SEG_B | SEG_C                                  // I
};


TM1637Display display = TM1637Display(CLK, DIO);
HX711_ADC LoadCell(DOUTPIN, SCKPIN);

void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  digitalWrite(LEDPIN, HIGH);
  delay(100);
  digitalWrite(LEDPIN, LOW);
  Serial.println("Starting...");
  display.setBrightness(5);

  pinMode(DisplayPIN, OUTPUT);
  pinMode(BUTTONPIN, INPUT);


  digitalWrite(DisplayPIN, HIGH);
  display.setSegments(SEG_REDY);

  LoadCell.begin();
  LoadCell.start(stabilisingtime);

  if (LoadCell.getTareTimeoutFlag()) {
    Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
  }
  else {
    LoadCell.setCalFactor(calValue); // set calibration value (float)
    Serial.println("Startup + tare is complete");
  }

  for (int i = 1; i <= 10; i++) {
    digitalWrite(LEDPIN, (i % 2));
    delay(200);
  }
  getWeight();

  digitalWrite(DisplayPIN, LOW);
}

void loop() {
  int btnVAL = digitalRead(BUTTONPIN);
  if (btnVAL != lastButtonState) {
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (btnVAL != buttonState) {
      buttonState = btnVAL;
      if (buttonState == HIGH) {
        getWeight();
        PrintResult();
      }
      else
      {
        delay(100);
        checkWeight();
      }
    }
  }
  lastButtonState = btnVAL;

  if ((millis() - lastTimeSensorCheck) > checkWeightInterv) {
    checkWeight();
  }
}
void checkWeight()
{
  getWeight();
  if (currentWeight <= MinWeight + WeightThreshold) {
    BlinkLED();
    PrintResult();
  }
  else
  {
    digitalWrite(DisplayPIN, LOW);
  }
}
void getWeight()
{
  bool isBalanced = false;
  while (!isBalanced) {
    LoadCell.update();
    float weightNow = 101560;//LoadCell.getData();
    weightNow = weightNow / 1000;
    if (weightNow > 0)
    {
      float differ = currentWeight - weightNow;
      if (differ < 0)
        differ = differ * -1;

      currentWeight = weightNow;

      if (differ < 0.5)
      {
        isBalanced = true;
      }
      else {
        delay(100);
      }
    }
  }
}

void PrintResult()
{
  Serial.print("Current Weight is");
  Serial.println(currentWeight);
  digitalWrite(DisplayPIN, HIGH);
  display.showNumberDecEx(currentWeight, 64, true);
}

void BlinkLED()
{
  digitalWrite(LEDPIN, HIGH);
  delay(100 * rapidlyBlink);
  digitalWrite(LEDPIN, LOW);
  delay(100 * rapidlyBlink);
  digitalWrite(LEDPIN, HIGH);
  delay(100 * rapidlyBlink);
  digitalWrite(LEDPIN, LOW);
  delay(100 * rapidlyBlink);
}

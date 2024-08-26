// include the library code:
#include <LiquidCrystal_PCF8574.h>
#include "printHelpers.h"

int reading = 0;
float vRef = 4.101;
float vScale = 2.1;
float voltage;
float exponent;
float pressure;
String input;

// sampling parameters
int nSamp = 16;   // Number of Samples to take for each reading - best if it's a power of two.
int interval = 7; // milliseconds interval between successive readings
unsigned long sum1 = 0;  // the total of nSamp readings from the ADC
int average; // calculated value of measured voltage in millivolts


LiquidCrystal_PCF8574 lcd(0x27);  // set the LCD address to 0x27 for a 16 chars and 2 line display


// 2 custom characters

byte dotOff[] = { 0b00000, 0b01110, 0b10001, 0b10001,
                  0b10001, 0b01110, 0b00000, 0b00000 };
byte dotOn[] = { 0b00000, 0b01110, 0b11111, 0b11111,
                 0b11111, 0b01110, 0b00000, 0b00000 };


// Configure Arduino reference voltage and LCD Display
void setup() {
  int error;
  analogReference (EXTERNAL);
  analogRead (A0);  // force voltage reference to be turned on

  Serial.begin(115200);
  Serial.println("LCD...");

  // wait on Serial to be available on Leonardo
  while (!Serial)
    ;

  Serial.println("Probing for PCF8574 on address 0x27...");

  // See http://playground.arduino.cc/Main/I2cScanner how to test for a I2C device.
  Wire.begin();
  Wire.beginTransmission(0x27);
  error = Wire.endTransmission();
  Serial.print("Error: ");
  Serial.print(error);

  if (error == 0) {
    Serial.println(": LCD found.");
    lcd.begin(16, 2);  // initialize the lcd

    lcd.createChar(1, dotOff);
    lcd.createChar(2, dotOn);

  } else {
    Serial.println(": LCD not found.");
  }  // if

  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("Pressure:");
}




void loop() {
  // if(Serial.available()){
  //       input = Serial.readStringUntil('\n');
  //       if(input == "QUERY"){
  //         Serial.println(voltage);
  //       }

  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setBacklight(255);
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  // lcd.print(millis() / 1000);

  sum1 = 0; //ready to start adding values

  //add up nSamp successive readings; 
  for (byte count = 0; count < nSamp; count++ )
  {
    reading = analogRead(A0);     // actual read
    sum1 = sum1 + reading;
    delay(interval);
  }

  average = averageReadings(sum1, nSamp);
  //Serial.println(average);
  
  voltage = average * vRef*vScale/1024.0;
  //Serial.println(voltage);
  //lcd.print(voltage);
  //lcd.setCursor(6, 1);
  //lcd.print("V");

  exponent = 1.25 * voltage - 12.75; //This is the conversion formula for Pfeiffer IKR 270 (cold cathode gauge) in Torr
  //exponent = 1.667 * voltage - 11.33; //This is the conversion formula for Pfeiffer PKR 361 (full range gauge) in mbar
  pressure = pow(10, exponent);
  Serial.println(sci(pressure,2));
  lcd.print(sci(pressure,2));
  lcd.setCursor(9, 1);
  lcd.print("mbar");
  delay(100);
}

int averageReadings(int sum, int number)
{
  sum = sum + (number >>1);  // add a correction of 0.5 of the reading - see notes
  float average = sum / number; //divide by number of readings last to keep precision high
  return ((int) average); //return the result as an integer as decimal point values arent valid
  // (int) is a "cast" to change a float variable to an integer. Values after the decimal point are "cut off" so float 1.93 becomes int 1
}

#include <LiquidCrystal.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Adafruit_Sensor.h>

const int rs = 12;
const int en = 11;
const int d4 = 6;
const int d5 = 5;
const int d6 = 4;
const int d7 = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//Time Display Related
#define LCD_Contrast_PIN 7
#define LCD_Backlight_PIN 8

int h, m, s;
int set_hour = 19;
int set_minute = 30;
int set_second = 15;
int secs = 1000;
//

//DHT11 Related
#define DHTPIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
//

void setup()
{
  Serial.begin(9600);
  
  LCD_Time_Display();
  DHT11Method();
}

void LCD_Time_Display()
{
  pinMode(LCD_Backlight_PIN, OUTPUT);
  analogWrite(LCD_Contrast_PIN, 50);
  analogWrite(LCD_Backlight_PIN, 128);

  lcd.begin(16,2);
  lcd.setCursor(0,0);
  lcd.print("Time: ");
}

void DHT11Method()
{
  dht.begin();
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
}

void printChar()
{
  //display hours
  lcd.setCursor(6,0);
  lcd.print(h/10);
  lcd.setCursor(7,0);
  lcd.print(h%10);

  lcd.setCursor(8,0);
  lcd.print(":");

  //display minutes
  lcd.setCursor(9,0);
  lcd.print(m/10);
  lcd.setCursor(10,0);
  lcd.print(m%10);

  lcd.setCursor(11,0);
  lcd.print(":");

  //display seconds
  lcd.setCursor(12,0);
  lcd.print(s/10);
  lcd.setCursor(13,0);
  lcd.print(s%10);
}

void Clock()
{
  for(h = set_hour; h < 24; h++)//Hours
  {
    for(m = set_minute; m < 60; m++)//minutes
    {
      for(s = set_second; s < 60; s++)//seconds
      {
        printChar();
        delay(secs - (millis() % 1000));
      }

      set_second = 00;        
    }

    set_minute = 00;
  }

  set_hour = 00;  
}

void Temperature()
{
  float fahrenheit = dht.readTemperature(true);
  float celsius = (fahrenheit - 32) * 5/9;

  lcd.setCursor(6,1);
  lcd.print(celsius);
  lcd.setCursor(8,1);
  lcd.print("C");

  delay(1000);
}

void loop() 
{
  Clock();
  Temperature();
}

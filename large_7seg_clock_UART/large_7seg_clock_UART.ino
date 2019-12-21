#include "RTClib.h"
#include "Adafruit_Si7021.h"
#include <FastLED.h>
#include <EEPROM.h>

#define BAUD_RATE 115200
#define yes 2
#define no 1
#define NUM_LEDS 30
#define DATA_PIN 13

#define degree 11
#define unit_f 12

#define r_time_loc 0
#define g_time_loc 1
#define b_time_loc 2

#define r_temp_loc 3
#define g_temp_loc 4
#define b_temp_loc 5

RTC_PCF8523 rtc;
Adafruit_Si7021 sensor = Adafruit_Si7021();
CRGB leds[NUM_LEDS];

int LED_ON = 0x00ff00; // Note that Red and Green are swapped
int LED_OFF = 0x000000;

int digit_set[13][7] = {
    {yes, yes, yes, no, yes, yes, yes},
    {yes, no, no, no, yes, no, no},
    {yes, yes, no, yes, no, yes, yes},
    {yes, yes, no, yes, yes, yes, no},
    {yes, no, yes, yes, yes, no, no},
    {no, yes, yes, yes, yes, yes, no},
    {no, yes, yes, yes, yes, yes, yes},
    {yes, yes, no, no, yes, no, no},
    {yes, yes, yes, yes, yes, yes, yes},
    {yes, yes, yes, yes, yes, no, no},
    {no, no, no, no, no, no, no},
    {yes, yes, yes, yes, no, no, no},
    {no, yes, yes, yes, no, no, yes}};

unsigned long time_1;
unsigned long time_2;
unsigned long time_3;
unsigned long time_4;

int dst_count = 0;
int dst_offset = 0;
bool dst_set = false;

int _month_ = 0;
int _day_ = 0;
int _year_ = 0;
int _hour_ = 0;
int _min_ = 0;
int _second_ = 0;
int _WeekDay_ = 0;

int hour_ones = 0;
int hour_tens = 0;
int min_ones = 0;
int min_tens = 0;
int sec_ones = 0;
int sec_tens = 0;

int temp_tens;
int temp_ones;
int humid_tens;
int humid_ones;

int ones = 0;
int tens = 0;
int hundreds = 0;

int humidity;
int temperature;

int red_time;
int green_time;
int blue_time;

int red_temp;
int green_temp;
int blue_temp;

int red_humid;
int green_humid;
int blue_humid;

char incomingByte = ' ';
char buffer[64] = " ";
String read_line = "";

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
char *buffer_1[255];
char *time_str = "time";
char *time_color_str = "color_time";
char *temp_color_str = "color_temp";
char *humid_color_str = "humid_temp";
char *date_str = "date";
char *save_str = "save";

void setup()
{
  Serial.begin(115200);
  if (!rtc.begin())
  {
    Serial.println("Could not find RTC");
  }

  if (!sensor.begin())
  {
    Serial.println("Did not find Si7021 sensor!");
    while (true)
      ;
  }

  //FastLED.addLeds<WS2812B, DATA_PIN, RGB>(leds, NUM_LEDS);
  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);
  //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  //rtc.adjust(DateTime(2021, 11, 7, 1, 59, 50));
  // put your setup code here, to run once:

  red_time = EEPROM.read(r_time_loc);
  green_time = EEPROM.read(g_time_loc);
  blue_time = EEPROM.read(b_time_loc);

  red_temp = EEPROM.read(r_temp_loc);
  green_temp = EEPROM.read(g_temp_loc);
  blue_temp = EEPROM.read(b_temp_loc);

  Serial.print(red_time);
  Serial.print(", ");
  Serial.print(green_time);
  Serial.print(", ");
  Serial.print(blue_time);
  Serial.print(", ");

  Serial.print(red_temp);
  Serial.print(", ");
  Serial.print(green_temp);
  Serial.print(", ");
  Serial.print(blue_temp);
  Serial.print("\n");
}

bool DST_check(int month, int dayOfWeek, int hour, int minute, int second)
{

  if (month == 3 && dayOfWeek == 0 && hour == 2 && minute == 00 && second == 00 && dst_count <= 2)
  {
    dst_count++;
    Serial.println("DST START");
  }
  else if (month == 11 && dayOfWeek == 0 && hour == 2 && minute == 00 && second == 00)
  {
    dst_count = 0;
    Serial.println("DST STOP");
  }

  if (dst_count == 2)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void get_temp_humid()
{

  humidity = int(sensor.readHumidity());
  temperature = int(sensor.readTemperature());
  temperature = int((temperature * 9 / 5) + 32.0);

  temp_tens = temperature / 10;
  temp_ones = temperature % 10;

  humid_tens = humidity / 10;
  humid_ones = humidity % 10;
}

void get_time()
{

  DateTime now = rtc.now();

  _month_ = now.month();
  _day_ = now.day();
  _year_ = now.year();
  _WeekDay_ = now.dayOfTheWeek();

  _hour_ = now.hour();
  _min_ = now.minute();
  _second_ = now.second();

  dst_set = DST_check(_month_, _WeekDay_, _hour_, _min_, _second_);

  if (dst_set == true)
  {
    dst_offset = 1;
  }
  else if (dst_set == false)
  {
    dst_offset = 0;
  }

  if (_hour_ == 0)
  {
    _hour_ = 12;
  }
  if (_hour_ > 12)
  {
    _hour_ = _hour_ - 12;
  }

  hour_tens = _hour_ / 10;
  hour_ones = _hour_ % 10;

  min_tens = _min_ / 10;
  min_ones = _min_ % 10;

  sec_tens = _second_ / 10;
  sec_ones = _second_ % 10;
}

void read_serial_port()
{
  int x = 0;
  bool done = false;
  int y = 0;
  int z = 0;

  while (Serial.available())
  {
    incomingByte = Serial.read();
    done = true;
    if (incomingByte == 13 || incomingByte == 10)
    {
      x = 0;
      done = true;
    }
    else
    {
      buffer[x] = incomingByte;
      x++;
    }
  }

  if (done == true)
  {
    char *parse = strtok(buffer, ",");
    char *command = parse;
    while (parse != NULL)
    {
      parse = strtok(NULL, ",");
      //Serial.println(parse);
      buffer_1[y] = parse;
      y++;
    }

    if (strcmp(command, time_str) == 0)
    {

      int y = atoi(buffer_1[0]);
      int mo = atoi(buffer_1[1]);
      int d = atoi(buffer_1[2]);
      int h = atoi(buffer_1[3]);
      int mi = atoi(buffer_1[4]);
      int s = atoi(buffer_1[5]);
      // Serial.print(y);
      // Serial.print(mo);
      // Serial.print(d);
      // Serial.print(h);
      // Serial.print(mi);
      // Serial.print(s);
      rtc.adjust(DateTime(y, mo, d, h, mi, s));
      Serial.println("Time Set");
    }

    if (strcmp(command, time_color_str) == 0)
    {
      int r = atoi(buffer_1[0]);
      int g = atoi(buffer_1[1]);
      int b = atoi(buffer_1[2]);

      red_time = r;
      green_time = g;
      blue_time = b;

      digit_show(hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);

      Serial.println("Color Set");
    }

    if (strcmp(command, temp_color_str) == 0)
    {
      int r = atoi(buffer_1[0]);
      int g = atoi(buffer_1[1]);
      int b = atoi(buffer_1[2]);

      red_temp = r;
      green_temp = g;
      blue_temp = b;

      digit_show(temp_tens, temp_ones, degree, unit_f, red_temp, green_temp, blue_temp);
      Serial.println("Color Set");
    }

    if (strcmp(command, humid_color_str) == 0)
    {
      int r = atoi(buffer_1[0]);
      int g = atoi(buffer_1[1]);
      int b = atoi(buffer_1[2]);

      red_humid = r;
      green_humid = g;
      blue_humid = b;

      Serial.println("Color Set");
    }

    if (strcmp(command, save_str) == 0)
    {
      EEPROM.write(r_time_loc, red_time);
      EEPROM.write(g_time_loc, green_time);
      EEPROM.write(b_time_loc, blue_time);

      EEPROM.write(r_temp_loc, red_temp);
      EEPROM.write(g_temp_loc, green_temp);
      EEPROM.write(b_temp_loc, blue_temp);

      Serial.println("Colors Saved");
    }

    y = 0;
    done = false;

    for (int a = 0; a < sizeof(buffer); a++)
    {
      buffer[a] = " ";
    }
  }
}

void digit_show(int h_t, int h_o, int m_t, int m_o, int r, int g, int b)
{

  //Serial.println(m_o);
  for (int a = 0; a < 7; a++)
  {
    if (digit_set[m_o][a] == yes)
    {
      leds[a] = CRGB(r, g, b);
    }

    else if (digit_set[m_o][a] == no)
    {
      leds[a] = CRGB(0, 0, 0);
    }
  }

  for (int a = 7; a < 14; a++)
  {
    if (digit_set[m_t][a - 7] == yes)
    {
      leds[a] = CRGB(r, g, b);
    }

    else if (digit_set[m_t][a - 7] == no)
    {
      leds[a] = CRGB(0, 0, 0);
    }
  }

  for (int a = 14; a < 21; a++)
  {
    if (digit_set[h_o][a - 14] == yes)
    {
      leds[a] = CRGB(r, g, b);
    }

    else if (digit_set[h_o][a - 14] == no)
    {
      leds[a] = CRGB(0, 0, 0);
    }
  }

  for (int a = 21; a < 28; a++)
  {
    if (h_t == 0)
    {
      leds[a] = CRGB(0, 0, 0);
    }
    else
    {
      if (digit_set[h_t][a - 21] == yes)
      {
        leds[a] = CRGB(r, g, b);
      }

      else if (digit_set[h_t][a - 21] == no)
      {
        leds[a] = CRGB(0, 0, 0);
      }
    }
  }

  FastLED.show();
}
void loop()
{

  if (millis() > time_1 + (1e3))
  {

    time_1 = millis();
    get_time();
    get_temp_humid();

    Serial.print(hour_tens);
    Serial.print(hour_ones);
    Serial.print(":");
    Serial.print(min_tens);
    Serial.print(min_ones);
    Serial.print(":");
    Serial.print(sec_tens);
    Serial.print(sec_ones);
    Serial.print(",");
    Serial.print(temp_tens);
    Serial.print(temp_ones);
    Serial.print(", ");
    Serial.print(humid_tens);
    Serial.print(humid_ones);
    Serial.print("\n");

    if (_second_ >= 0 && _second_ < 45)
    {
      digit_show(hour_tens, hour_ones, min_tens, min_ones, red_time, green_time, blue_time);
    }
    else if (_second_ >= 45 && _second_ <= 59)
    {
      digit_show(temp_tens, temp_ones, degree, unit_f, red_temp, green_temp, blue_temp);
    }
  }

  if (millis() > time_3 + 20)
  {
    time_3 = millis();
    read_serial_port();
  }

  if (millis() > time_4 + 1e3)
  {
    time_4 = millis();
    int sample = analogRead(A0);
    // Serial.println(sample);
    if (sample > 900)
    {
      FastLED.setBrightness(25);
    }
    else
    {
      FastLED.setBrightness(255);
    }
  }

  // put your main code here, to run repeatedly:
}

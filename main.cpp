// Time - main.cpp

/***********************************************************************************
  
  Objective
    Time utilities for ESP32, that includes TZ and DST adjustments.
    Get the POSIX style TZ format string from  https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv

  References
  - Wifi :
    . https://randomnerdtutorials.com/esp32-useful-wi-fi-functions-arduino/
  - Current time :
    . https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
    . https://randomnerdtutorials.com/esp32-ntp-timezones-daylight-saving/
    . https://sourceware.org/newlib/libc.html#Timefns
    . https://cplusplus.com/reference/ctime/ 
    . https://github.com/espressif/arduino-esp32/blob/master/cores/esp32/esp32-hal-time.c#L47
    . https://www.gnu.org/software/libc/manual/html_node/TZ-Variable.html

/***********************************************************************************
  Libraries and types
***********************************************************************************/

#include <WiFi.h>
#include <time.h>

/***********************************************************************************
  Constants
***********************************************************************************/

// Local network access point
const char *SSID = "---------";
const char *PWD = "---------"; 

// NTP server (=>UTC time) and Time zone
const char* NTP_SERVER = "pool.ntp.org";  // Server address (or "ntp.obspm.fr", "ntp.unice.fr", ...) 
const char* TIME_ZONE = "CET-1CEST,M3.5.0,M10.5.0/3"; // Europe/Paris time zone 

/***********************************************************************************
  Tool functions
***********************************************************************************/

void setTimeZone(const char *timeZone)
{
  // To work with Local time (custom and RTC)
  setenv("TZ", timeZone, 1); 
  tzset();
}

void initRTC(const char *timeZone)
{
  // Set RTC with Local time, using an NTP server
  configTime(0, 0, "pool.ntp.org"); // To get UTC time
  tm time;
  getLocalTime(&time);    
  setTimeZone(timeZone);  // Transform to Local time
}

bool getCustomTime(int year, int month, int day, int hour, int minute, int second, tm *timePtr)
{
  // Set a time (date) without DST indication
  *timePtr = {0};
  timePtr->tm_year = year - 1900; 
  timePtr->tm_mon = month-1;
  timePtr->tm_mday = day;
  timePtr->tm_hour = hour;
  timePtr->tm_min = minute;
  timePtr->tm_sec = second;
  time_t t = mktime(timePtr);
  timePtr->tm_hour--;
  time_t t1 = mktime(timePtr);
  memcpy(timePtr, localtime(&t), sizeof(tm)); 
  if (localtime(&t1)->tm_isdst==1)
  {
    if (timePtr->tm_isdst==0) return false;  // Ambigous
    else timePtr->tm_hour--;
  }
  return true;
}

void printTime(const char *str, tm *timePtr, bool ok)
{
  // Print time with DST indication (like ISO 8601)
  if (ok)
  {
    char buf[30];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S %z", timePtr);
    Serial.printf("%s : %s\n", str, buf);
  }
  else Serial.printf("%s : is ambigous\n", str);
}

/***********************************************************************************
  setup and loop functions
***********************************************************************************/

void setup()
{
  // Open serial port
  Serial.begin(115200);
  while (!Serial);

  // Set time zone (not necessary after InitRTC)
  setTimeZone(TIME_ZONE);

  // Custom time
  struct tm time;	
  bool okTime;
  Serial.println("\nCUSTOM TIME");
  Serial.println("Summer time ON");
  okTime = getCustomTime(2024, 3, 31, 1, 59, 59, &time); printTime("31/03/2024 01h59m59s", &time, okTime);
  okTime = getCustomTime(2024, 3, 31, 2, 0, 0, &time); printTime("31/03/2024 02h00m00s", &time, okTime);
  okTime = getCustomTime(2024, 3, 31, 2, 59, 59, &time); printTime("31/03/2024 02h59m59s", &time, okTime);
  okTime = getCustomTime(2024, 3, 31, 3, 0, 0, &time); printTime("31/03/2024 03h00m00s", &time, okTime); 
  okTime = getCustomTime(2024, 3, 31, 3, 59, 59, &time); printTime("31/03/2024 03h59m59s", &time, okTime); 
  okTime = getCustomTime(2024, 3, 31, 4, 0, 0, &time); printTime("31/03/2024 04h00m00s", &time, okTime); 
  Serial.println("Summer time OFF");
  okTime = getCustomTime(2024, 10, 27, 1, 59, 59, &time); printTime("27/10/2024 01h59m59s", &time, okTime); 
  okTime = getCustomTime(2024, 10, 27, 2, 0, 0, &time); printTime("27/10/2024 02h00m00s", &time, okTime);    
  okTime = getCustomTime(2024, 10, 27, 2, 59, 59, &time); printTime("27/10/2024 02h59m59s", &time, okTime); 
  okTime = getCustomTime(2024, 10, 27, 3, 0, 0, &time); printTime("27/10/2024 03h00m00s", &time, okTime); 
  Serial.println("Automatic correction");
  okTime = getCustomTime(2023, 2, 29, 0, 0, 0, &time); printTime("29/02/2023 00h00m00s", &time, okTime); 
  Serial.println("\nLOCAL TIME");

  // Connect to the Wifi access point 
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED);
  Serial.printf("IP=%s RSSI=%d\n", WiFi.localIP().toString(), WiFi.RSSI());

  // Init RTC with Local time using an NTP server
  initRTC(TIME_ZONE);
  WiFi.disconnect(true);

  // Local time from RTC (without Wifi)
	getLocalTime(&time); printTime("now", &time, true);
  delay(1000);
  getLocalTime(&time); printTime("now+1s", &time, true);
}

void loop()
{
}

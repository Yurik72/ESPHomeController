#ifndef utilities_h
#define utilities_h

//color converters
#define REDVALUE(x) ((x >> 16) & 0xFF)
#define GREENVALUE(x)  ((x >> 8) & 0xFF)
#define BLUEVALUE(x) ((x >> 0) & 0xFF)


bool writeConfigFS(bool saveConfig);
bool readConfigFS();
String getFormattedTime(time_t tt);
unsigned long GetHours(time_t tt);
unsigned long GetMinutes(time_t tt);
time_t apply_hours_minutes_fromhhmm(time_t src, int hhmm, long offs);
time_t apply_hours_minutes(time_t src, int h, int m, long offs);
time_t	  mklocaltime(struct tm *_timeptr, long offs);
#endif
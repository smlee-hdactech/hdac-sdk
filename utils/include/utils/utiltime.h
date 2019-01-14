#ifndef UTILTIME_H
#define UTILTIME_H

#include <cstdint>
#include <string>

int64_t GetTime();
int64_t GetTimeMillis();
int64_t GetTimeMicros();

std::string DateTimeStrFormat(const char* pszFormat, int64_t nTime);

#endif // UTILTIME_H

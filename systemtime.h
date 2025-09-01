//
// Copyright 2018 Hans-Juergen Lange <hjl@simulated-universe.de>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the “Software”), to deal in the
// Software without restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
// and to permit persons to whom the Software is furnished to do so, subject to the
// following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
// INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
// CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef NETPLATTFORM_SYSTEMTIME_H
#define NETPLATTFORM_SYSTEMTIME_H

#include <string>
// **************************************************************************
//                             F o r w a r d s
// **************************************************************************
// **************************************************************************
//                  C l a s s    d e c l a r a t i o n
// **************************************************************************
class SystemTime {
public:
    SystemTime();
    int GetYear();
    int GetMonth();
    int GetDayOfMonth();
    int GetWeekDay();
    int GetHour();
    int GetMinute();
    int GetSecond();
    int GetMilliSecond();
    std::string Format(const char* aFormat);
    std::string Get1123Date();
    void Set(time_t aTime);
    bool operator==(SystemTime& b);
    bool operator!=(SystemTime& b);
    bool operator>(SystemTime& B);
    bool operator<(SystemTime& B);
private:
    time_t Time;
    int tm_year;
    int tm_mon;
    int tm_wday;
    int tm_mday;
    int tm_hour;
    int tm_min;
    int tm_sec;
    int tm_msec;
    static const char* WeekDay[];
    static const char* Month[];
};


#endif //NETPLATTFORM_SYSTEMTIME_H

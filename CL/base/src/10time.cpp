#include "../1cfg.h"

#if CL_Sys == CL_Sys_Win
#include <Windows.h>
#else 
#endif

#include "../10time.h"

namespace cl
{
    //计算某一年有多少天
    cl_si uv32 accumulate_year_day(uv32 year)
    {
        uv32 sum = 0;
        for (uv32 i = 1; i < year; i++)
        {
            if (is_leap_year(i))
                sum += 366;
            else
                sum += 365;
        }
        return sum;
    }

    //计算某一年从1月1号到指定月的天数
    cl_si uv32 accumulate_month_day(uv32 year, uv32 month)
    {
        uv32 two = 28;
        if (is_leap_year(year)) two += 1;

        switch (month - 1)
        {
        case 1:  return 31;
        case 2:  return 31 + two;
        case 3:  return 31 + two + 31;
        case 4:  return 31 + two + 31 + 30;
        case 5:  return 31 + two + 31 + 30 + 31;
        case 6:  return 31 + two + 31 + 30 + 31 + 30;
        case 7:  return 31 + two + 31 + 30 + 31 + 30 + 31;
        case 8:  return 31 + two + 31 + 30 + 31 + 30 + 31 + 31;
        case 9:  return 31 + two + 31 + 30 + 31 + 30 + 31 + 31 + 30;
        case 10: return 31 + two + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31;
        case 11: return 31 + two + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30; //永远都不会累计到12月, 否则直接用上面的函数就好了
        }
        return  0;
    }


    uv32 week_day(uv32 year, uv32 month, uv32 day)
    {
        auto y = accumulate_year_day(year);
        auto m = accumulate_month_day(year, month);
        uv32 sum = y + m + day;
        return (sum % 7);//从公元第一天开始计算,周一就是公元第一天
    }

    namespace lib
    {
        // us
        uv64 _tick()
        {
#if CL_Sys == CL_Sys_Win
            LARGE_INTEGER val;
            if (QueryPerformanceCounter(&val))
                return val.QuadPart;
            return 0;
#else
            return 0;
#endif
        }
    }

    uv64 Tick::ms() { return lib::_tick() / 1000; }
    uv64 Tick::us() { return lib::_tick(); }

    Time Time::now()
    {
        Time ccf; 

#if CL_Sys == CL_Sys_Win
        SYSTEMTIME tmp;
        GetLocalTime(&tmp);
         
        ccf.mon = (uv8)tmp.wMonth;
        ccf.day = (uv8)tmp.wDay;
        ccf.hour = (uv8)tmp.wHour;
        ccf.min = (uv8)tmp.wMinute;
        ccf.sec = (uv8)tmp.wSecond; 
        ccf.week_day = (uv8)tmp.wDayOfWeek;
        ccf.year = tmp.wYear;
        ccf.ms = tmp.wMilliseconds;
#endif
        return ccf;
    }
}
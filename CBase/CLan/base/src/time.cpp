#include "../12time.h"

namespace cl
{
    //计算某一年有多少天
    int accumulate_year_day(int year)
    {
        int sum = 0;
        for (int i = 1; i < year; i++)
        {
            if (is_leap_year(i))
                sum += 366;
            else
                sum += 365;
        }
        return sum;
    }

    //计算某一年从1月1号到指定月的天数
    int accumulate_month_day(int year, int month)
    {
        int two = 28;
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
    
    
    int week_day(int year, int month, int day)
    {
        auto y = accumulate_year_day(year);
        auto m = accumulate_month_day(year, month);
        int sum = y + m + day;
        return (sum % 7);//从公元第一天开始计算,周一就是公元第一天
    }
}
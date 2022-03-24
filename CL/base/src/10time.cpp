#include "../10time.h"

namespace cl
{
    //����ĳһ���ж�����
    uv32 accumulate_year_day(uv32 year)
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

    //����ĳһ���1��1�ŵ�ָ���µ�����
    uv32 accumulate_month_day(uv32 year, uv32 month)
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
        case 11: return 31 + two + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30; //��Զ�������ۼƵ�12��, ����ֱ��������ĺ����ͺ���
        }
        return  0;
    }


    uv32 week_day(uv32 year, uv32 month, uv32 day)
    {
        auto y = accumulate_year_day(year);
        auto m = accumulate_month_day(year, month);
        uv32 sum = y + m + day;
        return (sum % 7);//�ӹ�Ԫ��һ�쿪ʼ����,��һ���ǹ�Ԫ��һ��
    }
}
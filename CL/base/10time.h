#ifndef __cl_base_time__
#define __cl_base_time__

#include "2type.h"

namespace cl
{
	//是否是闰年
	inline bool is_leap_year(uv32 year)
	{
		//公转一圈(回归年)365日5时48分46秒,平年365日,比回归年短0.242199日
		//四年共短0.9688日，故每四年增加一日
		//但每四年增加一日又比四个回归年多0.0312日, 400年后将多3.12日, 
		//故在400年中少设3个闰年, 即100, 200, 300年不是闰年
		if (year % 400 == 0) return true;
		if (year % 100 == 0) return false;
		if (year % 4 == 0) return true;
		return false;
	}

	//返回指定年指定月份的天数
	inline uv32 month_day(uv32 year, uv32 month)
	{
		uv32 days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
		if (month == 2)
		{
			if (is_leap_year(year)) return 29;
			return 28;
		}
		else
			return days[month - 1];
	}

	//返回某年某月某日是星期几
	uv32 week_day(uv32 year, uv32 month, uv32 day);
}

#endif//__cl_base_time__ 
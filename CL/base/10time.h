#ifndef __cl_base_time__
#define __cl_base_time__

#include "2type.h"

namespace cl
{
	//�Ƿ�������
	inline bool is_leap_year(uv32 year)
	{
		//��תһȦ(�ع���)365��5ʱ48��46��,ƽ��365��,�Ȼع����0.242199��
		//���깲��0.9688�գ���ÿ��������һ��
		//��ÿ��������һ���ֱ��ĸ��ع����0.0312��, 400��󽫶�3.12��, 
		//����400��������3������, ��100, 200, 300�겻������
		if (year % 400 == 0) return true;
		if (year % 100 == 0) return false;
		if (year % 4 == 0) return true;
		return false;
	}

	//����ָ����ָ���·ݵ�����
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

	//����ĳ��ĳ��ĳ�������ڼ�
	uv32 week_day(uv32 year, uv32 month, uv32 day);
}

#endif//__cl_base_time__ 
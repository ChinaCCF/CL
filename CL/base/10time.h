#ifndef __cl_base_time__
#define __cl_base_time__

#include "2type.h"

namespace cl
{
	//�Ƿ�������
	cl_si bool is_leap_year(uv32 year)
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
	cl_si uv32 month_day(uv32 year, uv32 month)
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

	//�������ķ����ǳ������ܵ�
	//ͨ��һ���̻߳�ȡʱ��, �������ֵ, �����߳�ͨ��ԭ�ӽ�����ȡ
	class Tick
	{
	public:
		static uv64 ms();
		static uv64 us();
	};

	class Time
	{
	public: 
		uv8 sec;//��
		uv8 min;//��
		uv8 hour;//ʱ
		uv8 day;//������һ��
		uv8 mon;//��
		uv8 week_day;//���ڼ�
		uv16 year;//�� 
		uv16 ms;

		static Time now();
	};
}

#endif//__cl_base_time__ 
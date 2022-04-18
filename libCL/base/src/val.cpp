#include "../4val.h"
#include <Windows.h>

namespace cl
{
	void Random::init(uv32 init_v)
	{
		srand(init_v);
		  }

	uv32 Random::ma_x(uv32 m)
	{
		uv32 val = 0;
		uv32 org = m;
		while(m)
		{
			//��������д��ԭ����,rand����ֻ�ܷ���[0~0x7FFF]֮���ֵ
			//�������ó˳���,���ֲ�����
			val = RAND_MAX * val + ::rand();
			m /= RAND_MAX;
		}
		return val % org;
	}
	uv32 Random::between(uv32 mi_n, uv32 m) { return ma_x( m - mi_n) + mi_n; }
}
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
			//这里这样写的原因是,rand函数只能返回[0~0x7FFF]之间的值
			//所以利用乘除法,来分部计算
			val = RAND_MAX * val + ::rand();
			m /= RAND_MAX;
		}
		return val % org;
	}
	uv32 Random::between(uv32 mi_n, uv32 m) { return ma_x( m - mi_n) + mi_n; }
}
#ifndef __cl_set_heap__
#define __cl_set_heap__

#include "../base/3type_traits.h" 
#include "../base/4val.h"
#include "../base/5mem.h"
#include "../base/7call.h"

#include "1base.h"

namespace cl
{
	enum class HeapType
	{
		Max, //大堆
		Min  //小堆
	};

	//使用数组来实现大小堆
	template<typename T, class MA = MemAllocator, HeapType HT = HeapType::Max> requires MemAllocType<MA>
	class Heap : public NoCopyObj
	{
		//算法上, 索引从1开始计数, 实现上从0开始

		using ThisType = Heap<T, MA, HT>; 
		using MBT = MemBuf<T, MA>;
		MBT buf_;
		uv32 cnt_ = 0;
		    
		T& _data(uv32 i) { return buf_.data()[i - 1]; }
		uv32 top_i(uv32 i) { return i / 2; }
		uv32 left_i(uv32 i) { return i << 1; } //i * 2
		uv32 right_i(uv32 i) { return 1 + (i << 1); } //i * 2

		template<HeapType> struct _Compare
		{
			bool operator()(T* p, uv32 i, uv32 j) { return lib::Compare<T, T>()(p[i - 1], p[j - 1]) < 0; }
		};
		template<> struct _Compare<HeapType::Max>
		{
			bool operator()(T* p, uv32 i, uv32 j) { return lib::Compare<T, T>()(p[i - 1], p[j - 1]) > 0; }
		};

		void _move(T* dst, T* arr, uv32 cnt)
		{
			for (uv32 i = 0; i < cnt; i++)
				new(dst + i)T(std::move(arr[i]));
		}
		void _swap(uv32 i, uv32 j)
		{ 
			T t = std::move(_data(i));
			_data(i) = std::move(_data(j));
			_data(j) = std::move(t);
		}

		void _float_node(uv32 i)
		{
			if (i <= 1) return;

			auto top = top_i(i);
			if (_Compare<HT>()(buf_.data(), top, i)) return;
			_swap(top, i);
			_float_node(top);
		}

		//下沉节点, 当拿走最顶点的节点后, 把最后的节点放到顶点, 这时候需要下沉
		void _sink_node(uv32 i)
		{
			auto left = left_i(i);
			auto right = right_i(i);

			if (left > cnt_) return;

			if (right > cnt_)
			{
				if (_Compare<HT>()(buf_.data(), i, left)) return;
				_swap(i, left);
			}
			else
			{
				uv32 j = _Compare<HT>()(buf_.data(), left, right) ? left : right;
				if (_Compare<HT>()(buf_.data(), i, j)) return;
				_swap(i, j);
				_sink_node(j);
			}
		}
	public:
		~Heap() 
		{
		}
		uv32 size() const { return buf_.size(); }
		uv32 count() const { return cnt_; } 
		T& top() const { return buf_.data()[0]; }

		void reserve(uv32 cnt) 
		{ 
			cnt += cnt_;
			if (cnt <= size()) return;

			MBT tmp;
			tmp.alloc(cnt);

			_move(tmp.data(), buf_.data(), cnt_);

			buf_ = std::move(tmp);
		}

		template<typename ... Args>
		void push(Args&& ... args)
		{
			reserve(1);
			new(buf_.data() + cnt_)T(std::forward<Args>(args)...);
			++cnt_;
			_float_node(cnt_);
		}
		T pop()
		{
			CL_Assert(cnt_ > 0);

			T ret = std::move(top());
			top() = std::move(_data(cnt_));
			_data(cnt_).~T();
			cnt_--;
			_sink_node(1);
			return ret;
		}
	}; 
}

#endif//__cl_set_heap__
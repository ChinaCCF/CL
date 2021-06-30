#ifndef __clan_set_heap__
#define __clan_set_heap__

#include "../base/4concept.h"
#include "../base/5exception.h"
#include "../base/6val.h"
#include "../base/11call.h"

#include "1base.h"

namespace cl
{
	enum class HeapType
	{
		Max, //大堆
		Min  //小堆
	};

	//使用数组来实现大小堆
	template<typename T, AllocMemType A, HeapType type = HeapType::Max>
	class _HeapArr : public NoCopyObj
	{
		using ThisType = _HeapArr<T, A, type>;

		T* arr_ = nullptr;
		s32 size_ = 0;
		s32 cnt_ = 0;

		//数组的索引是从1开始的, 第0个元素不使用
		//返回父节点的索引
		s32 top_i(s32 i) { return i / 2; }
		s32 left_i(s32 i) { return i << 1; } //i * 2
		s32 right_i(s32 i) { return 1 + (i << 1); } //i * 2

		template<HeapType _type = HeapType::Min>
		struct _CMP
		{
			bool operator()(T* arr, s32 i, s32 j) { return CmpVal<T>()(arr[i], arr[j]) < 0; }
		};
		template<>
		struct _CMP<HeapType::Max>
		{
			bool operator()(T* arr, s32 i, s32 j) { return CmpVal<T>()(arr[i], arr[j]) > 0; }
		};

		void _swap(s32 i, s32 j)
		{
			T t = std::move(arr_[i]);
			arr_[i] = std::move(arr_[j]);
			arr_[j] = std::move(t);
		}

		//上升节点
		void _float_node(s32 i)
		{
			if (i <= 1) return;

			auto top = top_i(i);
			if (_CMP<type>()(arr_, top, i)) return;
			_swap(top, i);
			_float_node(top);
		}
		//下沉节点
		void _sink_node(s32 i)
		{
			auto left = left_i(i);
			auto right = right_i(i);

			if (left > cnt_) return;
			if (right > cnt_)
			{
				if (_CMP<type>()(arr_, i, left)) return;
				_swap(i, left);
			}
			else
			{
				s32 j = _CMP<type>()(arr_, left, right) ? left : right;
				if (_CMP<type>()(arr_, i, j)) return;
				_swap(i, j);
				_sink_node(j);
			}
		}
	public:
		~_HeapArr()
		{
			if (arr_)
			{
				for (s32 i = 1; i <= cnt_; i++)
					arr_[i].~T();
				A().free(arr_);
			}
		}
		s32 capactiy() { return size_ - 1; }
		s32 cnt() { return cnt_; }
		void need(s32 cnt)
		{
			auto dif = size_ - cnt_ - 1; //第0个元素不使用
			if (dif > cnt) return;
			size_ = align_power2((cnt_ + cnt + 1) << 1);

			auto p = (T*)A().alloc(sizeof(T) * size_);

			if (arr_)
			{
				for (s32 i = 1; i <= cnt_; i++)
				{
					new(p + i)T(std::move(arr_[i]));
					arr_[i].~T();
				}

				A().free(arr_);
			}

			arr_ = p;
		}
		T& top() { return arr_[1]; }

		template<typename ... Args>
		void push(Args&& ... args)
		{
			need(1);
			++cnt_;
			new(arr_ + cnt_)T(std::forward<Args>(args)...); 
			_float_node(cnt_);
		}
		T pop()
		{
			cl_assert(cnt_ > 0);
			T ret = std::move(arr_[1]);
			arr_[1] = std::move(arr_[cnt_]);
			arr_[cnt_].~T();
			cnt_--;
			_sink_node(1);
			return ret;
		}
	}; 
}

#endif//__clan_set_heap__
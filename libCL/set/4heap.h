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
		Max, //���
		Min  //С��
	};

	//ʹ��������ʵ�ִ�С��
	template<typename T, class MA = MemAllocator, HeapType HT = HeapType::Max> requires MemAllocType<MA>
	class HeapArr : public NoCopyObj
	{
		//�㷨��, ������1��ʼ����, ʵ���ϴ�0��ʼ

		using ThisType = HeapArr<T, MA, HT>; 
		using MBT = MemBuf<T, MA>;
		MBT buf_;
		uv32 cnt_ = 0;
		    
		T& _data(uv32 i) { return buf_.data()[i - 1]; }

		uv32 _top_index(uv32 i) { return i / 2; }
		uv32 _left_index(uv32 i) { return i << 1; } //i * 2
		uv32 _right_index(uv32 i) { return 1 + (i << 1); } //i * 2

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

			auto top = _top_index(i);
			if (_Compare<HT>()(buf_.data(), top, i)) return;
			_swap(top, i);
			_float_node(top);
		}

		//�³��ڵ�, ���������Ľڵ��, �����Ľڵ�ŵ�����, ��ʱ����Ҫ�³�
		void _sink_node(uv32 i)
		{
			auto left = _left_index(i);
			auto right = _right_index(i);

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
		~HeapArr()
		{}
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
#ifndef __cl_set_arr__
#define __cl_set_arr__

#include "../base/3type_traits.h" 
#include "../base/4val.h"
#include "../base/7call.h"

#include "1base.h"

namespace cl
{
	namespace lib
	{
		template<typename T>
		class ArrayIt
		{ 
		public:
			uv32 index_ = 0;
			T* buf_ = nullptr;

			ArrayIt(T* arr, uv32 index) : buf_(arr), index_(index) {}

			T& operator*() const { return buf_[index_]; }
			T* operator->() const { return buf_ + index_; }

			ArrayIt& operator++() { index_++; return *this; }
			bool operator==(const ArrayIt& it) const { return index_ == it.index_; }
			bool operator!=(const ArrayIt& it) const { return index_ != it.index_; }
		};
	}

	//堆栈
	template<typename T, uv32 N>
	class Stack : NoCopyObj
	{
		using ThisType = Stack<T, N>;
		uv32 cnt_ = 0;
		T buf_[N];
	public:
		Stack() { clear(); }

		uv32 count() const noexcept { return cnt_; }
		T& operator[](uv32 index) { return buf_[index]; }
		T operator[](uv32 index) const { return buf_[index]; }

		void clear()
		{
			for (uv32 i = 0; i < cnt_; i++)
				(buf_ + i)->~T();
			cnt_ = 0;
		}

		template<typename ... Args>
		void push(Args&& ... args)
		{
			new(buf_ + cnt_)T(std::forward<Args>(args)...);
			cnt_++;
		}

		T pop()
		{
			cnt_--;
			T tmp = std::move(buf_[cnt_]);
			(buf_ + cnt_)->~T();
			return tmp;
		}
		T& back() { return buf_[cnt_ - 1]; }

		lib::ArrayIt<T> begin() const { return lib::ArrayIt<T>((T*)buf_, 0); }
		lib::ArrayIt<T> end() const { return lib::ArrayIt<T>((T*)buf_, cnt_); }
	};

	template<typename T, class MA = MemAllocator> requires MemAllocType<MA>
	class Array
	{
		using ThisType = Array<T, MA>;
		using MBT = MemBuf<T, MA>;
		MBT buf_;
		uv32 cnt_ = 0;
		  
		void _copy(T* dst, const T* arr, uv32 cnt)
		{
			for (uv32 i = 0; i < cnt; i++)
				new(dst + i)T(arr[i]);
		}
		void _move(T* dst, T* arr, uv32 cnt)
		{
			for (uv32 i = 0; i < cnt; i++)
				new(dst + i)T(std::move(arr[i]));
		} 
		 
		ThisType& _append(const T* p, uv32 cnt)
		{
			reserve(cnt);
			_copy(data() + cnt_, p, cnt);
			cnt_ += cnt;
			return *this;
		}
	public:
		Array() { }

		uv32 count() const { return cnt_; }
		uv32 size() const { return buf_.size(); }
		T* data() const { return buf_.data(); }

		T& operator[](uv32 index) const { return data()[index]; }

		Array(const ThisType& arr) { _append(arr.data(), arr.count()); }

		Array(ThisType&& arr) noexcept { cnt_ = arr.cnt_; buf_ = std::move(arr.buf_); }

		~Array() { clear(); }

		void clear()
		{
			for (uv32 i = 0; i < cnt_; i++)
				data()[i].~T();
			cnt_ = 0;
		}

		void reserve(uv32 cnt)
		{
			cnt += cnt_;
			if (cnt <= size()) return;

			MBT tmp;
			tmp.alloc(cnt);

			_move(tmp.data(), data(), cnt_);

			buf_ = std::move(tmp);
		}

		Array& operator=(const ThisType& arr)
		{ 
			clear();
			return _append(arr.data(), arr.count()); 
		}

		Array& operator=(ThisType&& arr) noexcept
		{
			clear();
			cnt_ = arr.cnt_;
			buf_ = std::move(arr.buf_);
			return *this;
		}

		Array& operator<<(const ThisType& arr)
		{ 
			return _append(arr.data(), arr.count());  
		}

		Array& operator<<(ThisType&& arr)
		{
			reserve(arr.count());
			_move(data() + cnt_, arr.data(), arr.count());
			cnt_ += arr.count();
			return *this;
		}

		template<typename ... Args>
		void push(Args&& ... args)
		{
			reserve(1);
			new(data() + cnt_)T(std::forward<Args>(args)...);
			cnt_++;
		}

		T pop()
		{
			cnt_--;
			T tmp = std::move(data()[cnt_]);
			(data() + cnt_)->~T();
			return tmp;
		}

		template<typename ... Args>
		void insert(uv32 index, Args&& ... args)
		{
			reserve(1);

			for (uv32 i = cnt_; i > index; i--)
				new(data() + i)T(std::move(data()[i - 1]));

			data()[index].~T();
			new(data() + index)T(std::forward<Args>(args)...);
			cnt_++;
		}

		void remove(uv32 index)
		{
			(data() + index)->~T();
			_move(data() + index, data() + index + 1, cnt_ - index - 1); 
			data()[--cnt_].~T(); 
		}
		void remove(const lib::ArrayIt<T>& it) { remove(it.index_); }
		template<typename T2>
		void remove(const T2& val)
		{
			auto it = find(val);
			CL_Assert(it.index_ < cnt_);
			remove(it);
		}

		lib::ArrayIt<T> begin() const { return lib::ArrayIt<T>(data(), 0); }
		lib::ArrayIt<T> end() const { return lib::ArrayIt<T>(data(), cnt_); }

		//这个方法需要优化
		//todo
		template<typename T2>
		lib::ArrayIt<T> find(const T2& val) const
		{
			uv32 i = 0;
			for (; i < cnt_; i++)
			{
				if (lib::Compare<T, T2>()(data()[i], val) == 0)
					break;
			}
			return lib::ArrayIt<T>(data(), i);
		}

		void remove_if(const Call<bool(T*)>& is_remove, bool keep_sort = false)
		{
			auto p = data();

			if (keep_sort)
			{ 
				for (uv32 i = 0; i < cnt_; i++)
				{
					if (is_remove(p + i))
						remove(i);
				}
			}
			else
			{ 
				auto end = data() + cnt_ - 1;
				while (true)
				{
					while (p <= end && is_remove(end))
					{
						end->~T();
						--cnt_;
						--end;
					}
					while (p <= end && !is_remove(p)) ++p;
					if (p >= end) break;

					p->~T();
					new(p)T(std::move(*end));
					--cnt_;
				}
			} 
		}

		void mov_up(uv32 index)
		{
			if (index == 0 || index >= cnt_) return;
			swap(data(), index,  index - 1);
		}
		void mov_down(uv32 index)
		{
			if (index >= cnt_ - 1) return;
			swap(data(), index, index + 1);
		}
	private:
		void _quick_sort(T* arr, uv32 size, const Call<sv32(T*, T*)>& cmp)
		{
			if (size <= 1) return;
			uv32 head = 0;
			uv32 tail = size - 1;
			T key = std::move(arr[head]);

			while (head < tail)
			{
				while (head < tail && cmp(arr + tail, &key) >= 0) --tail;
				arr[head] = std::move(arr[tail]);
				while (head < tail && cmp(arr + head, &key) <= 0) ++head;
				arr[tail] = std::move(arr[head]);
			}
			arr[head] = std::move(key);
			_quick_sort(arr, head, cmp);
			_quick_sort(arr + head + 1, size - head - 1, cmp);
		}
	public:
		void sort(const Call<sv32(T*, T*)>& cmp) { _quick_sort(data(), cnt_, cmp); }
	};
}

#endif//__cl_set_arr__
#ifndef __clan_set_arr__
#define __clan_set_arr__

#include "../base/4concept.h"
#include "../base/5exception.h"
#include "../base/6val.h"
#include "../base/11call.h"

#include "1base.h"

namespace clan
{
	template<typename T, AllocMemType A>
	class Array
	{
		T* arr_ = nullptr; 
		s32 cnt_ = 0;//当前存储的对象数
		s32 size_ = 0;//可以存储的对象的数目

		void _copy(const Array& arr)
		{
			clan_assert(size_ - cnt_ >= arr.cnt_);
			 
			for (s32 i = 0; i < arr.cnt_; i++) 
				new(arr_ + cnt_ + i)T(arr.arr_[i]);
			cnt_ += arr.cnt_;
		}

		void _move(Array* arr)
		{
			cnt_ = arr->cnt_; arr->cnt_ = 0;
			size_ = arr->size_; arr->size_ = 0;
			arr_ = arr->arr_; arr->arr_ = nullptr;
		}
		 
	public:
		Array() {}

		Array(const Array& arr)
		{
			size_ = arr.size_;
			arr_ = (T*)A().alloc(sizeof(T) * size_); 
			_copy(arr);
		}

		Array(Array&& arr) noexcept { _move(&arr); }
		 
		~Array()
		{
			clear();
			if (arr_) A().free(arr_);  
		}
		/*########################################################################################*/
		s32 size() const noexcept { return cnt_; } 
		T& operator[](s32 index) const { return arr_[index]; }

		void clear()
		{
			for (s32 i = 0; i < cnt_; i++)
				(arr_ + i)->~T();
			cnt_ = 0;
		}

		void need(s32 cnt)
		{
			if (cnt_ + cnt >= size_)
			{
				auto tc = align(cnt_ + cnt + 1, 16);
				size_ = align_power2(tc << 1); 
				{//不要使用realloc来重新申请内存, 有可能对象T内部存储了关联指针
					auto tmp = (T*)A().alloc(sizeof(T) * size_);
					for (s32 i = 0; i < cnt_; i++)
					{
						new(tmp + i)T(std::move(arr_[i]));
						arr_[i].~T();
					} 
					if (arr_) A().free(arr_);
					arr_ = tmp;
				} 
			}
		}

		Array& operator=(const Array& arr)
		{ 
			if (size_ < arr.size_)
			{
				this->~Array();
				new(this)Array(arr);
			}
			else
			{
				clear();
				_copy(arr);
			} 
			return *this;
		}

		Array& operator=(Array&& arr) noexcept
		{
			this->~Array();
			new(this)Array(std::move(arr)); 
			return *this;
		}
		 
		Array& operator<<(const Array& arr)
		{
			need(arr.cnt_);
			_copy(arr);
			return *this;
		}

		Array& operator<<(Array&& arr)
		{
			need(arr.cnt_);
			auto dst = arr_ + cnt_;
			auto src = arr.arr_;
			for (s32 i = 0; i < arr.cnt_; i++)
			{
				new(dst + i)T(std::move(src[i]));
				src[i].~T();
			} 
			cnt_ += arr.cnt_;
			arr.cnt_ = 0; //arr无需clear
			return *this;
		}
		 
		/*########################################################################################*/ 
		template<typename ... Args>
		void push_back(Args&& ... val)
		{
			need(1); 
			new(arr_ + cnt_)T(std::forward<Args>(val)...);
			cnt_++;
		}

		T pop_back()
		{
			cnt_--;
			T tmp = std::move(arr_[cnt_]);
			(arr_ + cnt_)->~T();
			return tmp;
		}

		template<typename ... Args>
		void insert(s32 index, Args&& ... val)
		{
			 need(1); 

			for (s32 i = cnt_; i > index; i--)
				new(arr_ + i)T( std::move(arr_[i - 1]));
			 
			arr_[index].~T();
			new(arr_ + index)T(std::forward<Args>(val)...);
			cnt_++;
		}

		class It
		{
			s32 index_ = 0;
			T* arr_ = nullptr;
			friend class Array<T, A>;
		public:
			It(T* arr, s32 index) : arr_(arr), index_(index) {}

			T& operator*() { return arr_[index_]; }
			T* operator->() { return arr_ + index_; }

			It& operator++() { index_++; return *this; }
			//不支持后递增
			//It operator++(int) { It it = *this; operator++(); return it; }
			bool operator==(const It& it) { return index_ == it.index_; }
			bool operator!=(const It& it) { return index_ != it.index_; }
		};

		It begin() const { return It(arr_, 0); } 
		It end() const { return It(arr_, cnt_); }

		template<typename R> //使用类型R而不是T, 允许存储的对象的判断范围更广
		It find(const R& val) const
		{ 
			s32 i = 0;
			for (; i < cnt_; i++)
			{
				if (CmpVal<T>()(arr_[i], val) == 0)
					break; 
			}
			return It(arr_, i); 
		}

		void remove(s32 index)
		{ 
			(arr_ + index)->~T();
			for (s32 i = index; i < cnt_ - 1; i++)
				new(arr_ + i)T(std::move(arr_[i + 1]));
			arr_[cnt_ - 1].~T();
			cnt_--;
		}
		void remove(const It& it) { remove(it.index_); } 
		template<typename R>
		void remove(const R& val)
		{
			auto it = find(val);
			cl_assert(it.index_ < cnt_);
			remove(it);
		}

		void remove_if(const clan::Call<bool(T*)>& is_remove)
		{
			u8 _buf[128];
			bool need_free = false;
			u8* tag;
			if (cnt_ > 128)
			{
				tag = (u8*)A().alloc(cnt_ + 1);
				need_free = true;
			}
			else 
				tag = _buf; 
			
			for (s32 i = 0; i < cnt_; i++)
			{
				if (is_remove(arr_ + i))
					tag[i] = 0;
				else
					tag[i] = 1;
			}
			s32 j = 0;
			for (s32 i = 0; i < cnt_; i++)
			{
				if (tag[i] == 1)
				{
					if (i == j) ++j;
					else
					{
						(arr_ + j)->~T();
						new(arr_ + j)T(arr_[i]);
						++j;
					}
				}
			}
			for(s32 i = j; i < cnt_; i++ )
				(arr_ + i)->~T();
			cnt_ = j;

			if(need_free) A().free(tag);
		}

		void mov_up(s32 index)
		{
			if (index == 0 || index >= cnt_) return; 
			swap(arr_[index], arr_[index - 1]);
		}
		void mov_down(s32 index)
		{
			if (index >= cnt_ - 1) return; 
			swap(arr_[index], arr_[index + 1]);
		}
	private:
		void _quick_sort(T* arr, s32 size, const clan::Call<s32(T*, T*)>& cmp)
		{
			if (size <= 1) return;
			s32 head = 0;
			s32 tail = size - 1;
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
		void sort(const clan::Call<s32(T*, T*)>& cmp) { _quick_sort(arr_, cnt_, cmp); }
	};
}

#endif//__clan_set_arr__
#ifndef __clan_set_map__
#define __clan_set_map__
 
#include "../base/4concept.h"
#include "../base/5exception.h"
#include "../base/6val.h"
#include "../base/11call.h"

#include "1base.h"

namespace clan
{
	namespace detail
	{
		template<typename Key, typename Val>
		struct HashMapNode : public Pair<Key, Val>
		{
			HashMapNode* next_ = nullptr;
			HashMapNode() { }
		}; 
	}

	//不允许字符串作为字典的关键词
	template<NotCharsType Key, typename Val, AllocMemType A>
	class HashMap
	{ 
		using Node = detail::HashMapNode<Key, Val>;

		Node** arr_ = nullptr;
		s32 size_ = 0;
		s32 cnt_ = 0;

		template<typename T>
		inline Node* _find(const T& key, s32 index) const
		{
			auto node = arr_[index];
			while (node && CmpVal<Key>()(node->first, key) != 0) node = node->next_;
			return node;
		}

		inline Val& _add_to_list(Node* node, s32 index)
		{
			cnt_++;
			node->next_ = arr_[index];
			arr_[index] = node;
			return node->second;
		}
		void _add(const Key& key, const Val& val)
		{
			auto node = cl_new(Node);
			node->first = key;
			s32 index = hash(node->first) % size_;
			_add_to_list(node, index) = val;
		}
		inline void _move(HashMap* map)
		{
			size_ = map->size_; map->size_ = 0;
			cnt_ = map->cnt_; map->cnt_ = 0;
			arr_ = map->arr_; map->arr_ = nullptr;
		}
	public:
		HashMap() : HashMap(32) {}

		explicit HashMap(s32 size)
		{
			size_ = size;
			arr_ = cl_alloc(Node*, size_);
			for (s32 i = 0; i < size_; ++i)
				arr_[i] = nullptr;
		}

		HashMap(HashMap&& map) noexcept { _move(&map); }

		HashMap(const HashMap& map) : HashMap(map.size_)
		{
			for (auto& p : map)
				_add(p.first, p.second);
		}

		HashMap& operator=(HashMap&& map) noexcept
		{
			this->~HashMap();
			_move(&map);
			return *this;
		}

		HashMap& operator=(const HashMap& map)
		{
			clear();
			this->operator<<(map);
			return *this;
		}

		~HashMap() { clear(); if (arr_) { cl_free(arr_); arr_ = nullptr; } }

		void clear()
		{
			for (s32 i = 0; i < size_; ++i)
			{
				while (true)
				{
					auto p = arr_[i];
					if (!p) break;
					arr_[i] = p->next_;
					cl_del(p);
				};
				arr_[i] = nullptr;
			}
			cnt_ = 0;
		}

		s32 size() const { return cnt_; }

		class It
		{
			friend class HashMap<Key, Val, KC>;
			s32 size_ = 0;
			s32 index_ = 0;
			Node** arr_ = nullptr;
			Node* node_ = nullptr;
		public:
			Pair<Key, Val>* operator->() { return node_; }
			Pair<Key, Val>& operator*() { return *node_; }

			It& operator++()
			{
				node_ = node_->next_;
				do
				{
					if (node_) break;
					index_++;
					if (index_ >= size_) break;
					node_ = arr_[index_];
				} while (true);
				return *this;
			}
			It operator++(int) { It it = *this; operator++(); return it; }
			bool operator ==(const It& it) { return node_ == it.node_; }
			bool operator !=(const It& it) { return node_ != it.node_; }
		};

		It begin() const
		{
			It it;
			if (cnt_ > 0)
			{
				it.arr_ = arr_;
				it.size_ = size_;

				for (s32 i = 0; i < size_; ++i)
				{
					if (arr_[i])
					{
						it.node_ = arr_[i];
						it.index_ = i;
						break;
					}
				}
			}
			else
				it.node_ = nullptr;
			return it;
		}

		It end() const
		{
			It it;
			it.node_ = nullptr;
			return it;
		}

		template<typename T>
		It find(const T& key) const
		{
			s32 index = hash(key) % size_;
			Node* node = _find(key, index);

			It it;
			it.node_ = node;
			it.arr_ = arr_;
			it.size_ = size_;
			it.index_ = index;
			return it;
		}

		template<typename T>
		Val& operator[](const T& key)
		{
			s32 index = hash(key) % size_;
			auto node = _find(key, index);
			if (node) return node->second;
			node = cl_new(Node);
			node->first = key;
			return _add_to_list(node, index);
		}

		template<typename T>
		Val& create(const It& it, const T& key)
		{
			auto node = cl_new(Node);
			node->first = key;
			_add_to_list(node, it.index_);
			return node->second;
		}

		void remove(const It& it)
		{
			s32 index = it.index_;
			auto node = it.node_;

			auto p = arr_[index];

			if (p == node)
				arr_[index] = node->next_;
			else
			{
				while (p->next_ != node) p = p->next_;
				p->next_ = node->next_;
			}

			cl_del(node);
			cnt_--;
		}

		template<typename T>
		void remove(const T& key)
		{
			auto it = find(key);
			cl_assert(it.node_ != nullptr);
			remove(it);
		}

		void remove_if(const cl::Call<bool(Key&, Val&)>& call)
		{
			//第一次检测数组中每个链表的头
			for (s32 i = 0; i < size_; i++)
			{
				auto node = arr_[i];
				while (node)
				{
					if (call(node->first, node->second))
					{
						cnt_--;
						arr_[i] = node->next_;
						cl_del(node);
						node = arr_[i];
					}
					else
						break;
				}
			}
			//第二次遍历数组中每个链表
			for (s32 i = 0; i < size_; i++)
			{
				auto p = arr_[i];
				if (p == nullptr) continue;

				auto node = p->next_;
				while (node)
				{
					if (call(node->first, node->second))
					{
						cnt_--;
						p->next_ = node->next_;
						cl_del(node);
						node = p->next_;
					}
					else
					{
						p = node;
						node = node->next_;
					}
				}
			}
		}

		HashMap& operator<<(const HashMap& map)
		{
			for (auto& p : map)
				_add(p.first, p.second);
			return *this;
		}

		HashMap& operator<<(HashMap&& map)
		{
			for (s32 i = 0; i < map.size_; i++)
			{
				auto node = map.arr_[i];
				while (node)
				{
					auto tmp = node->next_;
					{
						s32 index = hash(node->first) % size_;
						node->next_ = arr_[index];
						arr_[index] = node;
					}
					node = tmp;
				}
				map.arr_[i] = nullptr;
			}
			cnt_ += map.cnt_;
			map.cnt_ = 0;
			return *this;
		}
	};
}

#endif//__clan_set_map__
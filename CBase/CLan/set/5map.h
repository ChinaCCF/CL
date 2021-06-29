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
			HashMapNode() {}
		};
	}

	//不允许字符串作为字典的关键词
	template<NotCharsType Key, typename Val, AllocMemType A>
	class HashMap
	{
		friend class It;
		using Node = detail::HashMapNode<Key, Val>;

		Node** arr_ = nullptr;
		s32 size_ = 0;
		s32 cnt_ = 0;

		template<typename R>
		inline Node* _find(const R& key, s32 index) const
		{
			auto node = arr_[index];
			while (node && CmpVal<Key>()(node->first, key) != 0) node = node->next_;
			return node;
		}
		inline void _add_to_list(Node* node, s32 index)
		{
			cnt_++;
			node->next_ = arr_[index];
			arr_[index] = node;
		}
		inline void _move(HashMap* map)
		{
			size_ = map->size_; map->size_ = 0;
			cnt_ = map->cnt_; map->cnt_ = 0;
			arr_ = map->arr_; map->arr_ = nullptr;
		}

		template<typename R>
		inline Node* new_node(const R& key)
		{
			auto node = (Node*)A().alloc(sizeof(Node));
			new(node)Node();
			node->first = key;
			return node;
		}
		inline void del_node(Node* node)
		{
			node->~Node();
			A().free(node);
		}

		void _add(const Key& key, const Val& val)
		{
			auto node = new_node(key);
			node->second = val;
			s32 index = hash(node->first) % size_;
			_add_to_list(node, index);
		}
	public:
		HashMap() : HashMap(32) {}
		explicit HashMap(s32 size)
		{
			size_ = size;
			arr_ = (Node**)A().alloc(sizeof(Node*) * size_);
			for (s32 i = 0; i < size_; ++i)
				arr_[i] = nullptr;
		}
		HashMap(HashMap&& map) noexcept { _move(&map); }
		HashMap(const HashMap& map) : HashMap(map.size_) { this->operator<<(map); }

		~HashMap() { clear(); if (arr_) A().free(arr_); }

		s32 size() const { return cnt_; }

		void clear()
		{
			for (s32 i = 0; i < size_; ++i)
			{
				auto p = arr_[i];
				while (p)
				{
					auto t = p;
					p = p->next_;
					del_node(t);
				};
				arr_[i] = nullptr;
			}
			cnt_ = 0;
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
					Node* p = node;
					node = node->next_;

					s32 index = hash(p->first) % size_;
					p->next_ = arr_[index];
					arr_[index] = p;
				}
				map.arr_[i] = nullptr;
			}
			cnt_ += map.cnt_;
			map.cnt_ = 0;
			return *this;
		}

		template<typename R>
		Val& operator[](const R& key)
		{
			s32 index = hash(key) % size_;
			auto node = _find(key, index);
			if (!node)
			{
				node = new_node(key);
				_add_to_list(node, index);
			}
			return node->second;
		}

		class It
		{
			friend class HashMap<Key, Val, A>;
			HashMap<Key, Val, A>* map_;
			s32 size_ = 0;
			s32 index_ = 0;
			Node** arr_ = nullptr;
			Node* node_ = nullptr;
		public:
			It() {}
			It(Node** arr, Node* node, s32 size, s32 index) : arr_(arr), node_(node), size_(size), index_(index) {}

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
			bool operator ==(const It& it) { return node_ == it.node_; }
			bool operator !=(const It& it) { return node_ != it.node_; }
		};

		It begin() const
		{
			s32 i = 0;
			Node* node = nullptr;
			if (cnt_ > 0)
			{
				for (; i < size_; ++i)
				{
					if (arr_[i])
					{
						node = arr_[i];
						break;
					}
				}
			}
			return It(arr_, node, size_, i);
		}

		It end() const { return It(); }

		template<typename R>
		It find(const R& key) const
		{
			s32 index = hash(key) % size_;
			Node* node = _find(key, index);

			return It(arr_, node, size_, index);
		}

		void remove(const It& it)
		{
			s32 index = it.index_;
			auto node = it.node_;
			if (node == nullptr) return;

			auto p = arr_[index];

			if (p == node)
				arr_[index] = node->next_;
			else
			{
				while (p->next_ != node) p = p->next_;
				clan_assert(p != nullptr);
				p->next_ = node->next_;
			}

			del_node(node);
			cnt_--;
		}

		template<typename T>
		void remove(const T& key)
		{
			auto it = find(key);
			clan_assert(it.node_ != nullptr);
			remove(it);
		}

		void remove_if(const Call<bool(Pair<Key, Val>*)>& call)
		{
			for (s32 i = 0; i < size_; i++)
			{
				auto list = arr_[i];

				Node* p = list;
				list = nullptr;

				while (p)
				{
					auto t = p;
					p = p->next_;

					if (call(t))
					{
						--cnt_;
						del_node(t);
					}
					else
					{
						t->next_ = list;
						list = t;
					}
				}
				arr_[i] = list;
			}
		}
	};
}

#endif//__clan_set_map__
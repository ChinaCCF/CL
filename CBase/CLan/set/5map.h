#ifndef __clan_set_map__
#define __clan_set_map__
 
#include "../base/6val.h"  
#include "3list.h"

namespace cl
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

	//serial 顾名思义 就是序列化, 本对象是用于读取例如xml或者保存xml时候用到的
	template<NotCharsType Key, typename Val, AllocMemType A>
	class _SerialHashMap : public NoCopyObj
	{
	protected:
		using ThisType = _SerialHashMap<Key, Val, A>;
		using Node = detail::HashMapNode<Key, Val>; 
		
		template<typename F, typename S>
		struct _ListNode
		{
			F& first;
			S& second; 
			_ListNode(Key& key, Val& val) : first(key), second(val) {}
		};
		using ListNode = _ListNode<Key, Val>;

		Node** arr_ = nullptr;
		s32 size_ = 0;
		s32 cnt_ = 0;
		_List<ListNode, A> list_;

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
			list_.push_back(ListNode(node->first, node->second));
		}
		inline void _move(ThisType* map)
		{
			size_ = map->size_; map->size_ = 0;
			cnt_ = map->cnt_; map->cnt_ = 0;
			arr_ = map->arr_; map->arr_ = nullptr;
			list_ = std::move(map->list_);
		} 
	public:
		_SerialHashMap() : _SerialHashMap(32) {}
		explicit _SerialHashMap(s32 size)
		{
			size_ = size;
			arr_ = (Node**)A().alloc(sizeof(Node*) * size_);
			for (s32 i = 0; i < size_; ++i)
				arr_[i] = nullptr;
		}
		_SerialHashMap(ThisType&& map) noexcept { _move(&map); }

		~_SerialHashMap() 
		{
			clear(); 
			if (arr_) A().free(arr_); 
		}

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
					_del<A>(t); 
				};
				arr_[i] = nullptr;
			}
			cnt_ = 0;
		}

		_SerialHashMap& operator=(ThisType&& map) noexcept
		{
			this->~_SerialHashMap();
			_move(&map);
			return *this;
		}

		template<typename R>
		bool contain(const R& key)
		{
			s32 index = hash(key) % size_;
			auto node = _find(key, index); 
			return node != nullptr;
		}

		template<typename R>
		auto get(const R& key)
		{
			s32 index = hash(key) % size_;
			auto node = _find(key, index);
			if (!node)
			{
				node = _new<A, Node>();
				node->first = key;
				_add_to_list(node, index);
			}
			return node;
		}

		template<typename R>
		Val& operator[](const R& key)
		{
			return get(key)->second;
		}
		  
		class It
		{
			friend class ThisType;
			using Org = typename _List<ListNode, A>::It;
			Org it_;
		public:
			It(Org it) : it_(it) {}
			ListNode& operator*() { return *it_; }
			ListNode* operator->() { return it_; }

			It& operator++() { ++it_; return *this; }
			bool operator==(const It& it) { return it_ == it.it_; }
			bool operator!=(const It& it) { return it_ != it.it_; }
		};


		auto begin() const { return It(list_.begin()); }
		auto end() const { return It(list_.end()); }
		 
		void push_order(Key& key, Val& val) { list_.push_back(ListNode(key, val)); }
	};


	//不允许字符串作为字典的关键词
	template<NotCharsType Key, typename Val, AllocMemType A>
	class _HashMap
	{
	protected:
		friend class It;
		using ThisType = _HashMap<Key, Val, A>;
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
		inline void _move(ThisType* map)
		{
			size_ = map->size_; map->size_ = 0;
			cnt_ = map->cnt_; map->cnt_ = 0;
			arr_ = map->arr_; map->arr_ = nullptr;
		}

	public:
		_HashMap() : _HashMap(32) {}
		explicit _HashMap(s32 size)
		{
			size_ = size;
			arr_ = (Node**)A().alloc(sizeof(Node*) * size_);
			for (s32 i = 0; i < size_; ++i)
				arr_[i] = nullptr;
		}
		_HashMap(ThisType&& map) noexcept { _move(&map); }
		_HashMap(const ThisType& map) : _HashMap(map.size_) { this->operator<<(map); }

		~_HashMap() { clear(); if (arr_) A().free(arr_); }

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
					_del<A>(t); 
				};
				arr_[i] = nullptr;
			}
			cnt_ = 0;
		}

		_HashMap& operator=(ThisType&& map) noexcept
		{
			this->~_HashMap();
			_move(&map);
			return *this;
		}

		_HashMap& operator=(const ThisType& map)
		{
			clear();
			this->operator<<(map);
			return *this;
		}

		_HashMap& operator<<(const ThisType& map)
		{
			for (auto& p : map)
			{
				auto node = _new<A, Node>();
				node->first = p.first;
				node->second = p.second;
				s32 index = hash(node->first) % size_;
				_add_to_list(node, index);
			} 
			return *this;
		}

		_HashMap& operator<<(ThisType&& map)
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
				node = _new<A, Node>();
				node->first = key;
				_add_to_list(node, index);
			}
			return node->second;
		}

		class It
		{
			using MapType = _HashMap<Key, Val, A>;
			friend class MapType;
			s32 index_ = 0; 
			const MapType* map_ = nullptr;
			Node* node_ = nullptr;
		public:
			It() {}
			It(const MapType* map, Node* node,s32 index) : map_(map), node_(node), index_(index) {}

			Pair<Key, Val>* operator->() { return node_; }
			Pair<Key, Val>& operator*() { return *node_; }

			It& operator++()
			{
				node_ = node_->next_;
				do
				{
					if (node_) break;
					index_++;
					if (index_ >= map_->size_) break;
					node_ = map_->arr_[index_];
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
			return It(this, node, i);
		}

		It end() const { return It(); }

		template<typename R>
		It find(const R& key) const
		{
			s32 index = hash(key) % size_;
			Node* node = _find(key, index);

			return It(this, node, index);
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
				cl_assert(p != nullptr);
				p->next_ = node->next_;
			}
			_del<A>(node); 
			cnt_--;
		}

		template<typename T>
		void remove(const T& key)
		{
			auto it = find(key);
			cl_assert(it.node_ != nullptr);
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
						_del<A>(t); 
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
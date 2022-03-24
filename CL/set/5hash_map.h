#ifndef __cl_set_hash_map__
#define __cl_set_hash_map__

#include "1base.h"

namespace cl
{
	namespace lib
	{
		template<typename Key, typename Val>
		struct HashMapNode : public Pair<Key, Val>
		{
			HashMapNode* next_ = nullptr;
			HashMapNode() {}
		};

		static constexpr uv32 g_Hash_Map_Default_Array_Size = 32;
	}

	//不允许字符串作为字典的关键词
	template<typename Key, typename Val, class MA = MemAllocator>
		requires (MemAllocType<MA> && !IsCharPtr_v<Key>)
	class HashMap
	{ 
	public:
		using Node = lib::HashMapNode<Key, Val>;
		using HMT = HashMap<Key, Val, MA>;
		using MBT = MemBuf<Node*, MA>;
	private:
		MBT buf_;
		uv32 cnt_ = 0;
	public:
		class It
		{
			friend class HMT;
			uv32 index_ = 0;
			const HMT* map_ = nullptr;
			Node* node_ = nullptr;
		public:
			It() {}
			It(const HMT* map, Node* node, uv32 index) : map_(map), node_(node), index_(index) { }

			Pair<Key, Val>* operator->() { return node_; }
			Pair<Key, Val>& operator*() { return *node_; }

			It& operator++()
			{
				node_ = node_->next_;
				do
				{
					if (node_) break;
					index_++;
					if (index_ >= map_->buf_.size()) break;

					node_ = map_->buf_.data()[index_];
				} while (true);
				return *this;
			}
			bool operator ==(const It& it) { return node_ == it.node_; }
			bool operator !=(const It& it) { return node_ != it.node_; }
		};
		friend class It;

		template<typename R>
		inline Node* _find_at(const R& key, uv32 index) const
		{
			auto node = buf_.data()[index];
			while (node && lib::Compare<Key, R>()(node->first, key) != 0) node = node->next_;
			return node;
		}
		inline void _add_at(Node* node, uv32 index)
		{ 
			node->next_ = buf_.data()[index];
			buf_.data()[index] = node;
		}

		inline void _add_node(Node* node)
		{
			uv32 index = hash(node->first) % buf_.size();
			_add_at(node, index); 
		}
		void _move(HashMap& map)
		{ 
			auto& buf = map.buf_;
			for (uv32 i = 0; i < buf.size(); i++)
			{
				auto p = buf.data()[i];
				while (p)
				{
					auto t = p;
					p = p->next_;
					_add_node(t); 
				};
				buf.data()[i] = nullptr;
			}
			cnt_ = map.cnt_;
			map.cnt_ = 0;
		}

	public:
		HashMap(uv32 size = lib::g_Hash_Map_Default_Array_Size)
		{ 
			buf_.alloc(size);
			for (uv32 i = 0; i < size; i++)
				buf_.data()[i] = nullptr;
		}
		HashMap(HashMap&& map) : HashMap(map.buf_.size())  { _move(map); }
		HashMap(const HashMap& map) : HashMap(map.buf_.size()) { this->operator<<(map); }

		~HashMap() { clear(); }

		uv32 count() const { return cnt_; }

		void clear()
		{
			for (uv32 i = 0; i < buf_.size(); ++i)
			{
				auto p = buf_.data()[i];
				while (p)
				{
					auto t = p;
					p = p->next_;
					free_obj<MA, Node>(t);
				};
				buf_.data()[i] = nullptr;
			}
			cnt_ = 0;
		}

		HashMap& operator=(HashMap&& map) noexcept
		{
			clear();
			_move(map); 
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
			for (uv32 i = 0; i < map.buf_.size(); i++)
			{
				auto node = map.buf_.data()[i];
				while (node)
				{
					auto it = find(node->first);
					if (it.node_ == nullptr)
					{
						auto p = alloc_obj<MA, Node>();
						p->first = node->first;
						p->second = node->second;
						 
						_add_node(p);
						cnt_++;
					}
					else 
						it.node_->second = node->second; 

					node = node->next_; 
				}
			}
			
			return *this;
		}

		HashMap& operator<<(HashMap&& map)
		{
			for (uv32 i = 0; i < map.buf_.size(); i++)
			{
				auto node = map.buf_.data()[i];
				map.buf_.data()[i] = nullptr;

				while (node)
				{
					Node* p = node;
					node = node->next_;
					auto it = find(p->first);
					if (it.node_ != nullptr)
					{
						it.node_->second = p->second;
						free_obj<MA>(p);
					}
					else
					{
						_add_node(p);
						cnt_ ++;
					} 
				}
			}
			
			map.cnt_ = 0;
			return *this;
		}

		template<typename R>
		Val& operator[](const R& key)
		{
			uv32 index = hash(key) % buf_.size();
			Node* node = _find_at(key, index);
			if (!node)
			{
				node = alloc_obj<MA, Node>();
				node->first = key;
				_add_at(node, index);
				cnt_++;
			}
			return node->second;
		}

		It begin() const
		{
			if (cnt_ > 0)
			{
				for (uv32 i = 0; i < buf_.size(); ++i)
				{
					auto p = (Node*)buf_.data()[i];
					if (p) return It(this, p, i); 
				}
			}
			return It(this, (Node*)nullptr, 0);
		}

		It end() const { return It(); }

		template<typename R>
		It find(const R& key) const
		{
			uv32 index = hash(key) % buf_.size();
			Node* node = _find_at(key, index);
			return It(this, node, index);
		}

		void remove(const It& it)
		{
			uv32 index = it.index_;
			auto node = it.node_;
			if (node == nullptr) return;

			auto p = buf_.data()[index];

			if (p == node)
				buf_.data()[index] = node->next_;
			else
			{
				while (p->next_ != node) p = p->next_;
				p->next_ = node->next_;
			}
			free_obj<MA>(node);
			cnt_--;
		}

		template<typename T>
		void remove(const T& key)
		{
			auto it = find(key);
			if (it.node_ == nullptr) return;
			remove(it);
		}

		void remove_if(const Call<bool(Pair<Key, Val>*)>& call)
		{
			for (uv32 i = 0; i < buf_.size(); i++)
			{
				Node* new_list = nullptr;
				auto p = buf_.data()[i];
				while (p)
				{
					auto t = p;
					p = p->next_;

					if (call(t))
					{
						--cnt_;
						free_obj<MA>(t);
					}
					else
					{
						t->next_ = new_list;
						new_list = t;
					}
				}
				buf_.data()[i] = new_list;
			}
		}
	};
}

#endif//__cl_set_hash_map__
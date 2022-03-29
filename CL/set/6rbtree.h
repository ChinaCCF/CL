
#ifndef __cl_set_rbtree__
#define __cl_set_rbtree__

#include "1base.h"

//rbtree 在范围查询和并发操作上确实比不上skiplist

namespace cl
{
	namespace lib
	{
		template<typename Key, typename Val>
			requires (!IsCharPtr_v<Key>)
		class RBNode
		{
		public:
			enum class RBColor
			{
				Red,
				Black
			};
			using Node = RBNode;

			Node* left_ = nullptr;
			Node* right_ = nullptr;
			Node* parent_ = nullptr;
			RBColor color_ = RBColor::Red;

			Pair<Key, Val> pair_;

			RBNode() {}

			inline void set_left(Node* node) { left_ = node; if (node) node->parent_ = this; }
			inline void set_right(Node* node) { right_ = node; if (node) node->parent_ = this; }

			inline bool is_red() { return color_ == RBColor::Red; }
			inline bool is_black() { return color_ == RBColor::Black; }

			inline void set_red() { color_ = RBColor::Red; }
			inline void set_black() { color_ = RBColor::Black; }

			inline Key& key() { return pair_.first; }
			inline Val& val() { return pair_.second; }

			//查找下一个比当前大的节点
			Node* next()
			{
				Node* n = this;
				if (n->right_)
				{
					n = n->right_;
					while (n->left_) n = n->left_;
					return n;
				}
				else
				{
					//N的下一个邻近值为G
					//               G
					//              / \ 
					//             P2
					//            / \
					//               P
					//             /  \
					//                 N
					auto p = n->parent_;
					while (p && p->right_ == n)
					{
						n = p;
						p = n->parent_;
					}
					return p;
				}
			}
		};
	}

	//红黑树
	template<typename Key, typename Val, class MA = MemAllocator>
		requires (MemAllocType<MA> && !IsCharPtr_v<Key>)
	class RBTree
	{
		using Node = lib::RBNode<Key, Val>;

		class It
		{
			friend class RBTree<Key, Val>;
			Node* parent_ = nullptr;//用于查找冗余, 方便插入新节点
		public:
			Node* node_ = nullptr;

			It() {}
			Pair<Key, Val>* operator->() { return &node_->pair_; }
			Pair<Key, Val>& operator*() { return node_->pair_; }

			void operator++() { node_ = node_->next(); }

			bool operator ==(const It& it) { return node_ == it.node_; }
			bool operator !=(const It& it) { return node_ != it.node_; }
		};

		Node* root_ = nullptr;
		uv32 cnt_ = 0;

		bool _is_black(Node* node) { return node == nullptr || node->is_black(); }

		//左旋
		//        |          |
		//        N          R
		//       / \        / \
		//          R      N
		void _left_rotate(Node* N)
		{
			Node* R = N->right_;
			{
				Node* P = R->parent_ = N->parent_;
				if (P)
				{
					if (P->left_ == N) P->left_ = R;
					else P->right_ = R;
				}
				else
					root_ = R;
			}

			N->set_right(R->left_);
			R->left_ = N; N->parent_ = R;
		}
		//右旋
		//        |          |
		//        N          L
		//       / \        / \
		//      L              N
		void _right_rotate(Node* N)
		{
			Node* L = N->left_;
			{
				Node* P = L->parent_ = N->parent_;
				if (P)
				{
					if (P->left_ == N) P->left_ = L;
					else P->right_ = L;
				}
				else
					root_ = L;
			}
			N->set_left(L->right_);
			L->right_ = N; N->parent_ = L;
		}

		void _add_child(Node* P, Node* N)
		{
			cnt_++;
			if (P)
			{
				if (P->key() > N->key())
					P->left_ = N;
				else
					P->right_ = N;
				N->parent_ = P;
			}
			else
				root_ = N;
		}
		//删除节点和节点的子节点
		void _del_node(Node* N)
		{
			if (N == nullptr) return;
			_del_node(N->left_);
			_del_node(N->right_);
			free_obj<MA>(N);
		}
		//用于复制树
		Node* _new_node(const Node* N)
		{
			if (N == nullptr) return nullptr;

			Node* M = alloc_obj<MA, Node>();
			M->color_ = N->color_;
			M->pair_ = N->pair_;
			M->left_ = _new_node(N->left_);
			M->right_ = _new_node(N->right_);
			if (M->left_) M->left_->parent_ = M;
			if (M->right_) M->right_->parent_ = M;
			return M;
		}
		//顺序弹出节点, 用于std::move 语义
		Node* _pop_node(Node* node)
		{
			if (node->left_) { auto n = _pop_node(node->left_); if (n == node->left_) node->left_ = nullptr; return n; }
			if (node->right_) { auto n = _pop_node(node->right_); if (n == node->right_) node->right_ = nullptr; return n; }
			return node;
		}

		void _move(RBTree&& tree)
		{
			root_ = tree.root_;
			cnt_ = tree.cnt_;
			tree.root_ = nullptr;
			tree.cnt_ = 0;
		}
	public:
		RBTree() { }
		RBTree(const RBTree& tree)
		{
			root_ = _new_node(tree.root_);
			cnt_ = tree.cnt_;
		}
		RBTree(RBTree&& tree) noexcept { _move(std::move(tree)); }

		RBTree& operator=(const RBTree& tree)
		{
			_del_node(root_);
			root_ = _new_node(tree.root_);
			cnt_ = tree.cnt_;
		}
		RBTree& operator=(RBTree&& tree) noexcept
		{
			_del_node(root_);
			_move(std::move(tree));
		}

		~RBTree() { _del_node(root_); }

		uv32 size() { return cnt_; }

		void clear() { _del_node(root_); root_ = nullptr; cnt_ = 0; }

		//若本树存在某个key,会覆盖其值
		RBTree& operator<<(const RBTree& tree)
		{
			for (auto& p : tree)
			{
				It it = find(p.first);
				if (it.node_)
				{
					it.node_->val() = p.second;
				}
				else
				{
					Node* node = alloc_obj<MA, Node>();
					node->key() = p.first;
					node->val() = p.second;
					_add_child(it.parent_, node);
					_insert_repair(node);
				}
			}
			return *this;
		}

		//若本树存在某个key,会覆盖其值
		RBTree& operator<<(RBTree&& tree)
		{
			if (tree.root_ != nullptr)
			{
				while (true)
				{
					Node* node = tree._pop_node(tree.root_);
					It it = find(node->key());
					if (it.node_ != nullptr)
					{
						it.node_->val() = std::move(node->val());
						free_obj<MA>(node);
					}
					else
					{
						_add_child(it.parent_, node);
						node->set_red();
						_insert_repair(node);
					}

					if (node == tree.root_)
						break;
				}
				tree.cnt_ = 0;
				tree.root_ = nullptr;
			}
			return *this;
		}

		It begin() const
		{
			It it;
			auto node = root_;
			if (node)
			{
				while (node->left_) node = node->left_;
				it.node_ = node;
			}
			return it;
		}
		It end() const { It it; return it; }

		template<typename T>
		It find(const T& key) const
		{
			It it;
			Node* parent = nullptr;
			Node* node = root_;
			while (node)
			{
				if (node->key() == key) break;
				parent = node;
				if (node->key() > key)
					node = node->left_;
				else
					node = node->right_;
			}
			it.node_ = node;
			it.parent_ = parent;
			return it;
		}

		void remove(const It& it)
		{
			Node* D = it.node_;

			cnt_--;

			Node* D_child = nullptr;
			if (D->left_ == nullptr)
				D_child = D->right_;
			else
			{
				if (D->right_ == nullptr)
					D_child = D->left_;
				else
				{
					//查找删除替代节点
					Node* tmp = D->right_;
					while (tmp->left_) tmp = tmp->left_;
					D_child = tmp->right_;
					if (tmp != D)
					{
						//node的key和val不用管, 因为要删除node
						D->key() = std::move(tmp->key());
						D->val() = std::move(tmp->val());
						D = tmp;
					}
				}
			}

			Node* parent = D->parent_;
			if (D_child) D_child->parent_ = parent;
			if (parent)
			{
				if (parent->left_ == D)
					parent->left_ = D_child;
				else
					parent->right_ = D_child;
			}
			else
				root_ = D_child;

			if (D->is_black()) _remove_repair(parent, D_child);
			free_obj<MA>(D);
		}

		template<typename T>
		void remove(const T& key)
		{
			It it = find(key);
			CL_Assert(it.node_ != nullptr);
			remove(it);
		}

		template<typename T>
		Val& operator[](const T& key)
		{
			It it = find(key);
			if (it.node_) return it.node_->val();

			Node* node = alloc_obj<MA, Node>();
			node->key() = key;
			_add_child(it.parent_, node);
			_insert_repair(node);
			return node->val();
		}

	private:
		void _insert_repair(Node* N)
		{
			Node* parent;
			Node* uncle;
			Node* grandparent;
		Label_Redo:
			parent = N->parent_;
			if (parent && parent->is_red())
			{
				grandparent = parent->parent_;

				if (grandparent->left_ == parent)
				{
					uncle = grandparent->right_;
					if (uncle && uncle->is_red())
					{
						uncle->set_black();
						parent->set_black();
						grandparent->set_red();
						N = grandparent;
						goto Label_Redo;
					}
					else
					{
						if (parent->right_ == N)
						{
							_left_rotate(parent);
							N = parent;
							goto Label_Redo;
						}
						else
						{
							parent->set_black();
							grandparent->set_red();
							_right_rotate(grandparent);
							return;
						}
					}
				}
				else
				{
					uncle = grandparent->left_;
					if (uncle && uncle->is_red())
					{
						uncle->set_black();
						parent->set_black();
						grandparent->set_red();
						N = grandparent;
						goto Label_Redo;
					}
					else
					{
						if (parent->left_ == N)
						{
							_right_rotate(parent);
							N = parent;
							goto Label_Redo;
						}
						else
						{
							parent->set_black();
							grandparent->set_red();
							_left_rotate(grandparent);
							return;
						}
					}
				}
			}
			root_->set_black();
		}

		void _remove_repair(Node* parent, Node* node)
		{
			Node* brother;
			while (node != root_ && _is_black(node))
			{
				if (parent->left_ == node)
				{
					brother = parent->right_;

					//case 1
					if (brother->is_red())
					{
						brother->set_black();
						parent->set_red();
						_left_rotate(parent);
						brother = parent->right_;
					}
					//case 2
					if (_is_black(brother->left_) && _is_black(brother->right_))
					{
						brother->set_red();
						node = parent;
						parent = node->parent_;
					}
					else
					{
						//case 3
						if (_is_black(brother->right_))
						{
							brother->left_->set_black();
							brother->set_red();
							_right_rotate(brother);
							brother = parent->right_;
						}
						//case 4
						{
							brother->color_ = parent->color_;
							parent->set_black();
							brother->right_->set_black();
							_left_rotate(parent);
							node = root_;
						}
					}
				}
				else
				{
					brother = parent->left_;

					//case 1
					if (brother->is_red())
					{
						brother->set_black();
						parent->set_red();
						_right_rotate(parent);
						brother = parent->left_;
					}
					//case 2
					if (_is_black(brother->left_) && _is_black(brother->right_))
					{
						brother->set_red();
						node = parent;
						parent = node->parent_;
					}
					else
					{
						//case 3
						if (_is_black(brother->left_))
						{
							brother->right_->set_black();
							brother->set_red();
							_left_rotate(brother);
							brother = parent->left_;
						}
						//case 4
						{
							brother->color_ = parent->color_;
							parent->set_black();
							brother->left_->set_black();
							_right_rotate(parent);
							node = root_;
						}
					}
				}
			}
			if (node) node->set_black();
		}
	};
}

#endif//__cl_set_rbtree__
#ifndef __cl_set_list__
#define __cl_set_list__
 
#include "../base/3type_traits.h" 
#include "../base/7call.h"

#include "1base.h"

namespace cl
{
    namespace lib
    { 
        template<typename T>
        struct ListNode
        {
            ListNode* pre_ = nullptr;
            ListNode* next_ = nullptr;
            T val_;

            template<typename ... Args>
			ListNode(Args&&... args) : val_(std::forward<Args>(args)...) {}
        };

        template<typename T>
        ListNode<T>* list_merge(ListNode<T>* n1, ListNode<T>* n2, const Call<sv32(T*, T*)>& cmp)
        {
            ListNode<T>* head;
            if (cmp(&n1->val_, &n2->val_) <= 0)
            {
                head = n1;
                n1 = n1->next_;
            }
            else
            {
                head = n2;
                n2 = n2->next_;
            }
            ListNode<T>* p = head;
            while (n1 && n2)
            {
                if (cmp(&n1->val_, &n2->val_) <= 0)
                {
                    p->next_ = n1;
                    n1 = n1->next_;
                }
                else
                {
                    p->next_ = n2;
                    n2 = n2->next_;
                }
                p = p->next_;
            }
            if (n1) p->next_ = n1;
            if (n2) p->next_ = n2;
            return head;
        }

        template<typename T>
        ListNode<T>* list_merge_sort(ListNode<T>* node, uv32 cnt, const Call<sv32(T*, T*)>& cmp)
        {
            if (cnt <= 1)
            {
                node->next_ = nullptr;
                return node;
            }

            uv32 c1 = cnt / 2;
            ListNode<T>* n1 = node;

            uv32 c2 = cnt - c1;
            ListNode<T>* n2 = n1; for (uv32 i = 0; i < c1; i++) n2 = n2->next_;

            n1 = list_merge_sort(n1, c1, cmp);
            n2 = list_merge_sort(n2, c2, cmp);

            node = list_merge(n1, n2, cmp);
            return node;
        }

      
        template<typename T>
        class ListIt
        {
            using Node = lib::ListNode<T>; 
        public:
            Node* node_;

            ListIt(Node* cur = nullptr) : node_(cur) {}

            T& operator*() { return node_->val_; }
            T* operator->() { return &node_->val_; }

            ListIt& operator++() { node_ = node_->next_; return *this; }
            bool operator==(const ListIt& it) { return node_ == it.node_; }
            bool operator!=(const ListIt& it) { return node_ != it.node_; }
        };
    }
  
    template<typename T, class MA = MemAllocator> requires MemAllocType<MA>
    class List
    {
        using ThisType = List<T, MA>;
        using Node = lib::ListNode<T>;

        struct HeadNode
        {
            Node* pre_ = nullptr;
            Node* next_ = nullptr;
        };

        uv32 cnt_ = 0;
        HeadNode head_;
        HeadNode tail_; 
        Node* head() const { return (Node*)&head_; }
        Node* tail() const { return (Node*)&tail_; }

        inline void _a2b(Node* a, Node* b)
        {
            a->next_ = b;
            b->pre_ = a;
        }
        inline void _push_front_node(Node* node)
        {
            _a2b(node, head()->next_);
            _a2b(head(), node);
            cnt_++;
        }
        inline void _push_back_node(Node* node)
        {
            _a2b(tail()->pre_, node);
            _a2b(node, tail());
            cnt_++;
        }
        inline Node* _pop_front_node()
        {
            CL_Assert(cnt_ > 0);
            auto node = head()->next_;
            _a2b(head(), node->next_);
            cnt_--;
            return node;
        }
        inline Node* _pop_back_node()
        {
            CL_Assert(cnt_ > 0);
            auto node = tail()->pre_;
            _a2b(node->pre_, tail());
            cnt_--;
            return node;
        }
        inline void _move(ThisType* list)
        {
            cnt_ = list->cnt_;
            _a2b(head(), list->head()->next_);
            _a2b(list->tail()->pre_, tail());
            _a2b(list->head(), list->tail());
            list->cnt_ = 0;
        }
        inline void _append(const ThisType& list)
        {
            if (list.cnt_ == 0) return;

            cnt_ += list.cnt_;
            auto self = tail()->pre_;
            auto p = list.head()->next_;
            do
            {
                auto node = alloc_obj<MA, Node>(p->val_); 
                _a2b(self, node);
                self = node;
                p = p->next_;
            } while (p != list.tail());
            _a2b(self, tail());
        }
        //返回下一节点
        inline Node* _remove_node(Node* node)
        {
            cnt_--;
            auto pre = node->pre_;
            auto next = node->next_;
            _a2b(pre, next);
            free_obj<MA>(node); 
            return next;
        }

        //交换2个相邻的节点
        inline void _swap_adjoin_node(Node* p1, Node* p2)
        {
            auto h = p1->pre_; 
            auto t = p2->next_;
            _a2b(h, p2);
            _a2b(p2, p1);
            _a2b(p1, t);
        }

        inline Node* _node_at_index(uv32 index)
        {
            auto p = head()->next_;
            while (index-- && p != tail())
                p = p->next_;
			CL_Assert(index == -1 && p != tail());
            return p;
        }
    public:
		List() { _a2b(head(), tail()); }
        List(const ThisType& list) 
        {
            _a2b(head(), tail());
            _append(list);
        }
        List(ThisType&& list) noexcept 
        {
            _a2b(head(), tail());
            _move(&list);
        }
        //#############################################
        List& operator=(const ThisType& list)
        {
            clear();
            _append(list);
            return *this;
        }
        List& operator=(ThisType&& list) noexcept
        {
            clear();
            _move(&list);
            return *this;
        }
        //#############################################
        List& operator<<(const ThisType& list)
        {
            _append(list);
            return *this;
        }
        List& operator<<(ThisType&& list)
        {
            cnt_ += list.cnt_;
            _a2b(tail()->pre_, list.head()->next_);
            _a2b(list.tail()->pre_, tail());
            _a2b(list.head(), list.tail());
            list.cnt_ = 0;
            return *this;
        }

        ~List()
        {
            auto node = head()->next_;
            while (node != tail())
            {
                auto tmp = node;
                node = node->next_;

                free_obj<MA>(tmp);
            }
        }
        void clear()
        {
            if (cnt_ == 0) return;
            this->~List();
            _a2b(head(), tail());
            cnt_ = 0;
        }

        uv32 size() const { return cnt_; }
        T& front() const { return head()->next_->val_; }
        T& back() const { return tail()->pre_->val_; }

        T& new_front() 
        {
            auto node = alloc_obj<MA, Node>(); 
            _push_front_node(node);
            return node->val_;
        }

        T& new_back() 
        {
            auto node = alloc_obj<MA, Node>();
            _push_back_node(node);
            return node->val_;
        }

        template<typename ... Args>
        void push_front(Args&& ... args)
        {
            auto node = alloc_obj<MA, Node>(std::forward<Args>(args)...); 
            _push_front_node(node);
        }

        template<typename ... Args>
        void push_back(Args&& ... args)
        { 
            auto node = alloc_obj<MA, Node>(std::forward<Args>(args)...);
            _push_back_node(node);
        }

        T pop_front()
        {
            auto node = _pop_front_node();
            T val = std::move(node->val_);
            free_obj<MA>(node); 
            return val;
        }
        T pop_back()
        {
            auto node = _pop_back_node();
            T val = std::move(node->val_);
            free_obj<MA>(node);
            return val;
        }

        //index 从0开始计数
        template<typename ... Args>
        void insert_at(uv32 index, Args&& ... args)
        {
            Node* p = _node_at_index(index); 
            cnt_++;

            auto node = alloc_obj<MA, Node>(std::forward<Args>(args)...); 

            auto pre = p->pre_;
            _a2b(pre, node);
            _a2b(node, p);
        }
         
        //反转链表
        void invert()
        {
            auto t = tail()->pre_;
            auto h = head();
            while (t != head())
            {
                auto p = t;
                t = t->pre_;

                _a2b(h, p);
                h = p;
            }
            _a2b(h, tail());
        }

        T& operator[](uv32 index) const
        {
            Node* p = _node_at_index(index); 
            return p->val_;
        }
          
        lib::ListIt<T> begin() const { return lib::ListIt<T>(head()->next_); }
        lib::ListIt<T> end() const { return lib::ListIt<T>(tail()); }

        template<typename T2>
        lib::ListIt<T> find(const T2& val) const
        {
            auto p = head()->next_;
			while (p != tail() && lib::Compare<T, T2>()(p->val_, val) != 0)
                p = p->next_;
            return lib::ListIt<T>(p);
        } 

        void remove(const lib::ListIt<T>& it)
        {
            auto node = it.node_;
            _remove_node(node);
        }
        template<typename T2>
        void remove(const T2& val)
        {
            auto it = find(val);
            CL_Assert(it.node_ != tail());
            remove(it);
        }
        void remove_if(const Call<bool(T*)>& call)
        {
            auto p = head()->next_;
            while (p != tail())
            {
                if (call(&p->val_))
                    p = _remove_node(p);
                else
                    p = p->next_;
            }
        }
          
		void mov_up(uv32 index)
		{
			if (index <= 0) return;
			auto p = _node_at_index(p);
			_swap_adjoin_node(p->pre_, p);
		}
		void mov_down(uv32 index)
		{
			if (index >= cnt_) return;
			auto p = _node_at_index(p);
			_swap_adjoin_node(p, p->next_);
		}
    public:
        void sort(const Call<sv32(T*, T*)>& cmp)
        {
            if (cnt_ <= 1) return;

            head()->next_ = lib::list_merge_sort(head()->next_, cnt_, cmp);

            //下面的操作是补齐链表的pre字段, 使单链表变双链表
            auto n = head()->next_;
            auto p = head();
            while (n != tail() && n)
            {
                n->pre_ = p;
                p = n;
                n = n->next_;
            }
            _a2b(p, tail());
        }
    };
}

#endif//__cl_set_list__
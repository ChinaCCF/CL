#ifndef __clan_set_list__
#define __clan_set_list__
 
#include "../base/4concept.h"
#include "../base/5exception.h"
#include "../base/11call.h"

#include "1base.h"

namespace cl
{
    namespace detail
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
        ListNode<T>* list_merge(ListNode<T>* n1, ListNode<T>* n2, const Call<s32(T*, T*)>& cmp)
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
        ListNode<T>* list_merge_sort(ListNode<T>* node, s32 cnt, const Call<s32(T*, T*)>& cmp)
        {
            if (cnt <= 1)
            {
                node->next_ = nullptr;
                return node;
            }

            s32 c1 = cnt / 2;
            ListNode<T>* n1 = node;

            s32 c2 = cnt - c1;
            ListNode<T>* n2 = n1; for (s32 i = 0; i < c1; i++) n2 = n2->next_;

            n1 = list_merge_sort(n1, c1, cmp);
            n2 = list_merge_sort(n2, c2, cmp);

            node = list_merge(n1, n2, cmp);
            return node;
        }
    }
  
    template<typename T, AllocMemType A>
    class _List
    {
        using ThisType = _List<T, A>;
        using Node = detail::ListNode<T>;

        struct HeadNode
        {
            Node* pre_ = nullptr;
            Node* next_ = nullptr;
        };
         
        char buf_[sizeof(HeadNode) * 2];
        Node* head() const { return (Node*)buf_; }
        Node* tail() const { return (Node*)(((HeadNode*)buf_) + 1); }

        s32 cnt_ = 0;

        inline void _a2b(Node* a, Node* b)
        {
            a->next_ = b;
            b->pre_ = a;
        }
        inline void _push_front_node(Node* node)
        {
            _a2b(node, (Node*)head()->next_);
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
            cl_assert(cnt_ > 0);
            auto node = (Node*)head()->next_;
            _a2b(head(), (Node*)node->next_);
            cnt_--;
            return node;
        }
        inline Node* _pop_back_node()
        {
            cl_assert(cnt_ > 0);
            auto node = (Node*)tail()->pre_;
            _a2b((Node*)node->pre_, tail());
            cnt_--;
            return node;
        }
        inline void _move(ThisType* list)
        {
            cnt_ = list->cnt_;
            _a2b(head(), (Node*)list->head()->next_);
            _a2b((Node*)list->tail()->pre_, tail());
            _a2b(list->head(), list->tail());
            list->cnt_ = 0;
        }
        inline void _append(const ThisType& list)
        {
            if (list.cnt_ == 0) return;

            cnt_ += list.cnt_;
            auto self = tail()->pre_;
            auto p = (Node*)list.head()->next_;
            do
            {
                auto node = _new<A, Node>(p->val_); 
                _a2b(self, node);
                self = node;
                p = (Node*)p->next_;
            } while (p != list.tail());
            _a2b(self, tail());
        }
        //返回下一节点
        inline Node* _remove_node(Node* node)
        {
            cnt_--;
            auto pre = (Node*)node->pre_;
            auto next = (Node*)node->next_;
            _a2b(pre, next);
            _del<A>(node); 
            return next;
        }

        inline void _swap_adjoin_node(Node* p1, Node* p2)
        {
            auto h = (Node*)p1->pre_; 
            auto t = (Node*)p2->next_;
            _a2b(h, p2);
            _a2b(p2, p1);
            _a2b(p1, t);
        }

        inline Node* _find_node(s32 index)
        {
            Node* p = (Node*)head()->next_;
            while (index-- && p != tail())
                p = p->next_;
			cl_assert(index == -1 && p != tail());
            return p;
        }
    public:
        _List()
        {
            _a2b(head(), tail());
            head()->pre_ = nullptr;
            tail()->next_ = nullptr;
        }
        _List(const ThisType& list) : _List()
        {
            _append(list);
        }
        _List(ThisType&& list) noexcept : _List()
        {
            _move(&list);
        }
        //#############################################
        _List& operator=(const ThisType& list)
        {
            clear();
            _append(list);
            return *this;
        }
        _List& operator=(ThisType&& list) noexcept
        {
            clear();
            _move(&list);
            return *this;
        }
        //#############################################
        _List& operator<<(const ThisType& list)
        {
            _append(list);
            return *this;
        }
        _List& operator<<(ThisType&& list)
        {
            cnt_ += list.cnt_;
            _a2b((Node*)tail()->pre_, (Node*)list.head()->next_);
            _a2b((Node*)list.tail()->pre_, tail());
            _a2b(list.head(), list.tail());
            list.cnt_ = 0;
            return *this;
        }

        ~_List()
        {
            auto node = (Node*)head()->next_;
            while (node != tail())
            {
                auto tmp = node;
                node = (Node*)node->next_;
                _del<A>(node); 
            }
        }
        void clear()
        {
            if (cnt_ == 0) return;
            this->~_List();
            _a2b(head(), tail());
            cnt_ = 0;
        }

        s32 size() const { return cnt_; }
        T& front() const { return head()->next_->val_; }
        T& back() const { return tail()->pre_->val_; }

        template<typename ... Args>
        void push_front(Args&& ... args)
        {
            auto node = _new<A, Node>(std::forward<Args>(args)...);
            _push_front_node(node);
        }

        template<typename ... Args>
        void push_back(Args&& ... args)
        {
            auto node = _new<A, Node>(std::forward<Args>(args)...);
            _push_back_node(node);
        }

        T pop_front()
        {
            auto node = _pop_front_node();
            T val = std::move(node->val_);
            _del<A>(node); 
            return val;
        }
        T pop_back()
        {
            auto node = _pop_back_node();
            T val = std::move(node->val_);
            _del<A>(node);
            return val;
        }

        template<typename ... Args>
        void insert_at(s32 index, Args&& ... args)
        {
            Node* p = _find_node(index); 
            cnt_++;
            auto node = _new<A, Node>(std::forward<Args>(args)...);

            auto pre = (Node*)p->pre_;
            _a2b(pre, node);
            _a2b(node, p);
        }
         
        void revert()
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

        T& operator[](s32 index) const
        {
            Node* p = _find_node(index); 
            return p->val_;
        }

        class It
        {
            Node* node_;
            friend class _List;
        public:
            It(Node* cur = nullptr) : node_(cur) {}

            T& operator*() { return node_->val_; }
            T* operator->() { return &node_->val_; }

            It& operator++() { node_ = (Node*)node_->next_; return *this; }
            bool operator==(const It& it) { return node_ == it.node_; }
            bool operator!=(const It& it) { return node_ != it.node_; }
        };

        It begin() const { return It(head()->next_); }
        It end() const { return It(tail()); }

        template<typename R>
        It find(const R& val) const
        {
            auto p = (Node*)head()->next_;
			while (p != tail() && CmpVal<T>()(p->val_, val) != 0)
                p = (Node*)p->next_;
            return It(p);
        } 

        void remove(const It& it)
        {
            auto node = it.node_;
            _remove_node(node);
        }
        template<typename R>
        void remove(const R& val)
        {
            auto it = find(val);
            cl_assert(it.node_ != tail());
            remove(it);
        }
        void remove_if(const Call<bool(T*)>& call)
        {
            auto p = (Node*)head()->next_;
            while (p != tail())
            {
                if (call(&p->val_))
                    p = _remove_node(p);
                else
                    p = p->next_;
            }
        }
          
		void mov_up(s32 index)
		{
			if (index <= 0) return;
			auto p = _find_node(p);
			_swap_adjoin_node((Node*)p->pre_, p);
		}
		void mov_down(s32 index)
		{
			if (index >= cnt_) return;
			auto p = _find_node(p);
			_swap_adjoin_node(p, (Node*)p->next_);
		}
    public:
        void sort(const Call<s32(T*, T*)>& cmp)
        {
            if (cnt_ <= 1) return;

            head()->next_ = detail::list_merge_sort(head()->next_, cnt_, cmp);
            auto n = (Node*)head()->next_;
            auto p = head();
            while (n != tail() && n)
            {
                n->pre_ = p;
                p = n;
                n = (Node*)n->next_;
            }
            _a2b(p, tail());
        }
    };
}

#endif//__clan_set_list__
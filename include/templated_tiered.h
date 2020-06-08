/********************************************************************************
* MIT License
* 
* Copyright (c) 2017 Mikko Berggren Ettienne 
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
********************************************************************************/
#ifndef _TEMPLATED_TIERED_H_
#define _TEMPLATED_TIERED_H_

#include <vector>
#include <cstring>
#include <ctime>

#include <iostream>
#include <cmath>
#include <assert.h>
#include <stack>
#include <queue>
#include <list>
#include <bitset>

#ifdef PPACK
#define INODE FakeNode<Elem>
#else
#define INODE FakeNode<void *> //inner node
#endif
#define LNODE FakeNode<T> // leaf node

#define WRAP(a,b) (((a) + (b)) % (b))

using namespace std;
int myPow(int x, int p)
{
    if (p == 0) return 1;
    if (p == 1) return x;

    int tmp = myPow(x, p/2);
    if (p%2 == 0) return tmp * tmp;
    else return x * tmp * tmp;
}
namespace Seq
{
    struct Info {
#ifdef ARRAY
        size_t* offsets;
#ifdef PACK
#else
        void** ptrs;
#endif
#ifdef PFREE
        void* elems;
#endif
#endif
    };

    struct Elem {
        size_t offset;
        size_t child;
    };

    template <size_t Num>
    struct Math {
       enum { log = Math<(Num + 1) / 2>::log + 1, logdown = Math<Num / 2>::logdown + 1 };
    };

    template <>
    struct Math<1> {
       enum { log = 0, logdown = 0 };
    };

    template <size_t Num>
    struct Pow2 {
       enum { value = Pow2<Num - 1>::value * 2 };
    };

    template <>
    struct Pow2<0> {
       enum { value = 1 };
    };

    struct LayerEnd { typedef LayerEnd child; enum { width = 0, capacity = 0, height = 0, nodes = 0, depth = 0 }; };

    template <size_t Width, typename NextType = LayerEnd>
    struct Layer { 
        enum { width = Width, capacity = NextType::capacity * Width, height = NextType::height + 1, nodes = NextType::nodes * Width + 1 };
        typedef NextType child;
    };

    template <size_t Width>
    struct Layer<Width, LayerEnd> { 
        enum { width = Width, capacity = Width, height = 0, nodes = 1 };
        typedef LayerEnd child;
    };

    template<typename Parents, typename Childs>
    struct LayerItr {
        typedef LayerItr<Layer<Childs::width, Parents>, typename Childs::child> child;
        typedef LayerItr<typename Parents::child, Layer<Parents::width, Childs> > parent;
        
        enum { 
            width = Childs::width, 
            capacity = Childs::capacity, 
            height = Childs::height, 
            nodes = Childs::nodes, 
            depth = Parents::height, 
            leaves = (unsigned long long)parent::width * parent::leaves, 
            top_nodes = parent::top_nodes + leaves,
            bit_width = Math<capacity>::log,
            offsets_per = Pow2<Math<sizeof(size_t) * 8 / bit_width>::logdown>::value,
            top_width = parent::top_width + (leaves + offsets_per - 1) / offsets_per
        };
    };

    struct FakeParent {
        typedef FakeParent parent;
        enum { top_nodes = 0, width = 1, top_width = 0 };
    };

    template<typename Childs>
    struct LayerItr<LayerEnd, Childs> {
        typedef LayerItr<Layer<Childs::width, LayerEnd>, typename Childs::child> child;
        typedef FakeParent parent;

        enum { width = Childs::width, capacity = Childs::capacity, height = Childs::height, nodes = Childs::nodes, depth = 0, leaves = 1, top_nodes = 1, bit_width = Math<capacity>::log, offsets_per = 1, top_width = 1 };
    };

    template <class T, size_t width>
        class Node {
            public:
                Node(size_t depth);
                size_t depth;
                size_t size = 0;
                size_t id;
#ifdef PPACK
#else
                size_t offset = 0;
#endif
                T elems[width];
        };

    template <class T>
        class FakeNode {
            public:
                size_t depth;
                size_t size = 0;
                size_t id;
#ifdef PPACK
#else
                size_t offset = 0;
#endif
                T elems[];
        };

    template <class T, class Layer>
        class Tiered {

            public:
                Info info;
                size_t size = 0;
#ifdef ARRAY
                const static size_t root = 0;
#else

#ifdef PPACK

#define root ((size_t) &relem)
                Elem relem;

#else
                size_t root;
#endif
#endif

                int print_helper(INODE * node, int n);

                T replace(T const elem, INODE * node, size_t index);
                T pop_push(T elem, INODE * node, size_t from, size_t count, bool goRight);
                size_t make_room(size_t node, size_t idx);

                void fill(T *&res, INODE * node, size_t from, size_t count);

                Tiered();
                void print();
                void drawString();
                void fill(T *res);
                void remove(size_t idx);

                T sum(size_t from, size_t count);
                size_t successor(T elem);

                void insert(size_t idx, T elem);
                void insert_sorted(T elem);

                const T& operator[](size_t idx) const;
                void randomize();

        void drawTree();
    };

};

#endif

#define TT template <class T, class Layer>


size_t ID = 0;


namespace Seq
{
    TT
    struct helper {

#ifdef ARRAY
        static size_t get_offset(size_t addr, Info info){
#ifdef LINE
            if(Layer::height % 2 == 0){
                return info.offsets[addr + Layer::parent::parent::top_nodes];
            }
            else{
                return info.offsets[addr + Layer::parent::top_nodes];
            }
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = Layer::parent::top_width + addr / Layer::offsets_per;
            return (info.offsets[pos] >> (Layer::bit_width * (addr % Layer::offsets_per))) & (((size_t)1 << Layer::bit_width) - 1);
            
#else
            addr += Layer::parent::top_nodes;
#endif
#endif
            return info.offsets[addr];
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef LINE
            if(Layer::height % 2 == 0){
                addr =addr + Layer::parent::parent::top_nodes;
            }
            else{
                addr =addr + Layer::parent::top_nodes;
            }
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = Layer::parent::top_width + addr / Layer::offsets_per;
            size_t mask = (((size_t)1 << Layer::bit_width) - 1) << (Layer::bit_width * (addr % Layer::offsets_per));
            info.offsets[pos] = (info.offsets[pos] & ~mask) | (offset << (Layer::bit_width * (addr % Layer::offsets_per)));
            return;            

#else
            addr += Layer::parent::top_nodes;
#endif
#endif
            info.offsets[addr] = offset;
        }
        static size_t get_child(size_t addr, size_t idx){
#ifdef LINE
            if(Layer::height % 2 == 0){
                return (addr - (addr/(Layer::parent::width + 1))) * (Layer::width + Layer::width * Layer::child::width) + idx*(Layer::child::width+1);
            }
            else{
                return addr + idx + 1;
            }
#elif defined(LEVEL)
            return Layer::width * addr + idx;
#endif
            return Layer::child::nodes * idx + 1 + addr;
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
            idx = (idx + get_offset(addr, info)) % Layer::capacity;
            auto childIdx = idx / Layer::child::capacity;

            return helper<T, typename Layer::child>::make_room(
                   get_child(addr, childIdx), idx, info);


        }
#else
        static size_t get_offset(size_t addr, Info info){
#ifdef PPACK
            return ((Elem*) addr)->offset;
#else
            return ((INODE*) addr)->offset;
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef PPACK
            ((Elem*) addr)->offset = offset;
#else
            ((INODE*) addr)->offset = offset;
#endif
        }
        static size_t get_child(size_t addr, size_t idx){
#ifdef PPACK
            Elem* node = (Elem*) addr;
            auto child = &((INODE*)node->child)->elems[idx];
#else
            INODE* node = (INODE*) addr;
            auto child = node->elems[idx];
#endif
            return (size_t) child;
        }

        static size_t make_room(size_t addr, size_t idx, Info info) {
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto node = (INODE*) elem->child;
            idx = (idx + elem->offset) % Layer::capacity;
            auto childIdx = idx / Layer::child::capacity;
            if (node->elems[childIdx].child == NULL) {
#else
            auto node = (INODE*) addr;
            idx = (idx + node->offset) % Layer::capacity;
            auto childIdx = idx / Layer::child::capacity;
            if (node->elems[childIdx] == NULL) {
#endif
                if (Layer::height == 1) {
#ifdef PPACK

                    node->elems[childIdx] = {0, (size_t)new Node<T, Layer::child::width>(0)};
#else
                    node->elems[childIdx] = new Node<T, Layer::child::width>(0);
#endif
                } else {
#ifdef PPACK
                    node->elems[childIdx] = {0, (size_t)new Node<Elem, Layer::child::width>(0)};
#else
                    node->elems[childIdx] = new Node<void*, Layer::child::width>(0);
#endif
                }
                node->size++;
            }

#ifdef PPACK
            return helper<T, typename Layer::child>::make_room((size_t)&node->elems[childIdx], idx, info);
#else
            return helper<T, typename Layer::child>::make_room((size_t)node->elems[childIdx], idx, info);
#endif
        }
#endif
        static T& get(size_t addr, size_t idx, Info info) {
            idx = (idx + get_offset(addr, info)) % Layer::capacity;
            auto child = get_child(addr, idx/ Layer::child::capacity);
            return helper<T, typename Layer::child>::get(child, idx, info);
        }
        static size_t get_leaf(size_t addr, size_t idx, Info info) {
            idx = (idx + get_offset(addr, info)) % Layer::capacity;
            auto child = get_child(addr, idx/ Layer::child::capacity);
            return helper<T, typename Layer::child>::get_leaf(child, idx, info);
        }
        static void drawTree(size_t idx, list<size_t>&lst, int & rank, int & exp) {
            size_t cur = lst.front();
            lst.pop_front();
            INODE * node = (INODE*) cur;
            if (node == NULL) {
                printf("EMPTY | ");
                return;
            }
            int offset = (idx + get_offset(cur, Info{})) % Layer::capacity;
            printf("[%d, %d, %d]", offset, node->size, node->id);
            for (int i = 0; i < Layer::width; ++i) {
                printf(" _");
            }
            printf(" | ");
            for (int j = 0; j < Layer::width; ++j) {
                auto child = get_child(cur, j % Layer::child::capacity);
                lst.push_back(child);
            }
            rank++;
            if (rank == myPow(Layer::width, exp)) {
                rank = 0;
                exp++;
                printf("\n");
            }
            for (int k = 0; k < Layer::width; ++k) {
                helper<T, typename Layer::child>::drawTree(idx, lst, rank, exp);
            }
        }
        static int* drawString(size_t addr, size_t idx, Info info, size_t size) {
            idx = (idx + get_offset(addr, info)) % Layer::capacity;
            int * ans = new int[size];
            int curSize = 0;
            for (int j = 0; j < Layer::width; ++j) {
                auto child = get_child(addr, idx/ Layer::child::capacity);
                int * re = helper<T, typename Layer::child>::drawString(child, idx, info, Layer::child::capacity);
                for (int i = 0; i < Layer::child::capacity; ++i) {
                    cout<<re[i]<<" ";
                }
                cout<<endl;
                copy(re, re+Layer::child::capacity, ans+j*Layer::child::capacity);
                delete []re;
                idx = (idx+Layer::child::capacity) ;
                curSize += Layer::child::capacity;
                if (curSize >= size) {
                    break;
                }
            }
            return ans;
        }


        static bool remove_room(size_t addr, size_t idx) {
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto node = (INODE*) elem;
            idx = (idx + elem->offset) % Layer::capacity;
#else
            auto node = (INODE*) addr;
            idx = (idx + node->offset) % Layer::capacity;
#endif
            auto childIdx = idx / Layer::child::capacity;

            if (helper<T, typename Layer::child>::remove_room((INODE *)node->elems[childIdx], idx)) {
                if (Layer::height == 1) {
                    delete (Node<T, Layer::child::width>*)node->elems[childIdx];
                } else {
                    delete (Node<void*, Layer::child::width>*)node->elems[childIdx];
                }
                node->elems[childIdx] = NULL;
                return --node->size == 0;
            }

            return false;
        }

        static T pop_push(T elem, size_t addr, size_t from, size_t count, bool goRight, Info info){
            size_t idx = (from + helper<T, Layer>::get_offset(addr, info)) % Layer::capacity;

            while (count > 0) {
                size_t doCount = min(count, goRight ? (Layer::child::capacity - (idx % Layer::child::capacity))
                : (idx % Layer::child::capacity + 1));

                auto child = get_child(addr, idx / Layer::child::capacity);


                if (doCount == Layer::child::capacity) {
                    helper<T, typename Layer::child>::set_offset(child, WRAP((helper<T, typename Layer::child>::get_offset(child, info)) + (goRight ? -1 : 1), Layer::child::capacity), info);
                    elem = helper<T, typename Layer::child>::replace(elem, child, idx, info);
                } else {
                    elem = helper<T, typename Layer::child>::pop_push(elem, child, idx, doCount, goRight, info);
                }

                idx = WRAP(idx + (goRight ? doCount : -doCount), Layer::capacity);
                count -= doCount;
            }
            return elem;
        }

        inline static T sum(size_t addr, size_t from, size_t count, Info info) {
            T s = T();
            size_t idx = (from + get_offset(addr, info)) % Layer::capacity;

            while (count > 0) {
                size_t doCount = min(count, Layer::child::capacity - (idx % Layer::child::capacity));
                auto child = get_child(addr, idx / Layer::child::capacity);
                s += helper<T, typename Layer::child>::sum(child, idx, doCount, info);
                idx = (idx + doCount) % Layer::capacity;
                count -= doCount;
            }

            return s;
        }

        static T replace(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
         //   LNODE *leaf;
          //  leaf = (LNODE *) get_child(addr, idx);
           // leaf->size++;
            return res;
        }

        static int print_helper(size_t addr, int n, Info info) {
            int x = n + 1;

#ifdef PPACK
            auto node = ((INODE*) ((Elem*) addr)->child);
#else
            auto node = (INODE*) addr;
#endif
            cout << n << " [label=\"" << node->id << " (" << node->size << "/" << get_offset(addr, info) << ")\"]" << endl;

            for (int i = 0; i < Layer::width; i++) {
                auto elem = node->elems[i];

#ifdef PPACK
                if(elem.child == 0){
#else
                if(elem == NULL){
#endif
                    cout << n << " -> "<< x << ';' << endl;
                    cout << x << " [label=\"NULL\"]" << endl;
                    x++;
                }
                else {
                    cout << n << " -> " << x << ';' << endl;

                    x = helper<T, typename Layer::child>::print_helper((size_t)&elem, x, Info{});
                }
            }

            return x;
        }

        static void randomize(size_t addr, size_t max, Info info) {
           for (int i = 0; i < Layer::width; i++) {
              if (max > i * Layer::child::capacity) {
                  helper<T, typename Layer::child>::randomize(get_child(addr, i), max - i * Layer::child::capacity , info);
              }
           }

           if (max >= Layer::capacity) {
              set_offset(addr, rand() % Layer::capacity, info);
           }
        }
    };

    template <class T, typename A, size_t W>
    struct helper<T, LayerItr<A, Layer<W, LayerEnd> > > {
        typedef LayerItr<A, Layer<W, LayerEnd> >  L;


#ifdef PACK
        static size_t get_fake_offset(size_t addr, Info info){
#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
            addr += L::parent::top_nodes;
#endif
            return info.offsets[addr];
        }
#endif

#ifdef ARRAY
        static size_t get_offset(size_t addr, Info info){
#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = L::parent::top_width + addr / L::offsets_per;
            return (info.offsets[pos] >> (L::bit_width * (addr % L::offsets_per))) & (((size_t)1 << L::bit_width) - 1);
#else
            addr += L::parent::top_nodes;
#endif
#endif
#ifdef PACK
            size_t off = info.offsets[addr];
            return off >> 48;
#else

           return info.offsets[addr];
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){

#ifdef LINE
            addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
#ifdef COMPACT
            size_t pos = L::parent::top_width + addr / L::offsets_per;
            size_t mask = (((size_t)1 << L::bit_width) - 1) << (L::bit_width * (addr % L::offsets_per));
            info.offsets[pos] = (info.offsets[pos] & ~mask) | (offset << (L::bit_width * (addr % L::offsets_per)));
            return;            
#else
            addr += L::parent::top_nodes;
#endif
#endif

#ifdef PACK
            size_t off = info.offsets[addr];
            size_t ptr = ((off << 16) >> 16);
            size_t n_offset = (offset << 48) | ptr;
            info.offsets[addr] = n_offset;
#else
            info.offsets[addr] = offset;
#endif
        }
        static T& get_elem(size_t addr, size_t idx, Info info) {
#ifdef PFREE
            return ((T*)info.elems)[addr * L::width + idx];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            //cerr << ((T*)ptr)[idx] << " GET " <<endl;
            return ((T*)ptr)[idx];
#else

            return ((T*)info.ptrs[addr])[idx];
#endif
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
#ifdef PFREE
            return addr;
#elif defined(PACK)
            size_t offset = get_fake_offset(addr, info);
            if(offset << 16 == 0) {
                size_t arr_addr = (size_t) new T[L::width];
                assert((arr_addr >> 48) == 0);

                size_t n_offset = (offset << 48) | arr_addr;

#ifdef LINE
                addr = addr + L::parent::parent::top_nodes;
#elif defined(LEVEL)
                addr += L::parent::top_nodes;
#endif
                info.offsets[addr] = n_offset;
            }
#else
            if (info.ptrs[addr] == NULL) {
                info.ptrs[addr] = new T[L::width];
            }
#endif
            return addr;
        }
#else
        static size_t get_offset(size_t addr, Info info){
#ifdef PPACK
            return ((Elem*) addr)->offset;
#else
            return ((INODE*) addr)->offset;
#endif
        }
        static void set_offset(size_t addr, size_t offset, Info info){
#ifdef PPACK
            ((Elem*) addr)->offset = offset;
#else
            ((INODE*) addr)->offset = offset;
#endif
        }
        static T& get_elem(size_t addr, size_t idx, Info info) {
#ifdef PPACK
            auto elem = (Elem*) addr;
            auto child = (LNODE*)elem->child;
            return child->elems[idx];
#else
            return ((LNODE *)addr)->elems[idx];
#endif
        }
        static size_t get_leaf(size_t addr, size_t idx, Info info) {
            return addr;
        }
        static size_t make_room(size_t addr, size_t idx, Info info) {
            return addr;
        }

#endif
        //elem is the element to insert, addr is root, from is the idx to insert, count is ??
        static T pop_push(T elem, size_t addr, size_t from, size_t count, bool goRight, Info info) {

            T res;

#ifdef ARRAY
#ifdef PFREE
            auto elems = &((T*)info.elems)[addr*L::width];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            auto elems =  (T*)ptr;
#else
            auto elems = (T*)info.ptrs[addr];
#endif
#else
#ifdef PPACK
            auto elems = ((LNODE*) ((Elem*)addr)->child)->elems;
#else
            auto elems = ((LNODE*)addr)->elems;
#endif
#endif
            if (goRight) {
                res = elems[(from + get_offset(addr, info) + count - 1) % L::capacity];

                size_t start = (from + get_offset(addr, info)) % L::capacity;
                size_t beforeWrap = min(L::capacity - start - 1, count - 1);

                // Move last part
                if (beforeWrap < count - 1) {
                    memmove(&elems[1], &elems[0], (count - beforeWrap - 2) * sizeof(T));
                    elems[0] = elems[L::capacity - 1];
                }

                // Move first part
                memmove(&elems[start + 1], &elems[start], beforeWrap * sizeof(T));
            } else {
                res = elems[(from + get_offset(addr, info) - count + 1) % L::capacity];

                size_t start = (from + get_offset(addr, info)) % L::capacity;
                size_t beforeWrap = min(start, count - 1);

                if (beforeWrap < count - 1) {
                    size_t afterWrap = count - beforeWrap - 2;
                    memmove(&elems[L::capacity - afterWrap - 1], &elems[L::capacity - afterWrap], afterWrap * sizeof(T));
                    elems[L::capacity - 1] = elems[0];
                }

                memmove(&elems[start - beforeWrap], &elems[start - beforeWrap + 1], beforeWrap * sizeof(T));
            }

            elems[(from + get_offset(addr, info)) % L::capacity] = elem;

            return res;
        }
        static T& get(size_t addr, size_t idx, Info info) {
            idx = (idx + helper<T, L>::get_offset(addr, info)) % L::capacity;
            return get_elem(addr, idx, info);
        }
        static int* drawString(size_t addr, size_t idx, Info info, size_t size) {
            int * ans = new int[size];
            idx = (idx + helper<T, L>::get_offset(addr, info)) % L::capacity;
            LNODE * node = (LNODE*)addr;
            for (int j = 0; j < node->size; ++j) {
                size_t cur = node->elems[idx];
                ans[j] = cur;
                idx = (idx+1) % L::capacity;
            }
            return ans;
        }
        static void drawTree(size_t idx, list<size_t>&lst, int & rank, int & exp) {
            size_t cur = lst.front();
            lst.pop_front();
            INODE * node = (INODE*) cur;
            if (node == NULL) {
                printf("EMPTY | ");
                return;
            }
            int offset = (idx + get_offset(cur, Info{})) % L::capacity;
            printf("[%d, %d, %d]", offset, node->size, node->id);
            for (int i = 0; i < L::width; ++i) {
                printf(" %d", ((LNODE *)cur)->elems[i]);
            }
            printf(" | ");
        }
        inline static T sum(size_t addr, size_t from, size_t count, Info info) {
            T s = T();

#ifdef ARRAY
#ifdef PFREE
            auto elems = &((T*)info.elems)[addr*L::width];
#elif defined(PACK)
            size_t off = get_fake_offset(addr, info);
            size_t ptr = ((off << 16) >> 16);
            auto elems =  (T*)ptr;
#else
            auto elems = (T*)info.ptrs[addr];
#endif
#else
#ifdef PPACK
            auto elems = ((LNODE*) ((Elem*)addr)->child)->elems;
#else
            auto elems = ((LNODE*)addr)->elems;
#endif
#endif
 

            from = (from + get_offset(addr, info)) % L::capacity;

            if (from + count < L::capacity) {
                for (size_t i = 0; i < count; i++)
                    s += elems[from + i];
            } else {
                size_t firstCount = L::capacity - from;
                size_t secondCount = count - firstCount;

                for (size_t i = 0; i < firstCount; i++)
                    s += elems[from + i];

                for (size_t i = 0; i < secondCount; i++)
                    s += elems[i];
            }

            return s;
        }


        static bool remove_room(INODE *node, size_t idx) {
            return --node->size == 0;
        }


        static T replace(T elem, size_t addr, size_t idx, Info info) {
            T& t = get(addr, idx, info);
            T res = t;
            t = elem;
            return res;
        }

        static int print_helper(size_t addr, int n, Info info) {
            int x = n + 1;

            LNODE * leaf = (LNODE *) addr;
            cout << n << " [label=\"" << leaf->id << " (" << leaf->size << "/" << get_offset(addr, info) << ")\"]" << endl;

            for (int i = 0; i < L::width; i++) {
                auto elem = leaf->elems[i];
                cout << n << " -> " << x << endl;
                cout << x << " [label=\"" << elem << "\"];" << endl;
                x++;
            }

            return x;
        }

        static void randomize(size_t addr, size_t max, Info info) {
           if (max >= L::width) {
               set_offset(addr, rand() % L::width, info);
           }
        }
    };

#ifdef ARRAY
    TT
        Tiered<T, Layer>::Tiered() {
#ifdef PACK

#else
            info.ptrs = new void*[Layer::nodes];
            memset(info.ptrs, 0, sizeof(void*)*Layer::nodes);
#endif

#ifdef PFREE
            info.elems = new T[Layer::capacity];
#endif
            info.offsets = new size_t[Layer::nodes];
            memset(info.offsets, 0, sizeof(size_t)*Layer::nodes);
        }
#else
    TT
        Tiered<T, Layer>::Tiered() {

#ifdef PPACK
            relem = {0, (size_t) new Node<Elem, Layer::width>(0)};
#else
            root = (size_t ) new Node<void*, Layer::width>(0);
#endif
        }


#endif


    template<class T, size_t width>
        Node<T, width>::Node(size_t depth) : depth(depth) {
            memset(elems, 0, sizeof(T)*width);
            id = ID;
            ID++;
        }

    TT
        T Tiered<T, Layer>::sum(size_t from, size_t count){
            return helper<T, Layer>::sum((size_t)root, from, count, info);
        }

    TT
        void Tiered<T, Layer>::insert(size_t idx, T elem){

            assert((size < Layer::capacity));
            assert (idx <= size);
            if (idx >= size/2) {
                elem = helper<T, Layer>::pop_push(elem, (size_t)root, idx, size - idx, true, info);
                helper<T, Layer>::make_room(root, size, info);
                helper<T, Layer>::replace(elem, (size_t)root, size, info);
            } else {
                elem = helper<T, Layer>::pop_push(elem, (size_t)root, WRAP(idx - 1, Layer::capacity), idx, false, info);
                helper<T, Layer>::set_offset(root, WRAP((helper<T, Layer>::get_offset(root, info) - 1), Layer::capacity), info);
                helper<T, Layer>::make_room(root, 0, info);
                helper<T, Layer>::replace(elem, (size_t)root, 0, info);
            }

            size++;
        }

    TT
        void Tiered<T, Layer>::insert_sorted(T elem){
            size_t left = 0, right = size;

            while (left < right) {
                size_t mid = (left + right) / 2;
                if (elem < (*this)[mid]) {
                    right = mid;
                } else {
                    left = mid + 1;
                }
            }

            insert(left, elem);
        }

    TT
        const T& Tiered<T, Layer>::operator[](size_t idx) const{
            assert (idx < size);

            return helper<T, Layer>::get((size_t)root, idx, info);
        }

    TT
        size_t Tiered<T, Layer>::successor(T elem){
            size_t left = 0, right = size;

            while (left < right) {
                size_t mid = (left + right) / 2;
                if (elem < (*this)[mid]) {
                    right = mid;
                } else {
                    left = mid + 1;
                }
            }

            return left;
        }

    TT
        void Tiered<T, Layer>::fill(T *res){
            fill(res, root, 0, size);
        }


    TT
        void Tiered<T, Layer>::remove(size_t idx) {
            if (idx >= size/2) {
                size--;
                T garbage = {};
                helper<T, Layer>::pop_push(garbage, root, size, size - idx, false, info);
            } else {
                T garbage = {};
                helper<T, Layer>::pop_push(garbage, root, 0, idx + 1, true, info);
                size--;

                helper<T, Layer>::set_offset(root, WRAP((helper<T, Layer>::get_offset(root, info)) + 1, Layer::capacity), info);
            }
        }

    TT
        void Tiered<T, Layer>::print(){
            cout << "digraph G {" << endl;
            helper<T, Layer>::print_helper(root, 0, Info{});
            cout << "}" << endl;
        }

    TT
        void Tiered<T, Layer>::randomize() {
            helper<T, Layer>::randomize(root, size, info);
        }

    template<class T, class Layer>
    int Tiered<T, Layer>::print_helper(FakeNode<void *> *node, int n) {
        return helper<T, typename Layer::child>::print_helper((size_t)node, n, 1);
    }
    TT
    void Tiered<T, Layer>::drawString(){
        //assert (idx < size);
        int * s = helper<T, Layer>::drawString((size_t)root, 0, info, size);
        for (int i = 0; i < size; ++i) {
            cout<<s[i]<<" ";
        }
        cout<<endl;
    }
    TT
    void Tiered<T, Layer>::drawTree(){
        list<size_t> lst;
        lst.push_back((size_t)root);
        int rank = 0, exp = 0;
        helper<T, Layer>::drawTree(0, lst, rank, exp);
        cout<<endl;
    }
}

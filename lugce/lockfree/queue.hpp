/*	This library is free software; you can redistribute it and/or
*	modify it under the terms of the GNU Lesser General Public
*	License as published by the Free Software Foundation; either
*	version 2.1 of the License, or (at your option) any later version.
*
*	This library is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*	Lesser General Public License for more details.
*
*	You should have received a copy of the GNU Lesser General Public
*	License along with this library; if not, write to the Free Software
*	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*****************************************************************************
*	A beg: Please tell me if you are using this library.
*	Author: Chen Wang ( China )
*	Email: jadedrip@gmail.com
*/

#pragma once

#include "atomic.hpp"
#include "memory_pool.hpp"

namespace lugce
{
	namespace lockfree
	{
		template< typename T, typename ALLOC=lugce::lockfree::memory_pool<T> >
		class queue
		{
		private:
			struct reader_queue_node
			{
				reader_queue_node *next;
				void *pointer;
				bool is_end;
			};

			// 把对象大小规格化为 32 或 64 位，以保证分配的内存可以对齐
			static const int object_size=(sizeof(T) % sizeof(intptr_t) == 0) ? sizeof(T) : sizeof(T) + sizeof(intptr_t) - sizeof(T) % sizeof(intptr_t);
			struct data
			{
				atomic_pointer<data> next;
				atomic_int32 ref;	// 引用计数
				union{
					T value;
					char value_data[ object_size ];	
				};
			};

			typedef typename ALLOC::rebind<data>::other MyAlloc;
		public:
			queue()
			{
				_head=nullptr;
				_end=_begin=_alloc.allocate();
				_end->ref=0;
				_end->next=nullptr;
			}
			~queue()
			{
			}

			void push_back( const T& obj )
			{
				data *n=_alloc.allocate();
				n->next=nullptr;
				n->ref=2;
				n->value=obj;

				data* old_end;
				for(;;){
					old_end=_end;
					if( old_end->next ){	// 如果取到的不是尾指针
						boost::this_thread::yield();
						continue;
					}
				} while ( !old_end->next.compare_and_set( 0, n ) );

				// 尝试更新 _end
				while( !_end.compare_and_set( old_end, n ) ){
					if( n->next==nullptr ) break;
					old_end=_end;
				}

				if( (--n->ref)==0 ) _alloc.deallocate(n);
			}

			bool pop( T& v )
			{
				data* old_head;
				for(;;){
					old_head=_head;
					if( old_head->next ){	// 存在下一个节点
						if( _head.compare_and_set(old_head, old_head->next) )
							break;
					}else{
						int r=old_head->ref;
						if(r==0) return false;
						if( !old_head->ref.compare_and_set( r, r-1 ) ) continue;
						v
					}
				}
			}
		private:
			atomic_pointer<data>	_head;	// 头指针
			atomic_pointer<data>	_end;	// 尾指针
			data* _begin;

			atomic_int32 _push_mark;
			atomic_int32 _pop_mark;

			MyAlloc _alloc;
		};
	};
};
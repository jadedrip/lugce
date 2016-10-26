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
		class stack
		{
		private:
			struct data
			{
				data* next;	// Next Node, nullptr for end.
				T value;
			};
			typedef typename ALLOC::rebind<data>::other MyAlloc;
		public:
			stack<T,ALLOC>() : _push_mark(0), _pop_mark(0)
			{
			}

			void push( const T t )
			{
				data* n=_alloc.allocate();
				n->value=t;

				int32 idx=_push_mark++ & 0x7; //% barrel;
				atomic_pointer<data> &head=_head[idx];
				do{
					n->next=head;
				}while( !head.compare_and_set( n->next, n ) );
			}

			bool pop( T& v )
			{
				int32 idx=_pop_mark++ & 0x7;// % barrel;
				atomic_pointer<data> &head=_head[idx];
				data * old;
				do{
					old=head;
					if( old == 0 ) return false;	// 没有数据了
				}while( !head.compare_and_set( old, old->next ) );

				v=old->value;
				_alloc.deallocate( old );
				return true;
			}
		public:
			std::shared_ptr<T> pop()
			{
				int32 idx=_pop_mark++ & 0x7;// % barrel;
				atomic_pointer<data> &head=_head[idx];
				data * old;
				do{
					old=head;
					if( old == 0 ) return std::shared_ptr<T>();	// 没有数据了
				}while( !head.compare_and_set( old, old->next ) );

				return std::shared_ptr<T>( &old->value, destorier( _alloc, old ) );
			}
		private:
			struct destorier
			{
				destorier( MyAlloc& alloc, data* ptr ) : _alloc(alloc), _ptr(ptr){}
				MyAlloc& _alloc;
				data * _ptr;
				void operator()( T* ){ _alloc.deallocate(_ptr); }
			};
		private:
			atomic_pointer<data> _head[8];
			atomic_int32 _push_mark;
			atomic_int32 _pop_mark;
			MyAlloc _alloc;
		};
	};
};
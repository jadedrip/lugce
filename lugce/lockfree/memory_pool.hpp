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
#include <exception>
#include "../../libs/shared_ptr.hpp"
#include "atomic.hpp"

/// C++ 库 lugce 的名字空间
namespace lugce
{
	namespace lockfree
	{
		namespace details
		{	/// \private
			class memory_pool_base
			{
				static const int barrel=16;
			public:
				memory_pool_base( const int osize, const size_t bsize )
					: _objsize(osize), _alloc_mark(0),_dealloc_mark(7)
				{
					_blocksize=(bsize/barrel);
					if( _blocksize < 256 ) _blocksize=256;
					for( int i=0; i<barrel; ++i ){
						_first_block[i]=tadem_block(i);
						_free_head[i]=reinterpret_cast<intptr_t>(_first_block[i])+sizeof(intptr_t)+_objsize;	// 指向链表头
					}
				}
			
				~memory_pool_base()
				{
					// 释放内存块
					for( int i=0; i<barrel; ++i ){
						char * next=_first_block[i];
						do{
							char *p=next;
							intptr_t x=*(intptr_t*)p;
							next=(char*)x;
							delete[] p;
						}while(next);
					}
				}
			protected:
				/// \~chinese	创建新的内存块
				/// \~english	Create a new block of the memory.
				void create_new_block( int idx )
				{
					atomic_intptr& head=_free_head[idx];
					char *block=tadem_block( idx );	// 分配内存
					atomic_intptr_t *p=(atomic_intptr_t*)(_first_block[idx]);

					// 尝试挂接到内存块链表
					while( p->data )	// 找到链表尾
						p=(atomic_intptr_t*)(p->data);	// 移动到链表下一位

					while( !atomic_compare_and_set( *p, 0, intptr_t(block) ) ){
						p=(atomic_intptr_t*)(p->data);	// 移动到链表下一位
					}

					p=(atomic_intptr_t*)( block+sizeof(intptr_t) );	// 让 p 指向链表尾部
					// 尝试挂接到空闲内存栈头上
					intptr_t old;
					intptr_t nval;
					
					do{
						old=head.data;
						p->data=old;		// 让链表尾指向当前尾	
						intptr_t x=*(intptr_t*)(p->data);
						nval=(intptr_t)(block+sizeof(intptr_t)+_objsize);	// 新的下块空闲指向本块
					} while( !head.compare_and_set( old, nval ) );
				}

				/// \~chinese	分配内存，并将内存串联为链表
				char* tadem_block( const int idx )
				{
					char *block=new char[ _blocksize * _objsize+sizeof(intptr_t)];	// 准备一块内存，注意 new 可能抛出异常
					char *p=block;
					*reinterpret_cast<intptr_t*>(p)=0;	// 内存的头是对齐的，我们用来保存下一块内存的地址，以构建内存块链表（用来内存池析构时释放内存块）
					p+=sizeof(intptr_t);
					*reinterpret_cast<intptr_t*>(p)=0;	// 接下来的字节，同样是对齐的，作为链表的尾部
					p+=_objsize;
					// 把这块内存做成链表
					for( uint32 i=0; i<_blocksize-2; ++i ){
						*reinterpret_cast<intptr_t*>(p)=reinterpret_cast<intptr_t>(p)+_objsize;	// 内容成为指向下一块的空闲单元的指针
						p+=_objsize;
					}
					// 最后的指针指向上面定义的尾部
					*reinterpret_cast<intptr_t*>(p)=reinterpret_cast<intptr_t>(block)+sizeof(intptr_t);		// 最后一块指向尾节点
					return block;
				}
			public:
				/// \~english	Allocate a block of memory for one element.
				/// \~chinese	申请可以存放一个元素的内存。
				/// \return 	一个指向新内存的指针。
				/// \note		同 STL 标准不同，如果内存分配失败，不会返回空指针，而将由 new 抛出 std::bad_alloc 方法。
				void* allocate()
				{
					// 尝试从堆栈中弹出一个空闲索引
					intptr_t nval;
					intptr_t old;
					
					const int idx= _alloc_mark++ & 0xF; //barrel; 
					//atomic_increment( g_alloc_count + idx );
					atomic_intptr& head=_free_head[idx];
					do{
						old=head.data;
						intptr_t *next=(intptr_t*)old;	// 指向下一块空闲单位的指针
						nval=*next;
						if( 0==nval ){	// 没有空闲，需要创建新块
							// 创建新块
							create_new_block( idx );
							// goto label;
							continue;
						}
					}while( !head.compare_and_set( old, nval ) );
					return (void*)old;
				}

				/// \~english
				/// \brief	Free the memory of the object.
				/// \~chinese	
				/// \brief	释放对象内存
				void deallocate( void* ptr )
				{
					intptr_t *p=(intptr_t*)ptr;
					intptr_t nval;
					intptr_t old;
					// 尝试将其放回链表
					const int idx= _dealloc_mark++ & 0xF;// barrel; 
					//atomic_increment( g_dealloc_count + idx );
					atomic_intptr& head=_free_head[idx];
					do{
						old=head.data;
						*p=old;	// 把内容改为下一个空闲索引
						nval=(intptr_t)ptr;
					}while( !head.compare_and_set( old, nval) );
				}
			protected:
				atomic_intptr _free_head[barrel];	// 下一个空闲块的指针
				atomic_int32 _alloc_mark;		
				atomic_int32 _dealloc_mark;
				uint32 _blocksize;					// 每块内存的可以存放的对象数
				char * _first_block[barrel];		// 首块内存
				const int _objsize;					// 对象的大小
			};
		}

		/// \~english 
		/// \brief	Lock-free memory pool
		/// \~chinese 
		/// \brief		高效的无锁内存池
		/// \remarks	比直接使用 new 的效率高100倍以上。
		template< typename T >
		class memory_pool 
		{
		private:
			std::shared_ptr< details::memory_pool_base > _pool;
		public:	// Typedef
			typedef T			value_type;
			typedef T*			pointer;
			typedef const T*	const_pointer;
			typedef T&			reference;
			typedef const T&	const_reference;
			typedef size_t		size_type;
			typedef ptrdiff_t	difference_type;

			/// \private
			/// \~english 
			/// \brief	A structure that enables an allocator for objects of one type to allocate storage for objects of another type.
			/// \~chinese 
			/// \brief	使对象分配器能够分配另一种类型的分配器的结构。
			template<class _Other>
			struct rebind
			{	
				typedef memory_pool<_Other> other;
			};
		public:	//
			void show_count( ){ _pool->show_count(); }
			/// \~chinese 
			/// \brief	构造函数
			/// \param bsize 每次分配的内存块可以分配多少个对象。
			memory_pool( const size_t bsize=1024 * 16 ) 
				: _pool( new details::memory_pool_base(
					sizeof(T) < sizeof(intptr_t) ? sizeof(intptr_t) : sizeof(T),
					(bsize < 8 ? 8 : bsize) 
					) )
			{
				
			}
		public:	// Interface for STL	// STL 接口
			/// \~english 
			/// \brief	Finds the address of an object whose value is specified.
			/// \~chinese
			/// \brief	获取指定对象的地址。
			pointer address( reference _Val	) const{ return &_Val; }
			/// \~english 
			/// \brief	Finds the address of an object whose value is specified.
			/// \~chinese
			/// \brief	获取指定对象的地址。
			const_pointer address( const_reference _Val	) const{ return &_Val; }

			/// \~english
			/// Allocates a block of memory large enough to store at least some specified number of elements.
			/// \param count The number of elements for which sufficient storage is to be allocated.
			/// \param _hint <i>the parameter is useless.</i>
			/// \~chinese
			/// 分配一个足够存放指定数目对象的内存。
			/// \param count 足够存放指定数量元素的内存将被分配。
			/// \param _hint <i>参数是无用的。</i>
			/// \note	这仅仅是为了兼容 STL 而提供的方法，只有传入的 count 为1时才会调用无参数的allocte方法通过内存池
			///			分配内存，count不为1时将使用 new 来分配内存，为了效率考虑，请尽量直接调用无参数的 allocte 方法。
			///			由于 vector、deque 等容器分配内存时并不是一个一个分配的，因此不适合使用这个内存池。
			/// \sa allocate()
			pointer allocate( size_type count=1, const void* _hint=NULL )
			{
				assert( count==1 );
				if( 1==count )
					return pointer( _pool->allocate() );
				else{
					return new T[count];
				}
			}

			/// \~english
			/// Frees a specified number of objects from storage beginning at a specified position.
			/// \~chinese
			/// 释放在指定位置的多个的对象。
			/// \param ptr		要被释放的对象的首指针。
			/// \param count	要释放的对象数量 。
			/// \note deallocate 必须和 allocate 配对使用，count 的数量必须和分配时的数量相同。
			/// \sa deallocate(pointer)
			void deallocate( pointer ptr, size_type count=1 )
			{
				assert( count==1 );
				if( count==1 )
					_pool->deallocate( ptr );
				else
					delete[] ptr;
			}

			/// \~english
			/// \brief	Constructs a specific type of object at a specified address that is initialized with a specified value.
			/// \~chinese
			/// \brief	在指定位置用指定值构造对象。
			void construct( pointer _Ptr, const value_type& _Val )
			{
				new (_Ptr) T( _Val );
			}

			/// \~chinese
			/// \brief	在指定位置用指定值构造对象，右值引用版本。
			template<class Types>
			void construct(	pointer ptr, Types &&val )
			{
				new (ptr) T( std::forward<value_type>( val ) );
			}

			/// \~english
			/// \brief	Calls an objects destructor without deallocating the memory where the object was stored.
			/// \~chinese	
			/// \brief	调用对象的析构函数而不释放内存。
			void destroy( pointer ptr )
			{
				ptr->~value_type();
			}

			/// \~english	
			/// \brief	Returns the number of elements of type Type that could be allocated by an object of 
			///			class allocator before the free memory is used up. 
			/// \~chinese	
			/// \brief	返回可以分配的对象最大数量。
			size_type max_size( ) const
			{	// estimate maximum array size
				size_type _Count = (size_type)(-1)/sizeof(value_type);
				return (0 < _Count ? _Count : 1);
			}
		};
	};
};
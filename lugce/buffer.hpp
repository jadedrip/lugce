/* 
* Copyright (C) 2010  
* author:	Chen Wang ( China )
* Email:	jadedrip@gmail.com
* ============================================
*		* GNU Lesser General Public License *
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* This library is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with this library; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
* =============================================
* Copyright (c) 2010，作者保留所有权利。( All rights reserved. )
* 文件名称：readable_data.hpp
* 摘 要：这里提供一个 buffer 类，可以用来管理一块内存做缓存使用
* 
* 当前版本：1.0
* 作 者：王琛
* 完成日期：2012年4月18日
* 平台测试：Visual Studio 2010，gcc 4.3.3
* =============================================
*/
#pragma once
#include <stdint.h>
#include <string>
#include <ostream>

namespace lugce
{
	/// 管理一块内存做缓存使用
	/// 本类兼容STL输入输出缓冲。出于效率考虑，本类是非线程安全的。
	/// 注意，对象复制后指向同一块内存，本类基于简单引用计数来实现内存的释放。
	class buffer 
	{
	public:
		typedef const uint8_t * const_reference;
		typedef uint8_t value_type;
	public:
		enum { block_size = 1024 };
		static const size_t npos = (size_t)-1;

		buffer()  { _block = 0; _size = 0; _data = NULL; }

		buffer( const buffer& v ) : _data(v._data), _size(v._size), _block(v._block)
		{
			if( _data )	++*reinterpret_cast<size_t*>(_data);
		}

#ifdef _HAS_CPP0X
		/// Move constructor
		buffer( buffer&& v ) : _data(v._data), _size(v._size), _block(v._block)
		{
			v._data=nullptr;
		}
#endif

		buffer(const void* v, size_t sz) : _data(NULL), _size(0), _block(0)
		{
			assign( v, sz );
		}

		virtual ~buffer()
		{
			if( _data ){
				size_t *p=(size_t*)_data;
				if(--*p==0)	free( _data );
			}
		}
	public:
		/// 返回引用计数
		size_t ref_count() const
		{
			return _data ? *reinterpret_cast<size_t*>(_data) : 0;
		}

		void assign( const void* data, size_t sz )
		{
			if(sz==0)
				clear();
			else{
				resize(sz);
				memcpy(_data+sizeof(size_t), data, sz);
			}
		}

		void assign( const std::string& v ){ assign(v.data(),v.size()); }

		inline bool   empty() const     { return _size == 0; }
		/// 清空数据
		void clear(){ _size=0; }

		uint8_t* data(){ return _data+sizeof(size_t); }
		inline uint8_t * data() const      { return _data+sizeof(size_t); }
		inline size_t size() const      { return _size; }
		const char * c_str()
		{
			reserve( _size+1 );
			_data[_size+sizeof(size_t)]=0;
			return (const char*)data();
		}

		/// 重新设定大小
		void resize(size_t n, uint8_t c)
		{
			reserve(n);
			if( n > _size )	memset( data()+_size, c, n-_size );
			_size=n;
		}

		/// 重新设定大小
		void resize(size_t n)
		{
			reserve(n);
			_size=n;
		}

		/// 预分配内存
		void reserve(size_t n)
		{
			n+=sizeof(size_t);
			if( _data ){
				if( n <= _block*block_size )	return; // 如果内存已经足够
				size_t nb=n / block_size + ( (n % block_size) ? 1 : 0 ); // 计算新块
				uint8_t *p=(uint8_t*)realloc( _data, block_size * nb );
				if( !p ) throw std::bad_alloc();
				_data=p;
				_block=nb;
			}else{
				_block=n / block_size + ( (n % block_size) ? 1 : 0 ); // 计算新块
				_data=(uint8_t*)malloc( block_size * _block );
				_size=0;
				*reinterpret_cast<size_t*>(_data)=1;
			}
		}

		/// 返回预分配的内存量
		size_t reserved() const{ return _block*block_size;  }

		/// 在结尾放入一个字符
		void push_back( uint8_t c )
		{
			reserve(1+_size);
			_data[ sizeof(size_t)+_size++ ]=c;
		}

		void append(const void *app, size_t len)
		{
			reserve(len+_size);
			memcpy( data()+_size, app, len );
			_size+=len;
		}

		void add(const uint8_t c, size_t len)
		{
			reserve(len+_size);
			memset( data()+_size, c, len );
			_size+=len;
		}

		void replace(size_t pos, const void *rep, size_t n)
		{
			reserve( pos+n );
			memcpy( data()+pos, rep, n );
			if( pos+n > _size ) _size=pos+n;
		}

		void erase(size_t pos = 0, size_t n = npos )
		{
			if( pos >= _size ) return;
			if( n==npos || (pos+n>=_size) )
				_size=pos;
			else{
				_size-=pos+n;
				memcpy( data()+pos, _data+pos+n, _size );
			}
		}

		void swap( buffer& bb )
		{
			std::swap( _data, bb._data );
			std::swap( _size, bb._size );
			std::swap( _block, bb._block );
		}

		uint8_t& operator[]( const size_t idx )
		{
			assert( idx < _size );
			return data()[idx];
		}

		const uint8_t& operator[]( const size_t idx ) const
		{
			assert( idx < _size );
			return data()[idx];
		}
	public:
		friend std::ostream& operator << ( std::ostream& os, const buffer& bu )
		{
			return os.write( (const char*)bu.data(), bu.size() );
		}
	private:
		uint8_t  *_data;
		size_t _size;	// 有效数据
		size_t _block;	// 预分配的数据块数
	};
};

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
* 摘 要：16进制的数据和字符串之间的相互转换
* 
* 当前版本：1.0
* 作 者：王琛
* 完成日期：2009年6月22日
* 平台测试：Visual Studio 2010，gcc 4.3.3
* =============================================
*/

#pragma once
#include <string>
#ifdef _WIN32
#	include <type_traits>
#endif

namespace lugce
{
	namespace detail		
	{	
		/// \private 
		// char to data
		inline  char charx( const wchar_t c )
		{
			if( c>='0' && c<='9' )	
				return (c-'0');
			else if(c>='a' && c<='f') 	
				return (c-'a') + 10;
			else if(c>='A' && c<='F') 
				return (c-'A') + 10;
				
			return -1;
		}

		/// \private 
		// data to char
		inline char xchar( const char c, bool upper )
		{
			if( upper )
				return (c >= 10) ? ('A'-10 + c) : ('0' + c);
			return (c >= 10) ? ('a'-10 + c) : ('0' + c);
		}

		/// \private
		template< typename Iterator, int Tsize >
		struct __DataToStringHelper
		{
			template< typename OutIter >
			static void conv( const Iterator begin, const Iterator end, OutIter out, bool upper )
			{
				for( Iterator i=begin; i!=end; i++ ){
					for( int x=Tsize-1; x>=0; --x ){
						*out=detail::xchar( *i >> ( x * 8 + 4 ) & 0xF, upper );
						++out;
						*out=detail::xchar( *i >> ( x * 8 ) & 0xF, upper );
						++out;
					}
				}
			}
		};

		/// \private
		template< typename T >
		struct __StringToDataHelper
		{
			template< typename InIter, typename Iterator >
			static size_t conv( const InIter begin, const InIter end, Iterator iter )
			{
				size_t count=0;
				InIter i=begin;
				unsigned int wcnt=0;
				T o=T();
				while( i!=end ){
					char a=charx(*i++);	
					if( a!=-1 ){
						o = o << 4 | a;
						if( ++wcnt < (sizeof(T) * 2) )		continue;
					}else if ( wcnt==0 ){
						continue;
					}

					*iter++=o;
					count++;
					wcnt=0; o=T();
				}	// while
				if( wcnt>0 )
					throw std::bad_cast();
				return count;
			}
		};	// end of struct __StringToDataHelper

		/// 从迭代器推导类型
		template< typename _Iter >
		struct iterator_type
		{
#ifdef _MSC_VER
			// visual studio 2008 的 back_inserter 之类的迭代器没有正确定义 value_type, 因此这里使用 reference
			typedef typename std::remove_const<typename std::remove_reference<typename _Iter::const_reference>::type>::type type;
#else
			// gcc 的迭代器 reference 和 value_type 都有问题
			typedef typename _Iter::container_type::value_type type;
#endif
		};

		template< typename T >
		struct iterator_type<T*>
		{
			typedef T type;
		};

	}

	/// 把内存数据转变成16进制文本的字符串以供阅读。
	/// \param begin	数据开始的迭代器
	/// \param end		数据结束的迭代器
	/// \return	结果字符串
	template< typename Iterator >
	inline std::string data_to_string( const Iterator begin, const Iterator end, bool upper=false )
	{
		std::string s;
		detail::__DataToStringHelper<Iterator, sizeof(*begin)>::conv( begin, end, std::back_inserter(s), upper );
		return s;
	}

	/// 把内存数据转变成16进制文本的宽字符串以供阅读。
	/// \param begin	数据开始的迭代器
	/// \param end		数据结束的迭代器
	/// \return	结果字符串
	template< typename Iterator >
	inline std::wstring data_to_wstring( const Iterator begin, const Iterator end, bool upper=false )
	{
		std::wstring s;
		detail::__DataToStringHelper<Iterator, sizeof(*begin)>::conv( begin, end, std::back_inserter(s), upper );
		return s;
	}

	template< typename Iterator, typename OutIterator >
	inline void data_to( const Iterator begin, const Iterator end, OutIterator out, bool upper=false )
	{
		detail::__DataToStringHelper<Iterator, sizeof(*begin)>::conv( begin, end, out, upper );
	}

	template< typename Iterator, typename OutIterator >
	inline void data_to( const Iterator begin, const size_t s, OutIterator out, bool upper=false )
	{
		detail::__DataToStringHelper<Iterator, sizeof(*begin)>::conv( begin, begin+s, out, upper );
	}

	/// 将一串16进制字符串转换为数据, 返回数据的长度
	/// \note 可以使用 back_insert_iterator 之类的迭代器将数据插入容器，或者直接使用容器的迭代器、指针。
	/// 如果使用容器的迭代器或指针，要注意避免溢出。
	/// \param	source			输入字符串
	/// \param 	out_iterator	输出的迭代器
	/// \return	结果数据的数量
	template< 
		typename Iterator
	>
	inline size_t readable_to_data( const std::string& source, Iterator out_iterator ) 
	{
		using namespace detail;
		return __StringToDataHelper<typename iterator_type<Iterator>::type>::conv( source.begin(), source.end(), out_iterator );
	}

	/// 将一串16进制宽字符串转换为数据, 返回数据的长度
	/// \note 可以使用 back_insert_iterator 之类的迭代器将数据插入容器，或者直接使用容器的迭代器。
	/// 如果使用容器的迭代器，要注意避免溢出。
	/// \param	source			输入字符串
	/// \param 	out_iterator	输出的迭代器
	/// \return	结果数据的数量
	template< 
		typename Iterator
	>
	inline size_t readable_to_data( const std::wstring& source, Iterator out_iterator )
	{
		using namespace detail;
		return __StringToDataHelper<typename iterator_type<Iterator>::type>::conv( source.begin(), source.end(), out_iterator );
	}

	template< 
		typename InIter,
		typename Iterator
	>
	inline size_t readable_to_data( const InIter begin, const InIter end, Iterator out_iterator )
	{
		using namespace detail;
		return __StringToDataHelper<typename iterator_type<Iterator>::type>::conv( begin, end, out_iterator );
	}
	template< 
		typename InIter,
		typename Iterator
	>
	inline size_t readable_to_data( const InIter begin, const size_t sz, Iterator out_iterator )
	{
		using namespace detail;
		return __StringToDataHelper<typename iterator_type<Iterator>::type>::conv( begin, begin+sz, out_iterator );
	}
};

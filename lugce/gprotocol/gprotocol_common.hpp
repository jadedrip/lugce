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
*/
#pragma once
#include <stdint.h>
#include <assert.h>
#include <string>
#include <sstream>
#include <iterator>
#include <utility>
#include "../type_traits_ex.hpp"

namespace lugce
{
	namespace gprotocol
	{
		enum option { unpacked=1, fixed=2 };

		template< class Archive, typename T >
		inline void serialize( Archive& archive, T& v )
		{
			v.serialize( archive );
		}

		inline uint32_t wire_key( const uint8_t type, const uint32_t index )
		{
			assert( (index < 0x1FFFFFFF) && (type < 7) );
			return (index << 3) | (type & 0x7);
		}

		/// for optional 
		template< typename T >
		class optional : public std::pair< bool, T >
		{
		public:
			optional(){ this->first=false; }
			optional( const T& v ){ this->first=true; this->second=v; }
		public:
			optional& operator = ( const T& v ){ set(v); return *this; }
			operator bool () const{ return this->first; }
			operator const T () const{ return this->second; }
			bool empty() const{ return !this->first; }
			void set( const T& v ){ this->first=true; this->second=v; }
			const T get() const{ return this->second; }
			const T get_by_default( const T& v ){ return this->first ? this->second : v; }
			void clear(){ this->first=false; }
		};

		template< typename T >
		struct is_optional : public std::false_type{};

		template< typename T >
		struct is_optional< std::pair< bool, T > > : public std::true_type{};

		template< typename T >
		struct is_optional< optional<T> > : public std::true_type{};

		/// code uint32, uint64 or sint32, sint64, enum
		template< class T >
		struct varint_coder 
		{
			enum { wire=0 };

			static void save( std::ostream& os, const T& v, int option=0 )
			{
				if( !std::is_unsigned<T>::value ){ 
					// Signed Integers
					int s=8 * sizeof(T) - 1;
					typedef typename add_unsigned<T>::value utype;
					utype x=(v << 1) ^ (v >> s);
					varint_coder< utype >::save( os, x );
				}else{  
					// Unsigned Varint
					if( v < 0x80 ){	os.put( (char)v ); return; }
					T x=v;
					while( x > 0x80 ){
						os.put( char(x & 0x7F | 0x80) );
						x >>= 7;
					}
					os.put( (char)x );
				}
			}

			static void save( std::ostream& os, uint32_t key, const T& v, int option=0 )
			{
				varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
				save( os, v );
			}

			/// load varint from istream
			static void load( std::istream& os, T& v, size_t sz=0 )
			{
				v=0;
				int m=0;
				while( true ){
					uint8_t c=(uint8_t)os.get();
					if( os.eof() ) break;
					T x=c & 0x7F;
					v |= (x << m);
					m+=7;
					if( 0==(c & 0x80) ) break;
					if( m > 128 ) throw std::bad_cast();
				}
				if( !std::is_unsigned<T>::value ){
					int s=8 * sizeof(T) - 1;
					if( v & 1 ){
						v=(v >> 1) ^ T(-1);
					}else 
						v >>= 1;
				}
			}
		};

		template<>
		struct varint_coder<bool>
		{
			enum { wire=0 };
			static void save( std::ostream& os, const bool& v, int option=0 ){ os.put( v ? 1 : 0 ); }
			static void save( std::ostream& os, uint32_t key, const bool& v, int option=0 )
			{
				varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
				save( os, v );
			}
			static void load( std::istream& os, bool& v, size_t ){ v=(os.get()!=0); }
		};

		/// code float 
		template<>
		struct varint_coder<float>
		{
			enum { wire=5 };
			static void save( std::ostream& os, const float& v, int option=0 ){ os.write( (const char*)&v, sizeof(float) ); }
			static void save( std::ostream& os, uint32_t key, const float& v, int option=0 )
			{
				varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
				save( os, v );
			}
			static void load( std::istream& os, float& v, size_t ){ os.read( (char*)&v, sizeof(float) ); }
		};

		/// code double
		template<>
		struct varint_coder<double>
		{
			enum { wire=1 };
			static void save( std::ostream& os, const double& v, int option=0 ){ os.write( (const char*)&v, sizeof(double) ); }
			static void save( std::ostream& os, uint32_t key, const double& v, int option=0 )
			{
				varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
				save( os, v );
			}
			static void load( std::istream& os, double& v, size_t ){ os.read( (char*)&v, sizeof(double) ); }
		};
		namespace details{

			template< class T, class Archive >
			struct pair_coder;

			template< class V, class Archive >
			struct class_coder;

			template< typename Archive, typename V >
			struct container_coder;

			/// optional value
			template< class T, class Archive >
			struct optional_coder;

			template< class T, class Archive >
			struct optional_coder< optional<T>, Archive > : public pair_coder<std::pair<bool,T>, Archive>{};

			template< typename Archive, typename T >
			struct get_coder
			{
				typedef typename std::remove_reference<T>::type _Ty;
				typedef typename
					std::conditional< std::is_arithmetic<_Ty>::value, varint_coder< _Ty >,	// 如果是整型或浮点
					typename std::conditional< std::is_enum<_Ty>::value, varint_coder< _Ty >, // 如果是枚举类型
					typename std::conditional< is_pair<_Ty>::value, pair_coder< _Ty, Archive >,
					typename std::conditional< is_optional<_Ty>::value, optional_coder< _Ty, Archive >,
					typename std::conditional< lugce::is_container<_Ty>::value, container_coder< Archive, _Ty >, 
					class_coder< _Ty, Archive >
					>::type
					>::type
					>::type
					>::type
					>::type type;
			};

			/// code custom class
			template< class T, class Archive >
			struct class_coder
			{
				enum { wire=2 };
				typedef typename get_coder<Archive,T>::type coder;
				static void save( std::ostream& os, const T& v, int option=0 )
				{
					std::ostringstream tmp;
					Archive ar( tmp );
					serialize( ar, const_cast<T&>(v) );

					// write size to stream
					varint_coder<uint32_t>::save( os, (uint32_t)tmp.tellp() );
					os.write( tmp.str().data(), tmp.tellp() );
				}

				static void save( std::ostream& os, uint32_t key, const T& v, int option=0 )
				{
					varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
					save( os, v );
				}

				static void load( std::istream& os, T& v, size_t sz )
				{
					uint32_t len=0;
					varint_coder<uint32_t>::load( os, len, sz );
					Archive ar(os);
					serialize( ar, v );
					ar.load( sz );
				}
			};

			/// coder for pair
			/// pair is code like(proto):
			///   message pair {
			///		required type left=1;
			///		required type right=2;
			///		}
			template< typename T, class Archive >
			struct pair_coder
			{
				enum { wire=2 };
				typedef T value_type;

				static void save( std::ostream& os, const value_type& v, int option=0 )
				{
					std::stringstream tmp;
					typedef typename get_coder<Archive, typename std::remove_cv<typename value_type::first_type>::type>::type coder1;
					varint_coder<uint32_t>::save( tmp, wire_key( coder1::wire, 1 ) );
					coder1::save( tmp, v.first );
					typedef typename get_coder<Archive, typename std::remove_cv<typename value_type::second_type>::type>::type coder2;
					varint_coder<uint32_t>::save( tmp, wire_key( coder2::wire, 2 ) );
					coder2::save( tmp, v.second );

					varint_coder<uint32_t>::save( os, (uint32_t)tmp.tellp() );	// write size to stream
					os.write( tmp.str().data(), tmp.tellp() );
				}

				static void save( std::ostream& os, uint32_t key, const value_type& v, int option=0 )
				{
					varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
					save( os, v, option );
				}

				static void load( std::istream& os, value_type& v, size_t sz )
				{
					uint32_t len=0;
					varint_coder<uint32_t>::load( os, len, sz );	// load size from stream

					uint32_t key;
					typedef typename get_coder<Archive, typename std::remove_cv<typename value_type::first_type>::type>::type coder1;
					varint_coder<uint32_t>::load( os, key );	
					if( key!=wire_key( coder1::wire, 1 ) ) throw std::bad_cast();
					coder1::load( os, v.first );

					typedef typename get_coder<Archive, typename std::remove_cv<typename value_type::second_type>::type>::type coder2;
					varint_coder<uint32_t>::load( os, key );	
					if( key!=wire_key( coder2::wire, 2 ) ) throw std::bad_cast();
					coder2::load( os, v.second );
				}
			};

			/// optional value
			template< class T, class Archive >
			struct pair_coder< std::pair<bool,T>, Archive >
			{
				enum { wire=2 };
				typedef typename get_coder<Archive,T>::type coder;
				static void save( std::ostream& os, const std::pair<bool,T>& v, int option=0 )
				{
					if( v.first )
						coder::save( os, v.second );
				}

				static void save( std::ostream& os, uint32_t key, const std::pair<bool,T>& v, int option=0 )
				{
					if( v.first ){
						varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
						coder::save( os, v.second );
					}
				}

				static void load( std::istream& os, std::pair<bool,T>& v, size_t sz )
				{
					v.first=true; // optional exists, if this function been called.
					coder::load( os, v.second, sz );
				}
			};

			/// coder for string
			template< class _Traits, class _Alloc, class Archive >
			struct class_coder< std::basic_string<char,_Traits,_Alloc>, Archive >
			{
				enum { wire=2 };
				typedef std::basic_string<char,_Traits,_Alloc> value_type;
				static void save( std::ostream& os, const value_type& v, int option=0 )
				{
					varint_coder<size_t>::save( os, v.size() );
					os.write( v.data(), v.size() );
				}

				static void save( std::ostream& os, uint32_t key, const value_type& v, int option=0 )
				{
					varint_coder<uint32_t>::save( os, wire_key( wire, key ) );
					save( os, v );
				}

				static void load( std::istream& os, value_type& v, size_t sz )
				{
					uint32_t len=0;
					varint_coder<uint32_t>::load( os, len, sz );
					v.resize(len);
					os.read( &v[0], len );
				}
			};

			template< class T, class Archive >
			struct class_coder< optional<T>, Archive > : public class_coder< std::pair<bool,T>, Archive >{};

			/// code vector, deque, list, set, muilset, map, muilmap
			template< typename Archive, typename V >
			struct container_coder
			{
				enum { wire=2 };
				typedef typename V::value_type value_type;
				typedef typename get_coder<Archive,value_type>::type coder;

				static void save( std::ostream& os, uint32_t key, const V& v, int option=0 )
				{
					bool not_packed=option & unpacked;
					if( !(std::is_arithmetic<value_type>::value || std::is_enum<value_type>::value) )
						not_packed=true; // Only arithmetic can been packed.

					if( v.empty() ) return;

					typename V::const_iterator i=v.begin(), e=v.end();
					for( ; i!=e; ++i ){
						if( not_packed ) 
							coder::save( os, key, *i, 0 );
						else
							coder::save( os, *i );
					}
				}

				static void load( std::istream& os, V& v, size_t sz )
				{
					if(sz==0) return;
					size_t g=os.tellg();
					std::back_insert_iterator<V> it=std::back_inserter(v);
					while( !os.eof() && (g+sz <= os.tellg()) )
						coder::load( os, *it++, 0 );
				}
			};
		};
	};
};

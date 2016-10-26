/* 
* Copyright (C) 2013 
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
#include <memory>
#include <exception>
#include <typeindex>
#include <vector>
#include <list>
#include <deque>
#include <set>
#include <map>
#include <utility>
#include <typeinfo>

namespace lugce
{
	class bad_index : public std::exception
	{
	public:
		virtual const char * what() const throw()
		{
			return "bad index";
		}
	};

	/// 判断对象是否支持加法
	template< typename T >
	struct is_addible{ static const bool value=std::is_arithmetic<T>::value; };
	template<>
	struct is_addible<std::string>{ static const bool value=true; };
	template<>
	struct is_addible<std::wstring>{ static const bool value=true; };

	/// 判断对象是否支持窄字符流（注意，类型转换是通过流来进行的）
	template< typename T >
	struct is_stream_support{ static const bool value=std::is_arithmetic<T>::value; };
	template<>
	struct is_stream_support<std::string>{ static const bool value=true; };
	template<>
	struct is_stream_support<bool>{ static const bool value=true; };

	/// 一个可以存放任意对象的类
	class object
	{
	private:
		struct basic_helper
		{
			virtual const std::type_info& info() const=0;
			virtual ~basic_helper(){}
			virtual basic_helper* clone()=0;
			virtual bool empty() const=0;
		};

		/// 对变量的操作
		struct variable_if
		{
			virtual void* get()=0;
			virtual const void* get() const=0;
		};

		/// 对变量的操作
		struct stream_if
		{
			virtual void save( std::ostream& )=0;
		};

		struct add_if
		{
			virtual void add( std::istream& )=0;
			virtual void add( const void* )=0;
		};

		/// 算术操作接口
		struct arithmetic_if
		{
			virtual void subtract( std::istream& )=0;
			virtual void subtract( const void* )=0;
			virtual void multiply( std::istream& )=0;
			virtual void multiply( const void* )=0;
			virtual void divide( std::istream& )=0;
			virtual void divide( const void* )=0;
		};

		/// 对容器的操作
		struct vector_if
		{
			virtual void push( object&& obj )=0;
			virtual object& at( const size_t index )=0;
			virtual const object& at( const size_t index ) const=0;
		};

		/// 对 table 的操作
		struct table_if
		{
			virtual void push( const std::string& key, object&& obj )=0;
			virtual object& at( const std::string& key )=0;
			virtual const object& at( const std::string& key ) const=0;
		};
	private:
		std::shared_ptr<basic_helper> _helper;
	private:
		template< typename A, typename T, bool addible >
		struct add_if_imp : public add_if
		{
			virtual void add( std::istream& o )
			{
				T& p=static_cast<A*>( this )->value;
				T v;
				o >> v;
				p+=v;
			}
			virtual void add( const void* v )
			{ 
				T& p=static_cast<A*>( this )->value;
				p+=*(T*)v; 
			}
		};

		template< typename A, typename T >
		struct add_if_imp<A,T,false>{};

		template< typename A, typename T, bool addible >
		struct stream_if_imp : public stream_if
		{
			virtual void save( std::ostream& o )
			{
				T& p=static_cast<A*>( this )->value;
				o << p;
			}
		};
		template< typename A, typename T>
		struct stream_if_imp<A,T,false>{};

		template< typename A, typename T, bool >
		struct arithmetic_if_imp : public arithmetic_if
		{
			const T cast( std::istream& o )
			{
				T v;
				o >> v;
				return v;
			}
			virtual void subtract( std::istream& o )
			{
				T& p=static_cast<A*>( this )->value;
				p-=cast(o);
			}

			virtual void subtract( const void* o )
			{
				T& p=static_cast<A*>( this )->value;
				p-=cast(o);
			}

			virtual void multiply( std::istream& o )
			{
				T& p=static_cast<A*>( this )->value;
				p*=cast(o);
			}

			virtual void multiply( const void* o )
			{
				T& p=static_cast<A*>( this )->value;
				p*=cast(o);
			}

			virtual void divide( std::istream& o )
			{
				T& p=static_cast<A*>( this )->value;
				p*=cast(o);
			}

			virtual void divide( const void* o )
			{
				T& p=static_cast<A*>( this )->value;
				p*=cast(o);
			}
		};
		template< typename A, typename T>
		struct arithmetic_if_imp<A,T,false>{};

		template< typename T >
		struct helper : public basic_helper, public variable_if, 
			public add_if_imp< helper<T>, T, is_addible<T>::value >,
			public stream_if_imp< helper<T>, T, is_stream_support<T>::value >
		{
			helper( const T& v ){ value=v; }
			helper( T&& v ){ value=std::move(v); }
			virtual const std::type_info& info() const{ return typeid(T); }
			virtual basic_helper* clone(){ return new helper<T>( value ); }
			virtual const void* get() const{ return &value; }
			virtual void* get(){ return &value; }
			virtual bool empty() const{ return false; }
			T value;
		};

		struct vector_helper : public basic_helper, public vector_if
		{
			vector_helper(){}
			virtual basic_helper* clone(){ return new vector_helper( *this ); }
			virtual void push( object&& obj ){ _vec.push_back( std::move(obj) ); }
			virtual object& at( const size_t index )
			{
				if( index == _vec.size() ){ // 如果是最后，那么添加
					_vec.push_back( object() );
					return _vec.back();
				}else if( index > _vec.size() )
					throw bad_index();
				return _vec.at(index); 
			}
			virtual const object& at( const size_t index ) const
			{
				if( index >= _vec.size() ) throw bad_index();
				return _vec.at(index); 
			}
			virtual bool empty() const{ return _vec.empty(); }
			virtual const std::type_info& info() const{ return typeid( decltype(_vec) );  }
			~vector_helper(){ _vec.clear(); }
		private:
			std::vector< object > _vec;
		};

		struct table_helper : public basic_helper, public table_if
		{
			table_helper(){ }
			virtual basic_helper* clone(){ return new table_helper( *this ); }
			virtual void push( const std::string& key, object&& obj )
			{
				_table[key]=std::move(obj);
			}
			virtual object& at( const std::string& key )
			{
				return _table[key];
			}
			virtual const object& at( const std::string& key ) const
			{
				auto i=_table.find( key );
				if( i==_table.end() ) throw bad_index();
				return i->second;
			}
			virtual bool empty() const{ return _table.empty(); }
			virtual const std::type_info& info() const{ return typeid( decltype(_table) );  }
			~table_helper(){ _table.clear(); }
		private:
			std::map< std::string, object > _table;
		};
	private:
		template< typename T >
		struct set_helper
		{
			static basic_helper* set( const T& v )
			{
				return new helper< T >( v );
			}

			static basic_helper* set( T&& v )
			{
				return new helper< T >( std::forward<T&&>(v) );
			}
		};

		template< typename T >
		struct container
		{
			static basic_helper* set( const T& v )
			{
				vector_helper *p=new vector_helper();
				for( const auto& i : v ){ p->push( i ); }
				return p;
			}
			static basic_helper* set( T&& v )
			{
				vector_helper *p=new vector_helper();
				for( auto& i : v ){ p->push( std::move(i) ); }
				return p;
			}
		};

		template< typename T >
		struct set_helper< std::vector<T> >
		{
			static basic_helper* set( const std::vector<T>& v ){ return	container< std::vector<T> >::set( v ); }
			static basic_helper* set( std::vector<T>&& v ){	return	container< std::vector<T> >::set( std::move(v) ); }
		};

		template< typename T >
		struct set_helper< std::list<T> >
		{
			static basic_helper* set( const std::vector<T>& v ){ return	container< std::list<T> >::set( v ); }
			static basic_helper* set( std::vector<T>&& v ){	return	container< std::list<T> >::set( std::move(v) ); }
		};

		template< typename T >
		struct set_helper< std::deque<T> >
		{
			static basic_helper* set( const std::vector<T>& v ){ return	container< std::deque<T> >::set( v ); }
			static basic_helper* set( std::vector<T>&& v ){	return	container< std::deque<T> >::set( std::move(v) ); }
		};

		template< typename T >
		struct set_helper< std::set<T> >
		{
			static basic_helper* set( const std::vector<T>& v ){ return	container< std::set<T> >::set( v ); }
			static basic_helper* set( std::vector<T>&& v ){	return	container< std::set<T> >::set( std::move(v) ); }
		};

		template< typename T >
		void set( T&& v )
		{
			typedef std::remove_const< typename std::remove_reference<typename T>::type >::type value_type;
			_helper.reset( set_helper<typename value_type>::set( std::forward<T&&>(v) ) );
		}
	private:
		template< typename T >
		void constructor( T&& v )
		{
			set( std::forward<T&&>(v) );
		}

		template<> /// Move constructor
		void constructor( object&& v )
		{
			_helper.swap(v._helper);
		}

		vector_if* get_vector_if()
		{
			if( _helper==nullptr ){ // 如果对象为空，自动创建为 vec
				vector_helper* p=new vector_helper();
				_helper.reset(p);
				return p;
			}
			vector_if *p=dynamic_cast<vector_if*>( _helper.get() );
			if(!p) throw bad_index();
			return p;
		}
	public:
		object(){}

		object( const char * str ) : _helper( new helper< std::string >( str ) ){}

		/// Copy constructor
		object( const object& o ) 
		{
			if( !o.empty() )
				_helper.reset( o._helper->clone() );
		}

		template< typename T >
		object( T&& v ) 
		{
			constructor( std::forward<T&&>(v) );
		}

		const std::type_info& info() const{ return _helper ? _helper->info() : typeid(void); }

		object clone() const
		{
			object o;
			if( _helper ) o._helper.reset( _helper->clone() );
			return o;
		}
		void clear(){ _helper.reset(); }
		bool empty() const{ return _helper ? _helper->empty() : true; }

		object& at( const size_t index )
		{
			if( empty() && index!=0 ) throw bad_index();
			vector_if *p=get_vector_if();
			if(!p) throw bad_index();
			return p->at(index);
		}

		const object& at( const size_t index ) const
		{
			const vector_if *p=dynamic_cast<const vector_if*>( _helper.get() );
			if(!p) throw bad_index();
			return p->at(index);
		}

		template< typename T >
		void push( T&& v )
		{
			vector_if *p=get_vector_if();
			if(!p) throw bad_index();
			p->push( std::forward<T&&>(v) );
		}

		object& at( const std::string& index )
		{
			if( _helper==nullptr ){ // 如果对象为空，自动创建为 vec
				table_helper* p=new table_helper();
				_helper.reset(p);
				return p->at(index);
			}
			table_if *p=dynamic_cast<table_if*>( _helper.get() );
			if(!p) throw bad_index();
			return p->at(index);
		}
		object& at( const char * index ){ return at( std::string(index) ); }

		const object& at( const std::string& index ) const
		{
			const table_if *p=dynamic_cast<const table_if*>( _helper.get() );
			if(!p) throw bad_index();
			return p->at(index);
		}
		const object& at( const char * index ) const{ return at( std::string(index) ); }

		object& at( const object& o )
		{
			auto *p= dynamic_cast<variable_if*>(o._helper.get());
			if(p==nullptr) throw bad_index();
			if( o.info()==typeid(std::string) )
				return at( *(std::string*)p->get() );
			else
				return at( o.get<size_t>() );
		}

		const object& at( const object& o ) const
		{
			auto *p= dynamic_cast<const variable_if*>(o._helper.get());
			if(p==nullptr) throw bad_index();
			if( o.info()==typeid(std::string) )
				return at( *(std::string*)p->get() );
			else
				return at( (size_t)o );
		}

		template< typename T >
		object& operator[]( const T& index ){ return at(index); }
		template< typename T >
		const object& operator[]( const T& index ) const{ return at(index); }

		template< typename T >
		const T get() const
		{
			if( !_helper ) return T();
			variable_if* var=dynamic_cast<variable_if*>(_helper.get());
			if( var==nullptr ) return T();

			if( typeid(T)==info() ){
				return *(T*)( var->get() );
			}else{ // Try to cast to the type with stream
				std::stringstream ss;
				stream_if* o=dynamic_cast<stream_if*>(_helper.get());
				if(!o) throw std::bad_cast();
				o->save(ss);
				T v=T();
				ss >> v;
				if( ss.rdbuf()->in_avail() > 0 ) 
					throw std::bad_cast();
				return v;
			}
		}

		template< typename T >
		object& operator += ( const T& right ){	return arithmetic( &add_if::add, right ); }

		template< typename T >
		object operator + ( const T& right ) const
		{
			if( !_helper ) return object( right );
			return (object( *this )+=right);
		}

		template< typename T >
		object& operator -= ( const T& right ){ return arithmetic( &arithmetic_if::subtract, right ); }

		template< typename T >
		object& operator *= ( const T& right ){ return arithmetic( &arithmetic_if::multiply, right ); }

		template< typename T >
		object& operator /= ( const T& right ){	return arithmetic( &arithmetic_if::divide, right ); }

		template< typename T >
		object operator - ( const T& right ) const
		{
			if( !_helper ) throw bad_cast();
			return (object( *this )-=right);
		}

		template< typename T >
		object operator * ( const T& right ) const
		{
			if( !_helper ) throw bad_cast();
			return (object( *this )*=right);
		}

		template< typename T >
		object operator / ( const T& right ) const
		{
			if( !_helper ) throw bad_cast();
			return (object( *this )/=right);
		}

		template< typename T >
		operator const T() const{ return get<T>(); }
	private:
		template< typename C, typename T, typename FUN >
		object& arithmetic( FUN* func, const T& v )
		{
			if( !_helper )
				throw bad_cast();
			else{
				C *p=dynamic_cast<C*>(_helper.get());
				if(!p) throw std::bad_cast();
				if( info()==typeid(T) )
					(p->*func)( &right );
				else{
					std::stringstream ss;
					ss << right;
					(p->*func)( ss );
				}
			}
			return *this;
		}
	public:
		friend std::ostream & operator << ( std::ostream& os, const object& o )
		{
			stream_if *p=dynamic_cast<stream_if*>(o._helper.get());
			if( p!=nullptr )
				p->save(os);
			return os;
		}
	};
};

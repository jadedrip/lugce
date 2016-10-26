#pragma once
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace lugce
{
	/// 一致性哈希算法容器
	/// <remarks>算法的描述可以参考 http://blog.csdn.net/sparkliang/article/details/5279393 </remarks>
	template< typename T >
	class consistent_hashing_container
	{
		friend class handle;
	private:
		typedef std::map< uint32_t, typename T > values_type;
		struct data_type
		{
			boost::shared_mutex rwmutex;
			values_type value_map;
		};
		boost::shared_ptr<data_type> _data;
	public:
		/// 一致性哈希算法句柄，插入时返回，可以用来移除插入的值
		class handle
		{
			friend class consistent_hashing_container<T>;
		public:
			typedef boost::shared_ptr< handle > pointer;
			/// 从一致性哈希算法容器中移除插入的节点
			void remove()
			{
				boost::unique_lock<boost::shared_mutex> write_lock( _data->rwmutex );
				BOOST_AUTO( i, _hash_keys.begin() );
				BOOST_AUTO( e, _hash_keys.end() );
				for( ; i!=e; ++i )
					_data->value_map.erase( *i );
				_hash_keys.clear();
			}
			operator const bool () const{ return !_hash_keys.empty(); }
		private:
			handle( const boost::shared_ptr< data_type >& d ) : _data(d){} //
			boost::shared_ptr< data_type > _data;
			std::vector< size_t > _hash_keys;
		};
	public:
		consistent_hashing_container() : _data( new data_type() )
		{
		}

		/// 通过 Key 获取一个值
		/// <remarks>调用时需要保证类型 U 可以作为被 boost::hash 函数的参数</remarks>
		template< typename U >
		const T get( const U& key ) const
		{
			assert( _data );
			if( _data->value_map.empty() ) return T();
			boost::hash<typename U> hasher;
			uint32_t v=hasher( key );
			boost::shared_lock<boost::shared_mutex> read_lock( _data->rwmutex );
			BOOST_AUTO( i, _data->value_map.lower_bound(v) );
			if( i==_data->value_map.end() ) i=_data->value_map.begin();
			return i->second;
		}

		template< typename U >
		handle set( const U& key, const T& value, const int cnt=32 )
		{
			assert( _data );
			size_t seed=boost::hash_value( key );
			handle h( _data );

			boost::unique_lock<boost::shared_mutex> write_lock( _data->rwmutex );
			if( !set_one(seed,value) )
				return h;
			h._hash_keys.push_back(seed);

			for( int i=0; i<cnt-1; ++i ){
				boost::hash_combine( seed, i+1 );
				if( set_one( seed, value ) )
					h._hash_keys.push_back(seed);
				else{
					h.remove();
					return h;
				};
			}
			return h;
		}
	private:
		bool set_one( const uint32_t hash_key, const T& v )
		{
			assert( _data );
			return _data->value_map.insert( std::make_pair(hash_key, v) ).second;
		}
	};
};
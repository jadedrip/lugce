#pragma once
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/preprocessor/variadic/size.hpp>
#include <boost/preprocessor/variadic/to_list.hpp>
#include <boost/preprocessor/punctuation/paren.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/if.hpp>
#include <boost/function.hpp>
#include <boost/typeof/typeof.hpp>
#include <boost/type_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/ref.hpp>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#	define _HAS_CPP0X 1
#endif

#define MAX_SIGNAL_FUNCTION_PARAMS 9
#ifdef _HAS_CPP0X
#	define OBSERVER_TYPE_ID_TYPE size_t
#	define OBSERVER_GET_ID_BY_TYPE(x) x.hash_code()
#else
#	define OBSERVER_TYPE_ID_TYPE std::string
#	define OBSERVER_GET_ID_BY_TYPE(x) x.name()
#endif

namespace lugce{
	// 定义事件的基类
	class event_base 
	{
		friend class observer;
	public:
		event_base( const OBSERVER_TYPE_ID_TYPE v ) : _id(v){}
		OBSERVER_TYPE_ID_TYPE get_id() const{ return _id; }
	protected:
		OBSERVER_TYPE_ID_TYPE _id;
		virtual void shot( boost::function_base* )=0;
	};

	namespace details{
		// 把 mpl 转换成列表, 由于引用类型在 tuple 里无法存在，因此对于引用，需要转换成 boost::reference_wrapper<X> 的形式
		template< int N, class T > 
		struct mpl_vec2tuple_helper;

#define OBSERVER_TYPE_LINE( Z, N, _ ) typedef typename boost::mpl::at_c<param_type, N>::type BOOST_PP_CAT(T,N);
#define OBSERVER_EVENT_TUPLE_LINE( Z, N, _) BOOST_PP_COMMA_IF(N) typename boost::mpl::if_c< boost::is_reference<BOOST_PP_CAT(T,N)>::value, typename boost::reference_wrapper< typename boost::remove_reference< BOOST_PP_CAT(T,N)>::type >, BOOST_PP_CAT(T,N) >::type
#define OBSERVER_TUPLE_HELPER( Z, N, _ ) \
	template< class T > \
		struct mpl_vec2tuple_helper<N,T>{ \
			typedef T param_type; \
			BOOST_PP_REPEAT( N, OBSERVER_TYPE_LINE, _ ) \
			typedef boost::tuple< BOOST_PP_REPEAT( N, OBSERVER_EVENT_TUPLE_LINE, _ ) > tuple_type; \
		};

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_TUPLE_HELPER, _ )

		struct shot_base{  
			virtual boost::function_base* get_function()=0;
			virtual ~shot_base(){} 
		}; 

		template< int i, class T > class shot_void_helper;

#define OBSERVER_SIGNAL_EVENT_IMPL( Z, N, _ ) \
	template< class T > \
		class shot_void_helper<N,T> : public shot_base \
		{ \
		public: \
			typedef typename T::param_type param_type; \
			BOOST_PP_REPEAT( N, OBSERVER_TYPE_LINE, _ ) \
			void shot( BOOST_PP_ENUM_BINARY_PARAMS(N, const T, &v ) ){ _callback( BOOST_PP_ENUM_PARAMS(N, v) ); } \
			virtual boost::function_base* get_function(){ return &_callback; } \
			shot_void_helper( const boost::function< void( BOOST_PP_ENUM_PARAMS(N, T) ) >& v ) : _callback(v){} \
		private: \
			boost::function< void( BOOST_PP_ENUM_PARAMS(N, T) ) > _callback; \
		};

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_SIGNAL_EVENT_IMPL, _ )
		/* 这里通过 PP 产生适合不同参数数量的包装，展开的代码类似这样:
		template< class T >
		class shot_void_helper<1,T> : public shot_base
		{
		public:
			typedef typename boost::mpl::at_c<T, 0>::type T0;
			void shot( const T0& v1 ){ _callback(v1); }
			virtual boost::function_base* get_function(){ return &_callback; }
			shot_void_helper( const boost::function< void( T0 ) >& v ) : _callback(v){}
		private:
			boost::function< void( T1 ) > _callback;
		};
		*/
	};

	/** 这是一个观察者模式的简化实现，有助于代码的解耦。
	* \remarks 你可以预先定义一些事件，事件的定义使用OBSERVER_EVENT宏：
	*	OBSERVER_EVENT( Name, <Params> )
	*		Name 是事件的名称
	*		Params 是参数表
	* 比如 
	*	OBSERVER_EVENT( MyEvent, int, std::string, long )
	*	 
	** 注：OBSERVER_EVENT 可以在类定义的内部使用的。
	*
	* 有必要的时候，就可以将一个回调函数绑定到这个事件（订阅）
	*	 observer a;
	*	 a.subscribe<Name>( Handle );
	*		Name 是事件的名称
	*		Handle 是回调函数，它的参数，应该和事件的参数表匹配
	* 当然也可以撤销订阅
	*	 a.unsubscribe<Name>();
	*		
	* 当事件发生时，可以通过 observer 对象来发送事件
	*	a.shot<Name>( <Params> );
	* 另外，也允许定义事件，并放入容器，以便延迟调用
	*   lugce::event_base* xx=new MyEvent( "hello" );
	*   a.shot( xx );
	*	
	* 这个对象可以作为基类使用，以帮助对象解耦，这样设计的优点在于，事件的定义、回调的参数表
	* 必须严格匹配，否则就会发生编译错误，以防止代码错误。
	* 特别的，参数可以定义为引用类型，以便让回调函数可以修改它，这时发送事件时，要注意使用 ref() 来包装参数。
	*/
	class observer
	{
	public:
#define OBSERVER_SHOT_FUNCTION(Z, N, _) \
	template<class X BOOST_PP_COMMA_IF(N) BOOST_PP_ENUM_PARAMS(N, class T)> \
	void shot( BOOST_PP_ENUM_BINARY_PARAMS(N, T, v) ) \
	{  \
		typedef typename X::param_type mpl_vec; \
		BOOST_AUTO( i, _shots.find( OBSERVER_GET_ID_BY_TYPE(typeid(X)) ) ); \
		if( i!=_shots.end() ) static_cast<details::shot_void_helper< N, X >*>(i->second)->shot( BOOST_PP_ENUM_PARAMS(N, v) ); \
	}

		BOOST_PP_REPEAT( MAX_SIGNAL_FUNCTION_PARAMS, OBSERVER_SHOT_FUNCTION, _ )

		template< class X, class HANDLE >
		void subscribe( HANDLE h )
		{
			typedef typename X::param_type mpl_vec;
			typedef details::shot_void_helper< boost::mpl::size<mpl_vec>::value, X > helper;
			assert( _shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ]==NULL );
			delete _shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ];
			_shots[ OBSERVER_GET_ID_BY_TYPE(typeid(X)) ]=new helper(h);
		}
		template< class X >
		void unsubscribe(){ unsubscribe(OBSERVER_GET_ID_BY_TYPE(typeid(X))); }
		void unsubscribe( const event_base* ev ){ unsubscribe( ev->get_id() ); }
		void unsubscribe( const OBSERVER_TYPE_ID_TYPE &id )
		{ 
			BOOST_AUTO( i, _shots.find(id) );
			if( i!=_shots.end() ){
				delete i->second;
				_shots.erase(i);
			}		
		}

		void shot( event_base* ev )
		{
			BOOST_AUTO( i, _shots.find( ev->get_id() ) ); 
			if( i!=_shots.end() ) 
				ev->shot( i->second->get_function() );
		}
		~observer()
		{
			BOOST_AUTO( i, _shots.begin() );
			BOOST_AUTO( e, _shots.begin() );
			for( ; i!=e; ++i ) delete i->second;
		}
	protected:
		std::map< OBSERVER_TYPE_ID_TYPE, lugce::details::shot_base* > _shots;
	};
};

#undef OBSERVER_EVENT_TUPLE_LINE
#undef OBSERVER_TUPLE_HELPER
#undef OBSERVER_SIGNAL_EVENT_IMPL
#undef OBSERVER_TYPE_LINE
#undef OBSERVER_SHOT_FUNCTION
#undef MAX_SIGNAL_FUNCTION_PARAMS

#define OBSERVER_EVENT_CREATOR(r, data, i, elem) BOOST_PP_COMMA_IF(i) \
	const boost::mpl::if_c< boost::is_reference<elem>::value, boost::reference_wrapper< boost::remove_reference<elem>::type >, elem >::type \
	BOOST_PP_CAT( &v, i )
#define OBSERVER_EVENT_TUPLE( ... ) lugce::details::mpl_vec2tuple_helper< \
			BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), \
			boost::mpl::vector< __VA_ARGS__ > \
		>::tuple_type
#define OBSERVER_EVENT_HELPER_GET_LINE( Z, N, _ ) BOOST_PP_COMMA_IF(N) boost::get<N>( *this )

/// 这个宏用来定义事件
#define OBSERVER_EVENT( NAME, ... ) \
	struct NAME : public OBSERVER_EVENT_TUPLE(__VA_ARGS__), public lugce::event_base { \
		typedef OBSERVER_EVENT_TUPLE(__VA_ARGS__) tuple_type; \
		typedef boost::function< void( __VA_ARGS__ ) > function_type; \
		typedef boost::mpl::vector< __VA_ARGS__ > param_type; \
		NAME( BOOST_PP_LIST_FOR_EACH_I(OBSERVER_EVENT_CREATOR, _, BOOST_PP_VARIADIC_TO_LIST( __VA_ARGS__ ) ) ) \
		 : tuple_type( BOOST_PP_ENUM_PARAMS(BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), v) ), event_base( OBSERVER_GET_ID_BY_TYPE(typeid(NAME) )) {} \
	private: \
		virtual void shot( boost::function_base* p ){ \
			function_type& cb=*static_cast< function_type* >(p); \
			cb BOOST_PP_LPAREN() BOOST_PP_REPEAT( BOOST_PP_VARIADIC_SIZE( __VA_ARGS__ ), OBSERVER_EVENT_HELPER_GET_LINE, _ ) BOOST_PP_RPAREN(); \
		} \
	};

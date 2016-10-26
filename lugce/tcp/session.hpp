#pragma once
#include <assert.h>
#include <string>
#include <streambuf>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/coroutine/all.hpp>
#include <boost/thread.hpp>

namespace lugce{ namespace tcp{
	class session;
	class tcpinbuf : public std::streambuf, private boost::noncopyable
	{
		friend class session;
		typedef boost::coroutines::coroutine< void( boost::system::error_code e, size_t ) > incoro_t;
	private:
		enum { bf_size = 1024 };

		int fetch()
		{
			assert( egptr() == gptr() );
			if( !_socket.is_open() ) return -1;

			// read bytes from the socket into internal buffer 'buffer_'
			// make incoro_t::operator() as callback, invoked if some
			// bytes are read into 'buffer_'
			_socket.async_read_some(
				boost::asio::buffer( _inbuf, bf_size ),
				boost::bind( & incoro_t::operator(), & _coro, _1, _2) );
			// suspend this coroutine
			_ca();

			// coroutine was resumed by boost::asio::io_sevice
			boost::system::error_code ec;
			std::size_t n = 0;

			// check arguments
			boost::tie(ec, n) = _ca.get();

			// check if an error occurred
			if (ec){
				setg( 0, 0, 0);
				if( !_error )	_error=ec;
				_socket.close(ec);
				return -1;
			}

			setg( _inbuf, _inbuf, _inbuf + n);
			return n;
		}
	private:
		boost::asio::ip::tcp::socket      &   _socket;
		incoro_t                          &   _coro;
		incoro_t::caller_type             &   _ca;
		char                              _inbuf[bf_size];
		boost::thread_specific_ptr< std::string > _outbuf;
		boost::system::error_code		  _error;	
		boost::thread::id				  _thread_id;
	protected:
		virtual int underflow()
		{
			if ( gptr() < egptr() )
				return traits_type::to_int_type( *gptr() );

			if ( 0 > fetch() )
				return traits_type::eof();
			else
				return traits_type::to_int_type( *gptr() );
		}

		virtual int_type overflow( int_type val )
		{
			if( !_outbuf.get() ){
				_outbuf.reset( new std::string() );
				_outbuf->reserve( 1024 );
			}
			_outbuf->push_back( val );
			return val;
		}

		virtual std::streamsize xsputn( const char *_Ptr, std::streamsize _Count )
		{
			if( !_outbuf.get() ){
				_outbuf.reset( new std::string() );
				_outbuf->reserve( 1024 );
			}
			_outbuf->append( _Ptr, (unsigned int)_Count );
			return _Count;
		}
		typedef boost::coroutines::coroutine< boost::system::error_code( std::string* ) > outcoro_t;

		void handle_write( std::string *s, const boost::system::error_code& e )
		{
			delete s;
			if( e && e!=boost::asio::error::operation_aborted )
				if( _error ) _error=e;
		}

		virtual int sync()
		{ 
			std::string *s=_outbuf.release();
			if(!s) return 0;

			size_t sz=s->size();
			if( boost::this_thread::get_id()==_thread_id ){ // 如果是同一线程，那么借用 coro 实现伪同步
				boost::asio::async_write( _socket, boost::asio::buffer(*s), boost::bind( & incoro_t::operator(), & _coro, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred) );
				_ca();
				boost::system::error_code ec;
				std::size_t n = 0;

				// check arguments
				boost::tie(ec, n) = _ca.get();

				// check if an error occurred
				if (ec){
					if( !_error ) _error=ec;
					return -1;
				}
			}else{
				assert( 0 ); // 异步写也许会带来其他问题，你确定要这么做？
				boost::asio::async_write( _socket, boost::asio::buffer(*s), boost::bind( &tcpinbuf::handle_write, this, s, boost::asio::placeholders::error ) );
			}
			return sz; 
		}
	public:
		tcpinbuf(
			boost::asio::ip::tcp::socket & s,
			incoro_t & coro,
			incoro_t::caller_type & ca) 
			: _socket( s), _coro( coro), _ca( ca)
		{ 
			setg( _inbuf, _inbuf, _inbuf); 
			_thread_id=boost::this_thread::get_id();
		}
	};

	class session : private boost::noncopyable
	{
		typedef boost::coroutines::coroutine< void( boost::system::error_code e, size_t ) > incoro_t;
	public:
		typedef incoro_t::caller_type pause_type;
		typedef const boost::function< void() > resume_type;
		typedef boost::function< void( std::iostream&, boost::system::error_code&, resume_type &, pause_type & ) > callback_t;
	private:
		void resume(){ _io_service.post( boost::bind(&incoro_t::operator(), &_coro, boost::system::error_code(), 0 ) ); }

		void handle_read_( const boost::system::error_code& ec, incoro_t::caller_type & ca)
		{
			// create stream-buffer with coroutine
			tcpinbuf buf( *_socket, _coro, ca );
			std::iostream s( &buf );
			if( ec ){ buf._error=ec; }
			_callback( s, boost::ref(buf._error), boost::bind( &session::resume, this), boost::ref(ca) );
			_io_service.post( boost::bind(&session::destroy_, this) );
		}

		void handle_connect_( const boost::asio::ip::tcp::endpoint& ep, incoro_t::caller_type & ca )
		{
			_socket->async_connect( ep, boost::bind( &incoro_t::operator(), &_coro, _1, 0 ) );
			ca();
			boost::system::error_code ec;
			std::size_t n = 0;
			// check arguments
			boost::tie(ec, n) = ca.get();
			handle_read_(ec,ca);
		}

		void destroy_(){ delete this; }
	private:
		incoro_t                        _coro;
		boost::asio::io_service     &   _io_service;
		boost::asio::ip::tcp::socket*   _socket;
		callback_t _callback;
	private:
		session( boost::asio::ip::tcp::socket* socket, const callback_t& callback ) : 
			_io_service( socket->get_io_service() ),
			_socket( socket ), _callback(callback)
		{}

		session( boost::asio::io_service& ios, const callback_t& callback ) : _io_service( ios ),
			_socket( new boost::asio::ip::tcp::socket(ios) ), _callback(callback)
		{}
	public:
		~session(){ delete _socket; }

		static void accept( boost::asio::ip::tcp::socket * c, const callback_t& callback )
		{
			session *p=new session(c, callback);
			// create and start a coroutine
			// handle_read_() is used as coroutine-function
			p->_coro = incoro_t( boost::bind( & session::handle_read_, p, boost::system::error_code(), _1) );
		}

		static void connect( boost::asio::io_service& is, const boost::asio::ip::tcp::endpoint& ep, const callback_t& callback )
		{
			session *p=new session(is, callback);
			p->_coro = incoro_t( boost::bind( & session::handle_connect_, p, ep, _1) );
		}
	};
} };

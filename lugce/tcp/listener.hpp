#pragma once

#include <assert.h>
#include <memory>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/thread/tss.hpp>
#include "lugce/observer.hpp"

namespace lugce{ namespace tcp{
	namespace asio=boost::asio;

	class listener : public observer
	{
	public:
		typedef boost::asio::ip::tcp::socket socket_type;
		OBSERVER_EVENT( accept, socket_type* );
		OBSERVER_EVENT( connect_error, boost::system::error_code& );
		OBSERVER_EVENT( system_error, boost::system::system_error& );
		OBSERVER_EVENT( exception, std::exception& );

		listener() : _acceptor(_io_service_acceptor){}
	public:
		boost::thread_group& threads(){ return _threads; }

		~listener()
		{
			close();
		}

		void close(){
			_threads.interrupt_all();
			boost::system::error_code x;
			_acceptor.close(x);
			_threads.join_all();
		}

		/// <summary>开始监听端口</summary>
		/// <remarks>
		///   本函数会创建 thread_count+1 个线程，一个线程用来监听和接受连接，其他线程处理
		///   网络 IO 操作，所有线程的 io_service 都独立，因此可以保证 IO 的回调都是非并行的。
		/// </remarks>
		void run( const uint16_t port, int thread_count=1, bool hold=false )
		{
			boost::asio::ip::tcp::endpoint _endpoint=asio::ip::tcp::endpoint( asio::ip::tcp::v4(), port );

			_acceptor.open( _endpoint.protocol() );
			_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));	// 允许绑定到同一个端口
			_acceptor.bind( _endpoint );
			_acceptor.listen( 32 ); // Maybe throw

			if( thread_count>0 ){
				for( int i=0; i<thread_count; ++i )
					_threads.create_thread(boost::bind(&listener::work, this ));
			}else{
				boost::asio::ip::tcp::socket *p=new boost::asio::ip::tcp::socket(_io_service_acceptor);
				_acceptor.async_accept( *p, 
					boost::bind(&listener::handle_accept, this, p, boost::asio::placeholders::error)
					);
			}
			if( hold )
				io_service_work( _io_service_acceptor );
			else
				_threads.create_thread( boost::bind(&listener::io_service_work, this, boost::ref(_io_service_acceptor) ) );
		}

		boost::thread_group& thread_group(){ return _threads; }
	private:
		static void check_interrupt( boost::asio::deadline_timer& c )
		{
			if( boost::this_thread::interruption_requested() ) 
				throw std::runtime_error("Thread interrupted.");
			c.expires_from_now( boost::posix_time::seconds(1) );
			c.async_wait( boost::bind( &listener::check_interrupt, boost::ref(c) ) );
		}

		// 工作线程
		void work()
		{
			boost::asio::io_service ios;
			boost::asio::deadline_timer c( ios, boost::posix_time::seconds(1) );
			c.async_wait( boost::bind( &listener::check_interrupt, boost::ref(c) ) );
			boost::asio::ip::tcp::socket* p=new boost::asio::ip::tcp::socket(ios);
			_acceptor.async_accept( *p, 
				boost::bind(&listener::handle_accept, this, p, boost::asio::placeholders::error)
				);

			io_service_work(ios);
		}

		void io_service_work( boost::asio::io_service& ios )
		{
			while( !boost::this_thread::interruption_requested() ){
				try{
					ios.run();
				}catch( boost::system::error_code &e ){
					shot<connect_error>( boost::ref(e) );
				}catch( boost::system::system_error & e ){
					shot<system_error>( boost::ref(e) );
				}catch( std::exception& e ){
					shot<exception>( boost::ref(e) );
				}catch(...){ assert(0); }
				if( ios.stopped() ){
					boost::this_thread::sleep( boost::posix_time::milliseconds(10) );
					ios.reset();
				}
			}
		}
	private:
		void on_connected( boost::asio::ip::tcp::socket* p ){ shot<accept>( p ); }

		// 接受连接，本函数在监听线程中执行
		void handle_accept( boost::asio::ip::tcp::socket* p, const boost::system::error_code& e )
		{
			if( e==boost::asio::error::operation_aborted )
				return;

			boost::asio::ip::tcp::socket* x=new boost::asio::ip::tcp::socket(p->get_io_service());
			_acceptor.async_accept( *x, 
				boost::bind(&listener::handle_accept, this, x, boost::asio::placeholders::error)
				);
			// 发送到处理线程，防止监听线程堵塞
			if (e){
				p->get_io_service().post( boost::bind( &listener::bad_socket, this, p, e ) );
			}else	
				p->get_io_service().post( boost::bind( &listener::on_connected, this, p ) );
		}

		void bad_socket( boost::asio::ip::tcp::socket* p, boost::system::error_code e )
		{
			delete p;
			shot<connect_error>(boost::ref(e));
		}
	private:
		boost::asio::io_service _io_service_acceptor;
		boost::asio::ip::tcp::acceptor _acceptor;
		boost::thread_group _threads;
	};
}; };

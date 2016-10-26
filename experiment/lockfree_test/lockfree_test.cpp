// lockfree_test.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "../../lugce/lockfree/atomic.hpp"
#include "../../lugce/lockfree/stack.hpp"
#include "../../lugce/lockfree/memory_pool.hpp"
#include "../../lugce/lockfree/queue.hpp"

#include <unordered_map>
#include <stack>
#include <boost/thread.hpp>

lugce::lockfree::stack<int>* g_lock_free_stack;
lugce::lockfree::stack<int>* g_lock_free_stack2;
std::stack<int>* g_stack;
//lugce::lockfree::memory_pool<int> mempool;
boost::mutex g_mutex;
DWORD g_std_stack_etime[1000];
DWORD g_lock_free_stack_etime[1000];

volatile int g_temp;

const int run_count=100;
/// 这里使用有锁的堆栈做对比测试
DWORD std_stack_test()
{
	Sleep(100);
	volatile DWORD d=GetTickCount();
	for( int x=0; x<run_count; ++x ){
		for(int i=0; i<1000; ++i){
			boost::mutex::scoped_lock lock( g_mutex );
			g_stack->push( 0xFFFF );
			lock.unlock();
		}

		for(int i=0; i<1000; ++i){
			boost::mutex::scoped_lock lock( g_mutex );
			if( !g_stack->empty() ){
				int o=g_stack->top();
				g_stack->pop();
				g_temp=o;

			}
			lock.unlock();
		}
	}
	return GetTickCount()-d;
}

DWORD lock_free_stack_test()
{
	Sleep(100);
	volatile DWORD d=GetTickCount();
	int o;
	for( int x=0; x<run_count; ++x ){
		for(int i=0; i<1000; ++i){
			g_lock_free_stack->push(0xFFFFFFFF);
		}

		for(int i=0; i<1000; ++i){
			if( g_lock_free_stack->pop(o) ){
				g_temp=o;
				if( o!=0xFFFFFFFF ) std::cout << "bad ";
			}
		}
	}
	return GetTickCount()-d;
}

void exec_std( int idx )
{
	g_std_stack_etime[idx]=std_stack_test();


	//HANDLE hr=GetCurrentThread();
	//PROCESSOR_NUMBER pr;
	//if( GetThreadIdealProcessorEx( hr, &pr ) ){

	//}

}

void exec_lockfree( int idx )
{
	g_lock_free_stack_etime[idx]=lock_free_stack_test();
}

template< typename T >
class lock_like_array
{
public:
	lock_like_array()
	{
		memset( _block, 0, sizeof(_block) );
		memset( _mark, 0, sizeof(_mark) );
		_block[0].data=(intptr_t)(new T[ 4096 ]);	// 12位
		_mark[0]=0x0000;
		_block_size=4096;
	}
	~lock_like_array()
	{
		char idx=0;
		while( _block[idx].data ){
			delete[] (void*)_block[idx].data;
			++idx;
		}
	}

	T& operator[]( int32 index )
	{
		int32 o=index >> 12;
		char bidx=0;
		while( o ){
			++bidx;
			o = o >> 1;
		}
		if( 0==_block[bidx].data ){
			_mark[bidx]=1 << (11 + bidx);
			size_t s=(4096 << (bidx-1));
			T* p= new T[s];
			if( !lugce::lockfree::atomic_compare_and_set( &_block[bidx], o, (intptr_t)p ) )
				delete []p;
		}
		index ^=_mark[bidx];
		T* p=(T*)(_block[bidx].data);
		p+=index;
		return *p;
	}
	const T& operator[]( int32 index ) const
	{
		int32 o=index >> 12;
		char bidx=0;
		while( o ){
			++bidx;
			o= o >> 1;
		}
		index ^=_mark[bidx];
		assert( _block[bidx] );
		T* p=(T*)(_block[bidx]);
		p+=index;
		return *p;
	}
	void push( const T& v )
	{

	}
private:
	int32 _block_size;
	lugce::lockfree::atomic_integer<size_t> _size;
	int32 _mark[20];
	lugce::lockfree::atomic_intptr_t _block[20];
};


using namespace std;
int _tmain(int argc, _TCHAR* argv[])
{
	using namespace std;
	using namespace lugce::lockfree;

	std::cout << "sizeof(int)=" << sizeof(int) << std::endl;
	std::cout << "sizeof(__int32)=" << sizeof(__int32) << std::endl;
	std::cout << "sizeof(long)=" << sizeof(long) << std::endl;

	srand( (unsigned)time( NULL ) );

	queue<int> que;

	atomic_int64_t i64;
	i64.data=5;
	int64 n64=atomic_exchange_and_add( i64, 100 );
	std::cout << n64 << " " << i64.data << std::endl;
	int arr[10];
	for( int i=0; i<10; ++i )
		arr[i]=i;
	lugce::lockfree::atomic_pointer<int> ptr=arr;
	std::cout << *++ptr << std::endl;


	volatile int64 xmy=0;
	int64 my=xmy;
	my++;
	xmy=my;
	std::cout << my << std::endl;


	atomic_int64 v=1;
	//atomic_compare_and_set( &v, 1, v.data+1 );
	++v;
	int64 n=v;
	std::cout << n << " " << v;
	n=atomic_exchange(v, 10);
	std::cout << n << " " << v.data;



	/*typedef std::vector< int, memory_pool<int> > intvec;
	intvec *mi=new intvec();
	mi->push_back(100);
	delete mi;

	atomic_int64 a;
	a.data=78;
	int64 cmp=atomic_swap( &a, 10 );
	std::cout << cmp;

	lock_like_array<int> v;
	DWORD d=GetTickCount();
	for( long i=2022; i<6553700; i++ ){
	v[i]=i;
	}
	for( long i=2022; i<6553700; i++ ){
	if( i!=v[i] ){
	std::cout << "Error :" << i << std::endl;
	}
	}
	std::cout << (GetTickCount()-d) << std::endl;

	atomic_integer<int> ptr;
	ptr=0;
	ptr=( [](const int v){ return v+1; } );*/

	DWORD std_stack_average=0;
	DWORD lock_free_stack_average=0;
	DWORD lock_free_stack2_average=0;
	const int times=5;
	for( int thread_count=4; thread_count < 64; thread_count*=2 ){
		std::cout << "线程数:" << thread_count << std::endl;
		g_lock_free_stack=new lugce::lockfree::stack<int>();
		g_lock_free_stack2=new lugce::lockfree::stack<int>();
		g_stack=new std::stack<int>();
		for( int x=0; x<times; x++ ){
			DWORD etime=0;
			boost::thread_group g;

			memset( g_std_stack_etime, 0, sizeof(g_std_stack_etime) );
			for( int i=0; i<thread_count; i++ ){
				g.create_thread( bind(&exec_std, i) );
			}
			g.join_all();
			//exec(0);
			//exec(1);

			std::cout << std::endl;
			for( int i=0; i<thread_count;i++ ){
				//std::cout << " 线程 " << i << " 用时：" << g_std_stack_etime[i] << std::endl;;
				etime+=g_std_stack_etime[i];
			}
			etime/=thread_count;
			std_stack_average+=etime;
			std::cout << "std::stack 用时:" << etime << std::endl;

			memset( g_lock_free_stack_etime, 0, sizeof(g_lock_free_stack_etime) );

			for( int i=0; i<thread_count; i++ ){
				g.create_thread( bind(&exec_lockfree, i) );
			}
			g.join_all();

			etime=0;
			for( int i=0; i<thread_count;i++ ){
				//std::cout << " 线程 " << i << " 用时：" << g_lock_free_stack_etime[i] << std::endl;;
				etime+=g_lock_free_stack_etime[i];
			}
			etime/=thread_count;
			lock_free_stack_average+=etime;
			std::cout << "lockfree::stack 用时:" << etime << std::endl;

			g.interrupt_all();
		};

		std::cout << "* std::stack 平均用时:" << (std_stack_average / times) << std::endl;
		std::cout << "* lockfree::stack 平均用时:" << (lock_free_stack_average / times) << std::endl;
		if( std_stack_average!=0 )
			std::cout << "差距" << ( (float)std_stack_average / (float)lock_free_stack_average) << std::endl;

		delete g_lock_free_stack;
		delete g_stack;
	};
	//system( "pause" );

	return 0;
}


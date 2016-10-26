#pragma once
#include <windows.h>
#include <map>

namespace lugce
{
	/** 这个类用来同时移动多个窗体
	参数参考 DeferWindowPos 或 SetWindowPos
	用法: <c>
	DeferWindowsPosition def;
	def( ... );
	def( ... );
	或者：
	DeferWindowsPosition()( ... )( ... )( ... );
	*/
	class window_defer
	{
	public:
		window_defer( int offsetX=0, int offsetY=0 ) : _offset_x( offsetX ), _offset_y( offsetY ){}
		/// 允许设置一个偏移量，以便把所有窗体同一一定一个固定值
		void offset( int x, int y ){ _offset_x=x; _offset_y=y; }

		window_defer& operator()( 
			HWND hWnd,
			HWND hWndInsertAfter,
			int x,
			int y,
			int cx=-1,
			int cy=-1,
			UINT uFlags=SWP_NOACTIVATE )
		{
			if( hWnd==0 ) return *this;
			if( hWndInsertAfter==0 ) uFlags |=SWP_NOZORDER;
			if( -1==cx && -1==cy ) uFlags |= SWP_NOSIZE;
			DefInfo info={ hWndInsertAfter, x, y, cx, cy, uFlags };
			_infos.insert( std::make_pair(hWnd, info) );
			return *this;
		}

		void set_size( HWND hWnd, const long cx, const long cy )
		{
			info_map_t::iterator i=_infos.find( hWnd );
			if(i==_infos.end()){
				DefInfo info={ 0, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE };
				_infos.insert( std::make_pair(hWnd, info) );
			}else{
				i->second.cx=cx; i->second.cy=cy;
				i->second.uFlags= i->second.uFlags & ~SWP_NOSIZE;
			}
		}

		void set_position( HWND hWnd, const long x, const long y )
		{
			info_map_t::iterator i=_infos.find( hWnd );
			if(i==_infos.end()){
				DefInfo info={ 0, x+_offset_x, y+_offset_y, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE };
				_infos.insert( std::make_pair(hWnd, info) );
			}else{
				i->second.x=x+_offset_x; i->second.y=y+_offset_y;
				i->second.uFlags= i->second.uFlags & ~SWP_NOMOVE;
			}
		}

		void move()
		{
			if( _infos.empty() ) return;
			HDWP hdwp=BeginDeferWindowPos( _infos.size() );
			for( info_map_t::const_iterator i=_infos.begin(); i!=_infos.end(); ++i ){
				DeferWindowPos( hdwp, 
					i->first, 
					i->second.hWndInsertAfter, 
					i->second.x+_offset_x,
					i->second.y+_offset_y, 
					i->second.cx, 
					i->second.cy, 
					i->second.uFlags );
			}
			EndDeferWindowPos( hdwp );
			_infos.clear();
		}

		~window_defer(){ move(); }
	private:
		int _offset_x;
		int _offset_y;
		struct DefInfo
		{
			HWND hWndInsertAfter;
			int x;
			int y;
			int cx;
			int cy;
			UINT uFlags;
		};
		typedef std::map<HWND, DefInfo> info_map_t;
		info_map_t _infos;
	};
};
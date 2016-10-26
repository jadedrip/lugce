#pragma once

#include "panel.hpp"
#include "defer.hpp"

namespace lugce
{
	namespace makeup
	{
		/// 定义了一个单一面板，用来将窗体转换为面板
		class wnd_panel : public panel
		{
		public:
			wnd_panel( HWND wnd=NULL, window_defer * defer=NULL ) : _wnd(wnd), _defer(defer){ _active=true; }
		public:
			void set_hwnd( HWND wnd ){ _wnd=wnd; }
			HWND get_hwnd() const{ return _wnd; }

			virtual void set_active( bool v )
			{
				ShowWindow( _wnd, v ? SW_SHOW : SW_HIDE );
				_active=v;
			}

			virtual bool get_active() const
			{ 
				if( !_wnd ) return false;
				return _active;
			}

			virtual size get_size() const
			{
				size sz={ 0, 0 };
				if( _wnd && IsWindow( _wnd )  ){
					DWORD style=(DWORD)GetWindowLong(_wnd, GWL_STYLE);
					if( style & WS_VISIBLE ){
						RECT rc;
						GetWindowRect( _wnd, &rc );
						sz.cx=rc.right-rc.left;
						sz.cy=rc.bottom-rc.top;
					}
				}
				return sz;
			}

			virtual void set_size( const long sx, const long sy )
			{
				if( _defer )
					_defer->set_size( _wnd, sx, sy );
				else
					::SetWindowPos( _wnd, 0, 0, 0, sx, sy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREPOSITION );
			}

			virtual void draw_at( const long x, const long y )
			{
				if( _wnd==0 ) return;
				if( _defer )
					_defer->set_position( _wnd, x, y );
				else
					::SetWindowPos( _wnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			}
			virtual pointer clone() const { return pointer( new wnd_panel(*this) ); }
		private:
			HWND _wnd;
			window_defer *_defer;
		};

		/// 定义了一个 Group 组合面板
		class group_panel : public panel
		{
		public:
			group_panel( HWND grp, const panel& p ) : _wnd(grp), _inwnd(p.clone())
			{
				_active=true;
				_inwnd=pointer( new margin( 5, 20, 5, 5, p ) );
			}
		public:
			margin& get_inside(){ return * boost::dynamic_pointer_cast<margin>(_inwnd); }

			virtual void set_active( bool v )
			{
				ShowWindow( _wnd, v ? SW_SHOW : SW_HIDE );
				_inwnd->set_active(v);
			}

			virtual bool get_active() const
			{ 
				//if( !_wnd ) return false;
				return _wnd!=0;
// 				BOOL b=::IsWindowVisible( _wnd );
// 				if(!b){
// 					TCHAR x[1024];
// 					::GetWindowText( _wnd, x, 1024 );
// 					return b;
// 				}
// 
// 				return b;
			}

			virtual size get_size() const
			{
				size sz={ 0, 0 };
				if( _wnd && IsWindow( _wnd )  ){
					DWORD style=(DWORD)GetWindowLong(_wnd, GWL_STYLE);
					if( style & WS_VISIBLE ){
						RECT rc;
						GetWindowRect( _wnd, &rc );
						sz.cx=rc.right-rc.left;
						sz.cy=rc.bottom-rc.top;
					}
				}
				return sz;
			}

			virtual void set_size( const long sx, const long sy )
			{
				::SetWindowPos( _wnd, 0, 0, 0, sx, sy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE );
				_inwnd->set_size( sx, sy );
			}

			virtual void draw_at( const long x, const long y )
			{
				if( _wnd==0 ) return;
				::SetWindowPos( _wnd, 0, x, y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
				_inwnd->draw_at( x, y );
			}
			virtual pointer clone() const { return shared_ptr<panel>( new group_panel(*this) ); }
		private:
			pointer _inwnd;
			HWND _wnd;
		};
	};
};

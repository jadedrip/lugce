#pragma once
#include <boost/shared_ptr.hpp>
#include <vector>
#include "panel.hpp"

namespace lugce
{

	namespace makeup
	{
		/// 用来放置单个面板，它保持大小漂浮在面板中
		class dock : public panel
		{
		public:
			enum dock_e{ dock_none, dock_center=0, dock_left=1, dock_right=2, dock_top=4, dock_bottom=8  };
			dock( dock_e dock=dock_center ): _dock(dock){};
			dock( const panel& p, dock_e dock=dock_center )
				: _dock(dock)
			{ 
				_panel=p.clone(); _sz=p.get_size();
			}
		public:
			void panel( const panel& p )
			{
				_panel=p.clone(); _sz=p.get_size();
			}

			panel::pointer panel() const{ return _panel; }

			virtual void set_size( const long sx, const long sy )
			{
				_sz.cx=sx; _sz.cy=sy;
				if( _panel ){
					size x=_panel->get_size();
					if( (_dock & dock_left) && (_dock & dock_right) )
						x.cx=sx;
					if( (_dock & dock_top) && (_dock & dock_bottom) )
						x.cy=sy;
					_panel->set_size(x.cx, x.cy);
				}
			}

			virtual void draw_at( int x, int y )
			{
				assert( _panel );
				if( !_panel ) return;
				size psz=_panel->get_size();
				point pt={0,0};
				if(_dock & dock_left){
					pt.x=0;
				}else if(_dock & dock_right){
					pt.x=_sz.cx-psz.cx;
				}else{
					pt.x=(_sz.cx-psz.cx) >> 1;
				}

				if(_dock & dock_top){
					pt.y=0;
				}else if(_dock & dock_bottom){
					pt.y=_sz.cy-psz.cy;
				}else{
					pt.y=(_sz.cy-psz.cy) >> 1;
				}

				_panel->draw_at( x+pt.x, y+pt.y );
			}

			virtual void set_active( bool v=true ){ _active=v; _panel->set_active(v); }
			virtual pointer clone() const { return pointer( new dock(*this) ); }
		protected:
			panel::pointer _panel;
			dock_e _dock;
		};

		/// 使用固定的空白包围一个面板
		class margin : public panel
		{
		public:
			margin( int margin_left, int margin_top, int margin_right, int margin_bottom, const panel& p ) 
				: _margin_left( margin_left ), _margin_right( margin_right ), _margin_top( margin_top ), _margin_bottom( margin_bottom )
			{
				_panel=p.clone();
			}
			margin( int margin, const panel& p )
				: _margin_left( margin ), _margin_right( margin ), _margin_top( margin ), _margin_bottom( margin ){ _panel=p.clone(); }
			margin( int margin_x, int margin_y, const panel& p )
				: _margin_left( margin_x ), _margin_right( margin_x ), _margin_top( margin_y ), _margin_bottom( margin_y ){ _panel=p.clone(); }
		public:
			virtual void set_active( bool v=true ){ _active=v; _panel->set_active(v); }

			virtual size get_size() const
			{
				size sz={0,0};
				if( _panel ) sz=_panel->get_size();
				sz.cx+=_margin_left+_margin_right;
				sz.cy+=_margin_top+_margin_bottom;
				return sz;
			}

			virtual void set_size( const long sx, const long sy )
			{
				if( _panel )
					_panel->set_size( sx - _margin_left - _margin_right, sy - _margin_top - _margin_bottom );
			}

			/// 移动到指定位置，并且设置为指定大小
			virtual void draw_at( const long x, const long y )
			{
				if( _panel )
					_panel->draw_at( x+_margin_left, y+_margin_top );
			}
			virtual pointer clone() const { return pointer( new margin(*this) ); }
		protected:
			int _margin_left;
			int _margin_right;
			int _margin_top;
			int _margin_bottom;
			panel::pointer _panel;
		};
	};
};

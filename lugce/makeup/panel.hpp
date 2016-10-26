#pragma once
#include <cstdint>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#ifdef _WIN32
#	include <WinDef.h>
#else
typedef struct tagSIZE
{
	LONG        cx;
	LONG        cy;
} SIZE;
#endif

namespace lugce
{
	using boost::shared_ptr;
	namespace makeup
	{
		typedef SIZE size;
		typedef POINT point;
		//	enum { SIZE_RESIZING=0, SIZE_FIXED_X=1, SIZE_FIXED_Y=2, SIZE_FIXED_XY=3, POS_CENTER_X=4, POS_CENTER_Y=8 };

		/// 定义面板
		class panel 
		{
			friend class container;
		public:
			typedef boost::shared_ptr<panel> pointer;
			panel( const size sz ) : _sz(sz){ _data=0; _active=true; }
			panel( long cx=0, long cy=0 ){ _sz.cx=cx; _sz.cy=cy; _data=0; _active=true; }
			virtual ~panel(){}
			
			virtual void set_size( const long sx, const long sy )
			{
				_sz.cx=sx; _sz.cy=sy;
			}
			virtual size get_size() const{ return _sz; }
			virtual void set_data( const intptr_t d ){ _data=d; }
			virtual const intptr_t get_data() const{ return _data; }
			/// 是否可用
			virtual void set_active( bool v=true ){ _active=v; }
			virtual bool get_active() const{ return _active; }

			virtual void draw_at( const long x, const long y ){}
		public:
			virtual pointer clone() const { return pointer( new panel(*this) ); }
		protected:
			intptr_t _data;
			size _sz;
			bool	_active;
		};

		/// 一个横向或者纵向平铺的面板容器，容器是可以嵌套的.
		/// 一个容器中允许有一个可变面板，这个面板的大小由其他面板决定
		class container : public panel
		{
		protected:		
			virtual pointer clone() const { return pointer( new container(*this) ); }

			typedef std::vector< shared_ptr<panel> > panels_t;
			// 
			panels_t _panels;
			boost::shared_ptr<panel> _sizeable;
			bool _horiz;
		public:
			container( bool horiz=true ) : _horiz( horiz )
			{
			}
			void set_horiz( bool horiz=true ){ _horiz=horiz; }
		public:
			/// 添加一个面板
			container& operator () ( const panel& pl, bool sizeable=false )
			{
				shared_ptr< panel > p=pl.clone();
				_panels.push_back( p );
				if( sizeable ) _sizeable=p;
				size sz=pl.get_size();
				if( _horiz ){
					if( sz.cy > _sz.cy ) _sz.cy=sz.cy;
					_sz.cx+=sz.cx;
				}else{
					if( sz.cx > _sz.cx ) _sz.cx=sz.cx;
					_sz.cy+=sz.cy;
				}
				return *this;
			}
		public:
			virtual void draw_at( const long x, const long y )
			{
				long ox=x; long oy=y;
				for( panels_t::const_iterator i=_panels.begin(); i!=_panels.end(); ++i ){
					const shared_ptr< panel >& p=*i;
					if( !p->get_active() ) continue;
					p->draw_at( ox, oy );
					if(_horiz)
						ox+=p->get_size().cx;
					else
						oy+=p->get_size().cy;
				}
			}

			virtual void set_size( const long sx, const long sy )
			{
				_sz.cx=sx; _sz.cy=sy;
				// 计算的策略：首先确定所有固定面板的大小，然后所有活动面板为剩余的空间
				int s=0;
				// 计算所有 固定面板的总大小
				for( panels_t::const_iterator i=_panels.begin(); i!=_panels.end(); ++i ){
					const shared_ptr< panel >& p=*i;
					size o=p->get_size();
					if( _horiz )
						p->set_size( o.cx, sy );
					else
						p->set_size( sx, o.cy );

					if( !p->get_active() ) continue;	// 不活跃的面板不进入计算
					if( p!=_sizeable ){
						s+=_horiz ? o.cx : o.cy;
					}
				}

				// 可变块大小
				if( _sizeable ){
					if( _horiz )
						_sizeable->set_size( _sz.cx-s, _sizeable->get_size().cy );
					else
						_sizeable->set_size(  _sizeable->get_size().cx, _sz.cy-s );
				}
			}
		};
	};
};

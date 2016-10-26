#pragma once

#include "defer.hpp"
#include "panel.hpp"
#include "simple_wnd_panel.hpp"

namespace Makeup
{
	class SplitterPanel : public Panel
	{
		INHERIT_FROM_PANEL( SplitterPanel )
	public:
		SplitterPanel()
		{
			_parent=0;
			_splitterPoint=_frontPanelMin=_behindPanelMin=0;	

			_splitterWidth=3;
			_horizontal=FALSE;

			_isDown=FALSE;
			_lastSplitter=-1;

			_redrawAfterResize=FALSE;

			memset( &_wndRc, 0, sizeof(RECT) );
		}
		virtual ~SplitterPanel(){}
	public:
		LRESULT MsgProcess(UINT message, WPARAM wParam, LPARAM lParam)
		{
			bool ret=FALSE;
			if( !IsValidate() )	return 0;

			switch ( message )
			{
			case WM_MOUSEMOVE:
				return OnMouseMove( wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)) );
			case WM_LBUTTONDOWN:
				return OnLButtonDown( wParam, CPoint((short)LOWORD(lParam), (short)HIWORD(lParam)) );
			case WM_LBUTTONUP:
			case WM_KILLFOCUS:
			case WM_CAPTURECHANGED:
			case WM_NCACTIVATE:
				if( _isDown ){			
					_isDown=FALSE;
					CPoint point;
					GetCursorPos( &point );
					::ScreenToClient( _parent, &point );

					if( _horizontal ){
						_splitterPoint=NumInside( point.y-_wndRc.top, _frontPanelMin, _wndRc.Height()-_behindPanelMin );
					}else{
						_splitterPoint=NumInside( point.x-_wndRc.left, _frontPanelMin, _wndRc.Width()-_behindPanelMin );
					}

					::ReleaseCapture();
					Refresh();
					ret=TRUE;
				}
				if( _lastSplitter > 0 ){
					DrawSplitterAt( _lastSplitter );
					_lastSplitter = -1;
				}
				return ret;
			};

			return 0;
		}
	public:
		void Initialize( HWND frontPanel, HWND behindPanel, HWND parent, bool horizontal, UINT splitterWidth=3, bool changeFrontOnSizing=TRUE )
		{	
			Initialize( SimpleWndPanel(frontPanel).CreatePointer(), SimpleWndPanel(behindPanel).CreatePointer(), parent, horizontal, splitterWidth );
		}

		void Initialize( Panel::Pointer frontPanel, Panel::Pointer behindPanel, HWND parent, bool horizontal, UINT splitterWidth=3, bool changeFrontOnSizing=TRUE )
		{
			assert( frontPanel && behindPanel );
			_frontPanel=frontPanel; _behindPanel=behindPanel;
			_horizontal=horizontal;
			_splitterWidth=splitterWidth;
			_parent=parent;
			_changeFront=changeFrontOnSizing;
		}

		void SetRedrawAfterResize( bool b ){ _redrawAfterResize=b; }

		void SetPanelMin( UINT frontMin, UINT behindMin )
		{
			_frontPanelMin=frontMin;
			_behindPanelMin=behindMin;
		}

		// 设定分割线位置
		void SetSplitterPos( UINT pos )
		{
			_splitterPoint=pos;
			Refresh();
		}

		int GetFrontMin() const{ return _frontPanelMin; }
		int GetBehindMin() const{ return _frontPanelMin; }
		UINT GetSplitterWidth() const{ return _splitterWidth; }

		bool IsValidate()
		{	// 检查必要数据是否完备
			if( 0==_frontPanel || 0==_behindPanel ) return FALSE;
			if( _parent==NULL ) return FALSE;
			if( (_wndRc.right==_wndRc.left) || (_wndRc.bottom==_wndRc.top)  ) return FALSE;
			return TRUE;
		}
	public:	
		virtual SIZE GetSize() const
		{
			SIZE sz={ _wndRc.right-_wndRc.left, _wndRc.bottom-_wndRc.top };
			return sz;
		}

		virtual void MoveTo( int x, int y, int cx, int cy, DeferWindowsPosition *defer=NULL )
		{
			// 如果只有一个面板可见，扩展可见面板
			if( !_behindPanel || !_frontPanel ) return;

			if( !_behindPanel->GetVisible() ){
				if( _frontPanel->GetVisible() ) 
					_frontPanel->MoveTo( x, y, cx, cy, defer );
				return;
			}else if( !_frontPanel->GetVisible() ){
				_behindPanel->MoveTo( x, y, cx, cy, defer );
			}

			// 如果修改大小时需要更改第一个面板，计算分割线新位置
			if( !_changeFront && !_wndRc.IsRectEmpty() ){
				if( _horizontal ){
					_splitterPoint+=cy - _wndRc.bottom + _wndRc.top;
				}else{
					_splitterPoint+=cx - _wndRc.right + _wndRc.left;
				}
			}

			_wndRc.left=x;
			_wndRc.top=y;
			_wndRc.right=x+cx;
			_wndRc.bottom=y+cy;

			Refresh( defer );
		}

		void Refresh( DeferWindowsPosition *defer=NULL )
		{	// 
			if( !IsValidate() ) return;

			int cx=_wndRc.right-_wndRc.left;
			int cy=_wndRc.bottom-_wndRc.top;

			DeferWindowsPosition *pdefer= defer ? defer : (new DeferWindowsPosition());

			int behindPoint;
			if( _horizontal ){	// 上下两块
				_frontPanel->MoveTo( _wndRc.left, _wndRc.top, cx, _splitterPoint, pdefer );
				behindPoint=_wndRc.top+_splitterPoint+_splitterWidth;
				_behindPanel->MoveTo( _wndRc.left, behindPoint, cx, cy-_splitterPoint-_splitterWidth, pdefer );
			}else{
				_frontPanel->MoveTo( _wndRc.left, _wndRc.top, _splitterPoint, cy, pdefer );
				behindPoint=_wndRc.left+_splitterPoint+_splitterWidth;
				_behindPanel->MoveTo( behindPoint, _wndRc.top, cx-_splitterPoint-_splitterWidth, cy, pdefer );
			}
			if( defer==NULL ) delete pdefer;
			if( _redrawAfterResize ) ::InvalidateRect( _parent, NULL, TRUE );
		}
	protected:
		void DrawSplitter( const CPoint& point )
		{
			if( _lastSplitter > 0 )	DrawSplitterAt( _lastSplitter );

			if( _horizontal )
				_lastSplitter = NumInside( point.y-_wndRc.top, _frontPanelMin, _wndRc.Height()-_behindPanelMin );
			else
				_lastSplitter = NumInside( point.x-_wndRc.left, _frontPanelMin, _wndRc.Width()-_behindPanelMin );
			
			DrawSplitterAt( _lastSplitter );
		}

		inline void DrawLine( HDC dc, LONG from_x, LONG from_y, LONG to_x, LONG to_y )
		{
			::MoveToEx(dc, from_x, from_y, NULL);
			::LineTo(dc, to_x, to_y);
		}

		void DrawSplitterAt( UINT offset )
		{
			if( NULL==_frontPanel || NULL==_behindPanel ) return;

			DWORD dwdStyles[] = {1, 1};
			LOGBRUSH lb;
			lb.lbColor = RGB(0,0,0);
			lb.lbStyle = BS_SOLID;

			HDC wDC = ::GetWindowDC( _parent );

			HPEN pen = ExtCreatePen(PS_GEOMETRIC|PS_USERSTYLE|PS_ENDCAP_FLAT, 1, &lb, 1, dwdStyles);
			::SelectObject(wDC, pen);
			int rop=::SetROP2(wDC, R2_NOTXORPEN);

			UINT i = 0;	

			// 以交错的方式绘制线
			// 计算客户区在窗体的位置
			CPoint pt(0,0);
			::ClientToScreen( _parent, &pt );
			CRect rc;
			::GetWindowRect( _parent, &rc );
			int o=pt.y-rc.top;
	
			if(_horizontal){
				for(i = offset; i < offset + _splitterWidth ; i++)
					DrawLine( wDC, _wndRc.left + (i&1), o + _wndRc.top+i, _wndRc.right - 1, o + _wndRc.top+i );
			}else{
					for(i = offset; i < offset + _splitterWidth; i++)
					DrawLine( wDC, _wndRc.left+i, o + _wndRc.top + (i&1), _wndRc.left+i, o + _wndRc.bottom - 1 );
			}
			::SetROP2(wDC, rop);
			::DeleteObject(pen);
			::ReleaseDC( _parent, wDC );
		}

		RECT GetSpliteRc()
		{	// 返回分割线所在的矩形
			RECT rc;
			memcpy( &rc, &_wndRc, sizeof(RECT) );
			if( _horizontal ){
				rc.top+=_splitterPoint;
				rc.bottom=rc.top+_splitterWidth;
			}else{
				rc.left+=_splitterPoint;
				rc.right=rc.left+_splitterWidth;
			}
			return rc;
		}

		// 保证输入值在最大最小范围内
		int NumInside( int num, int min, int max )
		{	
			if( num < min ) 
				return min;
			else if( num > max ) 
				return max;
			return num;
		}
	protected:
		bool OnMouseMove( UINT nFlags, CPoint point )
		{
			bool ret=FALSE;
			CRect spliteRc=GetSpliteRc();
			if(spliteRc.PtInRect(point) || _isDown){
				::SetCursor( ::LoadCursor( NULL, _horizontal ? IDC_SIZENS : IDC_SIZEWE ) );
				ret=TRUE;
			}
			if( _isDown && (nFlags & MK_LBUTTON) )
				DrawSplitter( point );
			return ret;
		}

		bool OnLButtonDown( UINT nFlags, CPoint point )
		{
			CRect spliteRc=GetSpliteRc();

			if( spliteRc.PtInRect(point) ){
				_isDown=TRUE;
				DrawSplitter( point );
				::SetCapture( _parent );
				return TRUE;
			}
			return FALSE;
		}
	protected:
		Panel::Pointer _frontPanel;
		Panel::Pointer _behindPanel;
		HWND _parent;

		bool _changeFront;
		// 面板最小值
		int  _frontPanelMin;	
		int _behindPanelMin;

		// 分割线所在位置
		int _splitterPoint;	
		// 分割线宽度
		UINT _splitterWidth;	

		// 整体大小
		CRect _wndRc;		
		bool _horizontal;

		bool _isDown;
		int _lastSplitter;
		bool _redrawAfterResize;
	};

};
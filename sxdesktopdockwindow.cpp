#include "stdafx.h"
#include "sxdesktopdockwindow.h"
#include "kpopupwindow_p.h"
#include "kxmlui.h"
#include "ktheme.h"
#include "kutility.h"

KX_WINDOW_CREATOR_GLOBAL_STATIC(SxDesktopDockWindow);

#define DOCK_PANNEL_PIXELS			(5)	
#define DOCK_AREA_PIXELS			(10)

class SxDesktopDockWindowPrivate : public KPopupWindowPrivate
{
	Q_DECLARE_PUBLIC(SxDesktopDockWindow)
public:
	enum DockType{eDockNone = 0, eDockLeft, eDockRight, eDockTop, eMinimized = 0xFF00};
public:
	QFlags<DockType> m_docktype;
	QPropertyAnimation m_animation;
	QBasicTimer m_timerExpand;

public:
	SxDesktopDockWindowPrivate()
	{
		m_docktype = eDockNone;
	}

	DockType PtInDockArea(const QPoint& pt, QRect *pRt)
	{
		Q_Q(SxDesktopDockWindow);
		QRect screenRt = KUtility::virtualScreenRect();
		QRect workArea = KUtility::desktopWorkArea();
		QMargins m = q->maskMargins();
		if(screenRt.top() == workArea.top())
		{
			QRect hitRt(workArea.left(), workArea.top(), workArea.width(), DOCK_AREA_PIXELS);
			QPoint ptNew = pt + QPoint(0, m.top());
			if(hitRt.contains(ptNew))
			{
				if(pRt)
				{
					*pRt = hitRt;
				}
				return eDockTop;
			}
		}
		if(screenRt.left() == workArea.left())
		{
			QRect hitRt(workArea.left(), workArea.top(), DOCK_AREA_PIXELS, workArea.height());
			QPoint ptNew = pt + QPoint(m.left(), 0);
			if(hitRt.contains(pt))
			{
				if(pRt)
				{
					*pRt = hitRt;
				}
				return eDockLeft;
			}
		}
		if(screenRt.right() == workArea.right())
		{
			QRect hitRt(workArea.left() + workArea.width() - DOCK_AREA_PIXELS, workArea.top(), DOCK_AREA_PIXELS, workArea.height());
			QPoint ptNew = pt + QPoint(q->width(), 0) - QPoint(m.right(), 0);
			if(hitRt.contains(ptNew))
			{
				if(pRt)
				{
					*pRt = hitRt;
				}
				return eDockRight;
			}
		}
		return eDockNone;
	}

	void init()
	{
		Q_Q(SxDesktopDockWindow);


		m_animation.setTargetObject(q);
		m_animation.setPropertyName("pos");

		QObject::connect(&m_animation, SIGNAL(finished()), q, SLOT(on_animation_finished()));
	}

	DockType setDockActiveOnMoving(QPoint shouldPt)
	{
		Q_Q(SxDesktopDockWindow);
		QRect rt;
		DockType dt = PtInDockArea(shouldPt, &rt);
		QMargins m = q->maskMargins();
		if(dt == eDockLeft)
		{
			QRect geom = q->geometry();
			q->move(rt.left() - m.left(), geom.top());
		}
		else if(dt == eDockRight)
		{
			QRect geom = q->geometry();
			q->move(rt.left() + rt.width() - geom.width() + m.right(), geom.top());
		}
		else if(dt == eDockTop)
		{
			QRect geom = q->geometry();
			q->move(geom.left(), rt.top() - m.top());
		}

		return dt;
	}

	bool isCollapsed()
	{
		Q_Q(SxDesktopDockWindow);
		QRect geom = q->geometry();
		QRect workArea = KUtility::desktopWorkArea();
		QMargins m = q->maskMargins();
		if(geom.left() + geom.width() - DOCK_PANNEL_PIXELS - m.right() == workArea.left())
		{
			m_docktype = eDockLeft;
			return true;
		}
		if(geom.top() + geom.height() - DOCK_PANNEL_PIXELS - m.bottom() == workArea.top())
		{
			m_docktype = eDockTop;
			return true;
		}
		if(geom.left() + DOCK_PANNEL_PIXELS + m.left() == workArea.left() + workArea.width())
		{
			m_docktype = eDockRight;
			return true;
		}
		return false;
	}

	bool isExpanded()
	{
		Q_Q(SxDesktopDockWindow);
		QRect geom = q->geometry();
		QRect workArea = KUtility::desktopWorkArea();
		QMargins m = q->maskMargins();
		if(geom.left() + m.left() == workArea.left())
		{
			m_docktype = eDockLeft;
			return true;
		}
		if(geom.top() + m.top() == workArea.top())
		{
			m_docktype = eDockTop;
			return true;
		}
		if(geom.left() + geom.width() - m.right() == workArea.left() + workArea.width())
		{
			m_docktype = eDockRight;
			return true;
		}
		return false;
	}

	void setExpandingFromCollapsed()
	{
		Q_Q(SxDesktopDockWindow);
		if(m_docktype == eDockLeft)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt + QPoint(geom.width() - DOCK_PANNEL_PIXELS - m.left() - m.right(), 0);
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
		else if(m_docktype == eDockTop)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt + QPoint(0, geom.height() - DOCK_PANNEL_PIXELS - m.top() - m.bottom());
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
		else if(m_docktype == eDockRight)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt - QPoint(geom.width() - DOCK_PANNEL_PIXELS - m.right() - m.left(), 0);
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
	}

	bool isCursorHoverOnWindow()
	{
		Q_Q(SxDesktopDockWindow);
		POINT pt;
		GetCursorPos(&pt);
		HWND hwnd = WindowFromPoint(pt);
		HWND hWndRoot = GetAncestor(hwnd, GA_ROOTOWNER);
		RECT rt;
		GetWindowRect(hWndRoot, &rt);
		return q->winId() == hWndRoot && PtInRect(&rt, pt);
	}

	void setCollapsingFromExpanded()
	{
		Q_Q(SxDesktopDockWindow);

		if(isCursorHoverOnWindow())
			return;

		m_timerExpand.stop();
		if(m_docktype == eDockLeft)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt - QPoint(geom.width() - DOCK_PANNEL_PIXELS - m.left() - m.right(), 0);
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
		else if(m_docktype == eDockTop)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt - QPoint(0, geom.height() - DOCK_PANNEL_PIXELS - m.top() - m.bottom());
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
		else if(m_docktype == eDockRight)
		{
			QRect geom = q->geometry();
			QMargins m = q->maskMargins();
			QPoint startPt = geom.topLeft();
			QPoint endPt = startPt + QPoint(geom.width() - DOCK_PANNEL_PIXELS - m.left() - m.right(), 0);
			m_animation.setStartValue(startPt);
			m_animation.setEndValue(endPt);
			m_animation.start();
		}
	}

	void setCollapsingTimeOutFromExpanded()
	{
		Q_Q(SxDesktopDockWindow);
		if(m_animation.state() == QAbstractAnimation::Running)
			return;
		if(m_docktype == eDockTop || m_docktype == eDockLeft || m_docktype == eDockRight)
		{
			m_timerExpand.start(1000, q);
		}
		else
		{
			m_timerExpand.stop();
		}
	}

	void setDockActiveOnRestore()
	{
		Q_Q(SxDesktopDockWindow);
		QRect geom = q->geometry();
		if(isExpanded())
		{
			if(setDockActiveOnMoving(geom.topLeft()))
			{
				setCollapsingTimeOutFromExpanded();
			}
		}
		else if(isCollapsed())
		{
			setExpandingFromCollapsed();
		}
	}

	void setDockActiveOnShowHide(bool bVisible)
	{
		Q_Q(SxDesktopDockWindow);
		QRect geom = q->geometry();
		if(bVisible)
		{
			if(isExpanded())
			{
				if(setDockActiveOnMoving(geom.topLeft()))
				{
					setCollapsingTimeOutFromExpanded();
				}
			}
			else if(isCollapsed())
			{
				setExpandingFromCollapsed();
			}
		}
		else
		{

		}
	}

	bool winEvent(MSG *pMsg, long *result)
	{
		Q_Q(SxDesktopDockWindow);
		if(pMsg->message == WM_LBUTTONDOWN || pMsg->message == WM_LBUTTONUP)
		{
			if(m_animation.state() == QAbstractAnimation::Running)
			{
				*result = 0;
				return true;
			}
		}
		else if(pMsg->message == WM_NCLBUTTONDOWN || pMsg->message == WM_NCLBUTTONUP)
		{
			if(m_animation.state() == QAbstractAnimation::Running)
			{
				*result = 0;
				return true;
			}
		}
		else if(pMsg->message == WM_MOUSEMOVE)
		{
			if(m_animation.state() == QAbstractAnimation::Running)
			{
				//动画状态
				*result = 0;
				return true;
			}
			if(bDragWindow)
			{
				/*拖动窗口状态*/
				QPoint pt(GET_X_LPARAM(pMsg->lParam), GET_Y_LPARAM(pMsg->lParam));
				QPoint screenPt = q->mapToGlobal(pt);
				QPoint shouldPt = screenPt - dragPosition;
				DockType dt = setDockActiveOnMoving(shouldPt);
				if(dt != m_docktype)
				{
					SetWindowPos(q->winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
					m_docktype = dt;
				}
				if(m_docktype != eDockNone)
				{
					*result = 0;
					return true;
				}
			}
			else if(pMsg->wParam == 0) 
			{
				if(isCollapsed())
				{
					//当前窗口正在折叠状态，现正由折叠到展开。
					setExpandingFromCollapsed();
					return true;
				}
				else if(isExpanded())
				{
					//当前正处于停靠状态。
					setCollapsingTimeOutFromExpanded();
					return true;
				}
			}
		}
		else if(pMsg->message == WM_SHOWWINDOW)
		{
			setDockActiveOnShowHide(pMsg->wParam);
		}
		else if(pMsg->message == WM_SIZE)
		{
			if(pMsg->wParam == SIZE_RESTORED)
			{
				if(m_docktype.testFlag(eMinimized))
				{
					m_docktype &= 0xFF;
					setDockActiveOnRestore();
				}
			}
			else if(pMsg->wParam == SIZE_MINIMIZED)
			{
				m_docktype |= eMinimized;
				m_timerExpand.stop();
			}
		}
		else if(pMsg->message == WM_NCHITTEST)
		{
			if(isCollapsed())
			{
				*result = HTCLIENT;
				return true;
			}
		}
		return false;
	}
};

SxDesktopDockWindow::SxDesktopDockWindow( QWidget *parent/*=NULL*/ )
: KPopupWindow(*new SxDesktopDockWindowPrivate, parent)
{
	Q_D(SxDesktopDockWindow);
	d->init();
}

SxDesktopDockWindow::~SxDesktopDockWindow()
{

}

bool SxDesktopDockWindow::winEvent( MSG * pMsg, long * result )
{
	Q_D(SxDesktopDockWindow);

	if(d->winEvent(pMsg, result))
		return true;
	return __super::winEvent(pMsg, result);
}

bool SxDesktopDockWindow::event(QEvent *e)
{
	Q_D(SxDesktopDockWindow);
	if(e->type() == QEvent::Timer)
	{
		QTimerEvent *te = (QTimerEvent*)e;
		quint32 tid = te->timerId();
		if(tid == d->m_timerExpand.timerId())
		{
			d->setCollapsingFromExpanded();
		}
	}
	return __super::event(e);
}

void SxDesktopDockWindow::on_animation_finished()
{
	Q_D(SxDesktopDockWindow);
	if(d->isCollapsed())
	{
		SetWindowPos(winId(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
	else
	{
		SetWindowPos(winId(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
	}
}

void SxDesktopDockWindow::saveLastPosition( const QString& file )
{
	QSettings inicfg(file, QSettings::IniFormat);
	QRect geom = geometry();
	inicfg.setValue("DesktopDockPos/geometry", geom);
}

void SxDesktopDockWindow::restoreLastPosition( const QString& file )
{
	QSettings inicfg(file, QSettings::IniFormat);
	QVariant vgeom = inicfg.value("DesktopDockPos/geometry");
	QRect geom = vgeom.toRect();
	if(!geom.isEmpty())
	{
		Q_D(SxDesktopDockWindow);
		setGeometry(geom);
		d->setDockActiveOnRestore();
	}
}
#pragma once;

#include "kpopupwindow.h"

class SxDesktopDockWindowPrivate;

class SxDesktopDockWindow : public KPopupWindow
{
	Q_OBJECT
	Q_DECLARE_PRIVATE(SxDesktopDockWindow)
public:

public:
	explicit SxDesktopDockWindow(QWidget *parent=NULL);
	~SxDesktopDockWindow();

	void saveLastPosition(const QString& file);
	void restoreLastPosition(const QString& file);
protected:
	bool winEvent(MSG * pMsg, long * result);
	bool event(QEvent * e);

private slots:
	void on_animation_finished();
};
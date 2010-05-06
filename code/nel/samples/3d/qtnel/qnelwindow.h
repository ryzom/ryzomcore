#ifndef NL_QNELWINDOW_H
#define NL_QNELWINDOW_H

///-----------------------------------------------------------------------------
/// C++ includes
#include <iostream>
#include <string>
#include <map>
#include <cstdlib>
#include <cmath>

///-----------------------------------------------------------------------------
/// Windowing Toolkit includes.
#include <QMainWindow>

///-----------------------------------------------------------------------------
/// The CameraTrack class which holds the objects and information to render
/// the terrain.
class QNelWindow : public QMainWindow {
	Q_OBJECT
public:
	//explicit QNelWindow (QWidget *parent=0, Qt::WFlags f=0) : QMainWindow (parent, f) { initWindow(); }
	explicit QNelWindow (QWidget *parent=0, Qt::WFlags f=0);
	virtual ~QNelWindow() { }



public slots:

protected:
	void initWindow();

	//CameraTrackWidget *myWidget;
	//CameraTrackWidget *myWidget2;
};

#endif // NL_QNELWINDOW_H

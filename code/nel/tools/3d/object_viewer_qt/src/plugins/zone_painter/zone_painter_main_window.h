#ifndef ZONE_PAINTER_MAIN_WINDOW_H
#define ZONE_PAINTER_MAIN_WINDOW_H

#include <QMainWindow>

namespace NLQT {
	class QNLWidget;
}

namespace Ui {
    class ZonePainterMainWindow;
}

class PainterDockWidget;

class ZonePainterMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ZonePainterMainWindow(QWidget *parent = 0);
    ~ZonePainterMainWindow();

private:
    Ui::ZonePainterMainWindow *ui;
	NLQT::QNLWidget *m_nelWidget;
	PainterDockWidget *m_painterDockWidget;

	QAction *_toolPaintModeAction;
	QAction *_toolFillModeAction;
	QAction *_toolSelectModeAction;
	QAction *_toolPickModeAction;
	QMenu *_toolModeMenu;
};

#endif // ZONE_PAINTER_MAIN_WINDOW_H

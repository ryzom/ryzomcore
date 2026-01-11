#ifndef ZONE_PAINTER_MAIN_WINDOW_H
#define ZONE_PAINTER_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <QtGui/QUndoStack>

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

	void loadConfig();
	void saveConfig();
	QUndoStack *getUndoStack() { return m_undoStack; }
public Q_SLOTS:
	void setToolMode(int value);
	void setToolMode();
	void updateStatusBar();
	void setBackgroundColor();

protected:
        virtual void showEvent(QShowEvent *showEvent);
        virtual void hideEvent(QHideEvent *hideEvent);

private:
    Ui::ZonePainterMainWindow *ui;
	NLQT::QNLWidget *m_nelWidget;
	PainterDockWidget *m_painterDockWidget;
	QTimer *m_statusBarTimer;
	QLabel *m_statusInfo;

	QAction *_toolPaintModeAction;
	QAction *_toolFillModeAction;
	QAction *_toolSelectModeAction;
	QAction *_toolPickModeAction;
	QMenu *_toolModeMenu;
	QUndoStack *m_undoStack;
	//QAction *m_setBackColorAction;
};

#endif // ZONE_PAINTER_MAIN_WINDOW_H

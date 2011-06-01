#ifndef MISSION_COMPILER_MAIN_WINDOW_H
#define MISSION_COMPILER_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <QtGui/QUndoStack>
#include <QStringListModel>

namespace Ui {
    class MissionCompilerMainWindow;
}

class MissionCompilerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MissionCompilerMainWindow(QWidget *parent = 0);
    ~MissionCompilerMainWindow();

	void loadConfig();
	void saveConfig();
	QUndoStack *getUndoStack() { return m_undoStack; }


private:
    Ui::MissionCompilerMainWindow *ui;

	QMenu *_toolModeMenu;
	QUndoStack *m_undoStack;
	QStringListModel *m_allPrimitivesModel;
};

#endif // MISSION_COMPILER_MAIN_WINDOW_H

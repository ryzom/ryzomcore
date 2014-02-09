#ifndef MISSION_COMPILER_MAIN_WINDOW_H
#define MISSION_COMPILER_MAIN_WINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QLabel>
#include <QAction>
#include <QtGui/QUndoStack>
#include <QStringListModel>
#include <QSortFilterProxyModel>
#include <QRegExp>

#include <nel/ligo/ligo_config.h>
#include <nel/ligo/primitive.h>

namespace Ui {
    class MissionCompilerMainWindow;
}

struct CMission;

class MissionCompilerMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MissionCompilerMainWindow(QWidget *parent = 0);
    ~MissionCompilerMainWindow();

	void loadConfig();
	void saveConfig();
	QUndoStack *getUndoStack() { return m_undoStack; }

	typedef std::map<std::string, CMission> TMissionContainer;

public Q_SLOTS:
	void handleFilterChanged(const QString &text);
	void handleValidation();
	void handleCompile();
	void handlePublish();
	void handleAllDoubleClick(const QModelIndex &index);
	void handleSelDoubleClick(const QModelIndex &index);
	void handleMoveSelectedRight();
	void handleMoveSelectedLeft();
	void handleMoveAllRight();
	void handleMoveAllLeft();
	void handleDataDirButton();
	void handleDataDirChanged(const QString &text);
	void handleResetFiltersButton();
	void handleChangedSettings();

private:
    Ui::MissionCompilerMainWindow *ui;

	void updateCompileLog();
	void populateAllPrimitives(const QString &dataDir = QString());
	bool parsePrimForMissions(NLLIGO::IPrimitive const *prim, TMissionContainer &missions);
	void compileMission(bool publish=false);
	void moveSelectedItem(const QModelIndex &index, QStringListModel *from, QStringListModel *to);
	void applyCheckboxes(const QStringList &servers);
	
	
	QMenu *_toolModeMenu;
	QUndoStack *m_undoStack;
	QStringListModel *m_allPrimitivesModel;
	QStringListModel *m_selectedPrimitivesModel;
	QSortFilterProxyModel *m_filteredProxyModel;
	QRegExp *m_regexpFilter;
	QString m_compileLog;
	QString m_lastDir;

	NLLIGO::CLigoConfig m_ligoConfig;
};

#endif // MISSION_COMPILER_MAIN_WINDOW_H

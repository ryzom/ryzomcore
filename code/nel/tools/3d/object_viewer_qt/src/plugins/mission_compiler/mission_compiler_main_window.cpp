#include "mission_compiler_main_window.h"
#include "ui_mission_compiler_main_window.h"

#include <QMenu>
#include <QSignalMapper>
#include <QColor>
#include <QColorDialog>
#include <QSettings>

#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

#include <nel/misc/path.h>

MissionCompilerMainWindow::MissionCompilerMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MissionCompilerMainWindow)
{
    ui->setupUi(this);

	// Load the settings.
	loadConfig();

	m_undoStack = new QUndoStack(this);

	// Populate the "all" primitives box.
	QStringList list;
	std::vector<std::string> paths;
	NLMISC::CPath::getFileList("primitive", paths);
	
	std::vector<std::string>::iterator itr = paths.begin();
	while( itr != paths.end() )
	{
		const char *path2 = (*itr).c_str();
		list << path2;
		++itr;
	}

	m_regexpFilter = new QRegExp();
	m_regexpFilter->setPatternSyntax(QRegExp::FixedString);
	m_regexpFilter->setCaseSensitivity(Qt::CaseInsensitive);
	
	m_allPrimitivesModel = new QStringListModel(list, this);
	m_filteredProxyModel = new QSortFilterProxyModel(this);
	m_filteredProxyModel->setSourceModel(m_allPrimitivesModel);
	m_filteredProxyModel->setDynamicSortFilter(true);
	m_filteredProxyModel->setFilterRegExp(*m_regexpFilter);
	ui->allPrimitivesList->setModel(m_filteredProxyModel);
	m_selectedPrimitivesModel = new QStringListModel(this);
	ui->selectedPrimitivesList->setModel(m_selectedPrimitivesModel);

	connect(ui->filterEdit, SIGNAL(textEdited(const QString&)), this, SLOT(handleFilterChanged(const QString&)));

}

void MissionCompilerMainWindow::handleFilterChanged(const QString &text)
{
	m_regexpFilter->setPattern(text);
	m_filteredProxyModel->setFilterRegExp(*m_regexpFilter);
}

void MissionCompilerMainWindow::loadConfig() {
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("MissionCompiler");

	//QColor color;
	//color = settings->value("BackgroundColor", QColor(80, 80, 80)).value<QColor>();
	//m_nelWidget->setBackgroundColor(NLMISC::CRGBA(color.red(), color.green(), color.blue(), color.alpha()));
}

void MissionCompilerMainWindow::saveConfig() {
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("MissionCompiler" );

	//QColor color(m_nelWidget->backgroundColor().R, m_nelWidget->backgroundColor().G, m_nelWidget->backgroundColor().B, m_nelWidget->backgroundColor().A);
	//settings->setValue("BackgroundColor", color);

	settings->endGroup();
	settings->sync();
}

MissionCompilerMainWindow::~MissionCompilerMainWindow()
{
    delete ui;
}

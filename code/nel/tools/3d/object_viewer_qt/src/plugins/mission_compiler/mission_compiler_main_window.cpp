#include "mission_compiler_main_window.h"
#include "ui_mission_compiler_main_window.h"
#include "validation_file.h"

#include <QMenu>
#include <QSignalMapper>
#include <QColor>
#include <QColorDialog>
#include <QSettings>
#include <QTextStream>

#include "../core/icore.h"
#include "../core/imenu_manager.h"
#include "../core/core_constants.h"

#include <nel/misc/common.h>

#include <nel/misc/path.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>

MissionCompilerMainWindow::MissionCompilerMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MissionCompilerMainWindow)
{
    ui->setupUi(this);

	m_compileLog = "";
	updateCompileLog();

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
	connect(ui->actionValidate, SIGNAL(triggered()), this, SLOT(handleValidation()));

	NLLIGO::Register();
	m_ligoConfig.readPrimitiveClass(NLMISC::CPath::lookup("world_editor_classes.xml").c_str(), false);
	NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &m_ligoConfig;
}

void MissionCompilerMainWindow::handleFilterChanged(const QString &text)
{
	m_regexpFilter->setPattern(text);
	m_filteredProxyModel->setFilterRegExp(*m_regexpFilter);
}

void MissionCompilerMainWindow::handleValidation()
{
	// First switch toolbox pages to show the compilation output.
	ui->toolBox->setCurrentIndex(2);

	m_compileLog.append("Begin mission validation.\n");
	updateCompileLog();

	// Load existing validation
	CValidationFile validation;
	validation.loadMissionValidationFile("mission_validation.cfg");

	// Go through each file.
	QStringList list = m_selectedPrimitivesModel->stringList();
	QStringListIterator itr(list);
	while(itr.hasNext())
	{
		QString filename = itr.next();
		//QString filePath = NLMISC::CPath::lookup(filename.toAscii().data(), false).c_str();
		m_compileLog.append("Parsing '"+filename+"'...\n");
		updateCompileLog();

		TMissionContainer missions;
		NLLIGO::CPrimitives primDoc;
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		NLLIGO::loadXmlPrimitiveFile(primDoc, NLMISC::CPath::lookup(filename.toAscii().data(), false), m_ligoConfig);
		parsePrimForMissions(primDoc.RootNode, missions);

		// Parse missions to check modification
		std::map<std::string, CMission>::iterator itMission, itMissionEnd = missions.end();
		for (itMission=missions.begin(); itMission!=itMissionEnd; ++itMission)
		{
			CValidationFile::TMissionStateContainer::iterator itMissionValidation = validation._MissionStates.find(itMission->first);
			if (itMissionValidation!=validation._MissionStates.end())
			{
				// Mission already registered, check hash key
				if (itMissionValidation->second.hashKey!=itMission->second.hashKey)
				{
					itMissionValidation->second.hashKey = itMission->second.hashKey;
					itMissionValidation->second.state = validation.defaultState();
				}
			}
			else
			{
				// New mission
				validation.insertMission(itMission->first, itMission->second.hashKey);
			}
			m_compileLog.append("Mission: '"+QString(itMission->first.c_str())+"->"+QString(itMission->second.hashKey.c_str())+"\n");
			updateCompileLog();
		}
	}
	validation.saveMissionValidationFile("mission_validation.cfg");
	
	m_compileLog.append("Validation finished");
	updateCompileLog();
}

bool MissionCompilerMainWindow::parsePrimForMissions(NLLIGO::IPrimitive const *prim, TMissionContainer &missions)
{
	std::string value;
	// if the node is a mission parse it
	if (prim->getPropertyByName("class",value) && !stricmp(value.c_str(),"mission") )
	{
		std::string name;
		prim->getPropertyByName("name",name);
		
		m_compileLog.append("  ** Parsing mission '"+QString(name.c_str())+"'\n");
		updateCompileLog();

		// parse the mission and put it in our manager
		CMission mission(value, "");
		if (!mission.parsePrim(prim) )
		{
			m_compileLog.append("    ** Previous errors in mission '"+QString(name.c_str())+"'");
			updateCompileLog();
			return false;
		}
		missions.insert(make_pair(name, mission));
		return true;
	}
	else
	{
		//this is not a mission node, so lookup recursively in the children
		bool ok = true;
		for (uint i=0;i<prim->getNumChildren();++i)	
		{
			const NLLIGO::IPrimitive *child;
			if ( !prim->getChild(child,i) || !parsePrimForMissions(child, missions) )
				ok = false;
		}
		return ok;
	}
}

void MissionCompilerMainWindow::updateCompileLog()
{
	ui->compileOutputText->setPlainText(m_compileLog);
	QCoreApplication::processEvents();
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

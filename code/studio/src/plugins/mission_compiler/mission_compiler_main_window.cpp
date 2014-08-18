#include "mission_compiler_main_window.h"
#include "ui_mission_compiler_main_window.h"
#include "validation_file.h"
#include "mission_compiler.h"
#include "mission_compiler_plugin_constants.h"

#include <QMenu>
#include <QSignalMapper>
#include <QColor>
#include <QColorDialog>
#include <QSettings>
#include <QTextStream>
#include <QFileDialog>
#include <QDirIterator>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

#include <nel/misc/common.h>

#include <nel/misc/path.h>
#include <nel/ligo/primitive_utils.h>
#include <nel/ligo/primitive.h>
#include <nel/ligo/ligo_config.h>

#include <string.h>

using namespace MissionCompiler::Constants;

MissionCompilerMainWindow::MissionCompilerMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MissionCompilerMainWindow)
{
    ui->setupUi(this);

	m_lastDir = ".";
	m_compileLog = "";
	updateCompileLog();

	// Load the settings.
	loadConfig();

	m_undoStack = new QUndoStack(this);

	m_regexpFilter = new QRegExp();
	m_regexpFilter->setPatternSyntax(QRegExp::FixedString);
	m_regexpFilter->setCaseSensitivity(Qt::CaseInsensitive);
	
	m_allPrimitivesModel = new QStringListModel(this);
	m_filteredProxyModel = new QSortFilterProxyModel(this);
	m_filteredProxyModel->setSourceModel(m_allPrimitivesModel);
	m_filteredProxyModel->setDynamicSortFilter(true);
	m_filteredProxyModel->setFilterRegExp(*m_regexpFilter);
	ui->allPrimitivesList->setModel(m_filteredProxyModel);
	m_selectedPrimitivesModel = new QStringListModel(this);
	ui->selectedPrimitivesList->setModel(m_selectedPrimitivesModel);
	
	// Connections for toolbar buttons.
	connect(ui->actionValidate, SIGNAL(triggered()), this, SLOT(handleValidation()));
	connect(ui->actionCompile, SIGNAL(triggered()), this, SLOT(handleCompile()));
	connect(ui->actionPublish, SIGNAL(triggered()), this, SLOT(handlePublish()));
	
	// Connections for selected item moves.
	connect(ui->allPrimitivesList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(handleAllDoubleClick(const QModelIndex &)));
	connect(ui->selectedPrimitivesList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(handleSelDoubleClick(const QModelIndex &)));
	connect(ui->addSelectedButton, SIGNAL(clicked()), this, SLOT(handleMoveSelectedRight()));
	connect(ui->removeSelectedButton, SIGNAL(clicked()), this, SLOT(handleMoveSelectedLeft()));
	connect(ui->addAllButton, SIGNAL(clicked()), this, SLOT(handleMoveAllRight()));
	connect(ui->removeAllButton, SIGNAL(clicked()), this, SLOT(handleMoveAllLeft()));

	// Connections for the filter group box.
	connect(ui->dataDirButton, SIGNAL(clicked()), this, SLOT(handleDataDirButton()));
	connect(ui->dataDirEdit, SIGNAL(textChanged(const QString &)), this, SLOT(handleDataDirChanged(const QString &)));
	connect(ui->filterEdit, SIGNAL(textEdited(const QString&)), this, SLOT(handleFilterChanged(const QString&)));
	connect(ui->resetFiltersButton, SIGNAL(clicked()), this, SLOT(handleResetFiltersButton()));

	// Connect for settings changes.
	connect(Core::ICore::instance(), SIGNAL(changeSettings()), this, SLOT(handleChangedSettings()));

	// Set the default data dir to the primitives path.
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(Core::Constants::DATA_PATH_SECTION);
	m_lastDir = settings->value(Core::Constants::PRIMITIVES_PATH).toString();
	ui->dataDirEdit->setText(m_lastDir);
	populateAllPrimitives(m_lastDir);
	settings->endGroup();

	NLLIGO::Register();
	// TODO try/catch exception. Crashes if path invalid.
	try{
		m_ligoConfig.readPrimitiveClass(NLMISC::CPath::lookup("world_editor_classes.xml").c_str(), false);
	}
	catch( NLMISC::Exception &e )
	{
		nlinfo( "Exception occured during Mission Compiler LIGO startup: %s", e.what() );
	}

	NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &m_ligoConfig;
}

void MissionCompilerMainWindow::populateAllPrimitives(const QString &dataDir)
{
	// First we need to clear out the models entirely.
	QStringList emptyList;
	m_selectedPrimitivesModel->setStringList(emptyList);
	m_allPrimitivesModel->setStringList(emptyList);

	
	// Populate the "all" primitives box.
	QStringList list;
	
	// Filter for only primitive files.
	QStringList filters;
	filters << "*.primitive";

	QDirIterator it(dataDir, filters, QDir::Files, QDirIterator::Subdirectories|QDirIterator::FollowSymlinks);
	while(it.hasNext())
	{
		it.next();
		list <<  it.fileName();
	}

	m_allPrimitivesModel->setStringList(list);
}

void MissionCompilerMainWindow::handleResetFiltersButton()
{
	handleDataDirChanged(m_lastDir);
	ui->filterEdit->setText("");
	handleFilterChanged("");
}

void MissionCompilerMainWindow::handleDataDirChanged(const QString &text)
{
	populateAllPrimitives(text);
}

void MissionCompilerMainWindow::handleDataDirButton()
{
	QString newPath = QFileDialog::getExistingDirectory(this, "", m_lastDir);
	if(!newPath.isEmpty())
	{
		ui->dataDirEdit->setText(newPath);
		m_lastDir = newPath;
		populateAllPrimitives(newPath);
	}
}

void MissionCompilerMainWindow::handleFilterChanged(const QString &text)
{
	m_regexpFilter->setPattern(text);
	m_filteredProxyModel->setFilterRegExp(*m_regexpFilter);
}

void MissionCompilerMainWindow::handleCompile()
{
	compileMission();
}

void MissionCompilerMainWindow::handlePublish()
{
	compileMission(true);
}

void MissionCompilerMainWindow::handleMoveSelectedRight()
{
	QModelIndexList indexes = ui->allPrimitivesList->selectionModel()->selectedIndexes();
	while(!indexes.isEmpty())
	{
		const QModelIndex index = indexes.takeFirst();
		moveSelectedItem(index, m_allPrimitivesModel, m_selectedPrimitivesModel);
		indexes = ui->allPrimitivesList->selectionModel()->selectedIndexes();
	}
}

void MissionCompilerMainWindow::handleMoveAllRight()
{
	ui->allPrimitivesList->selectAll();
	handleMoveSelectedRight();
}

void MissionCompilerMainWindow::handleMoveSelectedLeft()
{
	QModelIndexList indexes = ui->selectedPrimitivesList->selectionModel()->selectedIndexes();
	while(!indexes.isEmpty())
	{
		const QModelIndex index = indexes.takeFirst();
		moveSelectedItem(index, m_selectedPrimitivesModel, m_allPrimitivesModel);
		indexes = ui->selectedPrimitivesList->selectionModel()->selectedIndexes();
	}
}

void MissionCompilerMainWindow::handleMoveAllLeft()
{
	ui->selectedPrimitivesList->selectAll();
	handleMoveSelectedLeft();
}

void MissionCompilerMainWindow::moveSelectedItem(const QModelIndex &index, QStringListModel *from, QStringListModel *to)
{
	QString item = from->data(index, Qt::DisplayRole).toString();

	from->removeRows(index.row(),1);
	QStringList list = to->stringList();
	list << item;
	to->setStringList(list);
}

void MissionCompilerMainWindow::handleAllDoubleClick(const QModelIndex &index)
{
	moveSelectedItem(index, m_allPrimitivesModel, m_selectedPrimitivesModel);
}

void MissionCompilerMainWindow::handleSelDoubleClick(const QModelIndex &index)
{
	moveSelectedItem(index, m_selectedPrimitivesModel, m_allPrimitivesModel);
}

void MissionCompilerMainWindow::compileMission(bool publish)
{
	uint nbMission = 0;

	// First switch toolbox pages to show the compilation output.
	ui->toolBox->setCurrentIndex(2);

	m_compileLog.append("Begin mission compilation.\n");
	updateCompileLog();

	// Go through each file.
	QStringList list = m_selectedPrimitivesModel->stringList();
	QStringListIterator itr(list);
	while(itr.hasNext())
	{
		QString filename = itr.next();
		m_compileLog.append("Compiling '"+filename+"'...\n");
		updateCompileLog();

		NLLIGO::CPrimitives primDoc;
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = &primDoc;
		NLLIGO::loadXmlPrimitiveFile(primDoc, NLMISC::CPath::lookup(filename.toAscii().data(), false), m_ligoConfig);
		NLLIGO::CPrimitiveContext::instance().CurrentPrimitive = NULL;

		try
		{
			CMissionCompiler mc;
			mc.compileMissions(primDoc.RootNode, filename.toUtf8().constData());
			m_compileLog.append("Found "+QString::number(mc.getMissionsCount())+" valid missions\n");
			updateCompileLog();

			mc.installCompiledMission(m_ligoConfig, filename.toUtf8().constData());
			nbMission += mc.getMissionsCount();

			// publish files to selected servers
			if (publish)
			{
				m_compileLog.append("Begin publishing missions...\n");
				QSettings *settings = Core::ICore::instance()->settings();
				settings->beginGroup(MISSION_COMPILER_SECTION);

				// Retrieve the local text path.
				QString localPath = settings->value(SETTING_LOCAL_TEXT_PATH).toString();
				settings->endGroup();
				QStringList checkedServers;
				for(int i = 0; i<ui->publishServersList->count(); i++)
				{

					// Retrieve each checked server.
					QListWidgetItem *item = ui->publishServersList->item(i);
					if(item->checkState() == Qt::Checked)
						checkedServers << item->text();
				}
				
				Q_FOREACH(QString checkedServer, checkedServers)
				{
					m_compileLog.append("Processing publication configuration for '"+checkedServer+"'\n");
					QStringList items = settings->value(SETTING_SERVERS_TABLE_ITEMS).toStringList();
					int column = 0;
					int row = 0;
					QString servName;
					QString primPath;
					QString textPath;
					Q_FOREACH(QString var, items)
					{
						// Check to see if we're starting a new row.
						if(column > 2)
						{
							column = 0;
							row++;
						}
						if(column == 0)
							servName = var;
						else if(column == 1)
							textPath = var;
						else if(column == 2)
						{
							primPath = var;

							m_compileLog.append("Publishing to "+servName+" ...\n");
							for (uint j=0 ; j<mc.getFileToPublishCount() ; j++)
							{
								m_compileLog.append("   "+QString(NLMISC::CFile::getFilename(mc.getFileToPublish(j)).c_str())+"\n");
							}
							mc.publishFiles(primPath.toUtf8().constData(), textPath.toUtf8().constData(), localPath.toUtf8().constData());
						}
						
						column++;
					}
				}
				m_compileLog.append("End publishing missions...\n");
			}
		}
		catch(const EParseException &e)
		{

			if (e.Primitive != NULL)
				m_compileLog.append("In '"+QString(buildPrimPath(e.Primitive).c_str())+"'\n");

			m_compileLog.append("Error while compiling '"+filename+"' :\n"+QString(e.Why.c_str())+"\n");
			updateCompileLog();
			break;
		}
	}

	m_compileLog.append("Mission compilation complete.\n");
	updateCompileLog();
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
	if (prim->getPropertyByName("class",value) && !NLMISC::nlstricmp(value.c_str(),"mission") )
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
	settings->beginGroup(MISSION_COMPILER_SECTION);

	// Retrieve the local text path.
	QString localPath = settings->value(SETTING_LOCAL_TEXT_PATH).toString();
	QListWidgetItem *item = new QListWidgetItem("Local");
	item->setForeground(Qt::blue);
	item->setCheckState(Qt::Unchecked);
	ui->publishServersList->addItem(item);

	QStringList items = settings->value(SETTING_SERVERS_TABLE_ITEMS).toStringList();
	int column = 0;
	int row = 0;
	Q_FOREACH(QString var, items)
	{
		// Check to see if we're starting a new row.
		if(column > 2)
		{
			column = 0;
			row++;
		}
		if(column == 0)
		{
			item = new QListWidgetItem(var);
			item->setCheckState(Qt::Unchecked);
			ui->publishServersList->addItem(item);
		}

		column++;
	}

	// Reapply the checkboxes for servers we had checked previously.
	QStringList servers = settings->value(SETTING_PUBLISH_SERVER_CHECKS).toStringList();
	applyCheckboxes(servers);

	settings->endGroup();
}

void MissionCompilerMainWindow::saveConfig() {
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup(MISSION_COMPILER_SECTION);

	QStringList servers;
	for(int row = 0; row < ui->publishServersList->count(); row++)
	{
		QListWidgetItem *item = ui->publishServersList->item(row);
		if(item->checkState() == Qt::Checked)
			servers << item->text();
	}

	settings->setValue(SETTING_PUBLISH_SERVER_CHECKS, servers);

	settings->endGroup();
	settings->sync();
}

void MissionCompilerMainWindow::handleChangedSettings()
{
	QStringList servers;
	for(int row = 0; row < ui->publishServersList->count(); row++)
	{
		QListWidgetItem *item = ui->publishServersList->item(row);
		if(item->checkState() == Qt::Checked)
			servers << item->text();
	}
	ui->publishServersList->clear();
	loadConfig();

	applyCheckboxes(servers);
}

void MissionCompilerMainWindow::applyCheckboxes(const QStringList &servers)
{
	Q_FOREACH(QString server, servers)
	{
		QList<QListWidgetItem*> items = ui->publishServersList->findItems(server, Qt::MatchExactly);
		if(items.size() != 1)
			continue;
		items.at(0)->setCheckState(Qt::Checked);
	}
}

MissionCompilerMainWindow::~MissionCompilerMainWindow()
{
	saveConfig();
    delete ui;
}

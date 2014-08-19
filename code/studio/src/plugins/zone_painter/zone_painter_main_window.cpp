#include "zone_painter_main_window.h"
#include "ui_zone_painter_main_window.h"

#include <QMenu>
#include <QSignalMapper>
#include <QColor>
#include <QColorDialog>
#include <QSettings>

#include "qnel_widget.h"
#include "painter_dock_widget.h"

#include "../core/icore.h"
#include "../core/menu_manager.h"
#include "../core/core_constants.h"

ZonePainterMainWindow::ZonePainterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ZonePainterMainWindow)
{
    ui->setupUi(this);
	m_nelWidget = new NLQT::QNLWidget(this);
	setCentralWidget(m_nelWidget);

	// Load the settings.
	loadConfig();

	// Set up dock widget(s) and toolbar.
	m_painterDockWidget = new PainterDockWidget(this);
	addDockWidget(Qt::RightDockWidgetArea, m_painterDockWidget);
	m_painterDockWidget->setVisible(true);

	// Insert tool modes
	_toolModeMenu = new QMenu(tr("Tool Mode"), ui->painterToolBar);
	_toolModeMenu->setIcon(QIcon(":/painterTools/images/draw-brush.png"));
	ui->painterToolBar->addAction(_toolModeMenu->menuAction());
	//connect(_renderModeMenu->menuAction(), SIGNAL(triggered()), this, SLOT(setRenderMode()));

	QSignalMapper *modeMapper = new QSignalMapper(this);

	_toolPaintModeAction = _toolModeMenu->addAction(tr("Paint Mode"));
	_toolPaintModeAction->setIcon(QIcon(":/painterTools/images/draw-brush.png"));
	_toolPaintModeAction->setStatusTip(tr("Set paint mode"));
	connect(_toolPaintModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_toolPaintModeAction, 0);

	_toolFillModeAction = _toolModeMenu->addAction(tr("Fill Mode"));
	_toolFillModeAction->setStatusTip(tr("Set fill mode"));
	_toolFillModeAction->setIcon(QIcon(":/painterTools/images/color-fill.png"));
	connect(_toolFillModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_toolFillModeAction, 1);

	_toolSelectModeAction = _toolModeMenu->addAction(tr("Select mode"));
	_toolSelectModeAction->setIcon(QIcon(":/painterTools/images/go-jump-4.png"));
	_toolSelectModeAction->setStatusTip(tr("Set select mode"));
	connect(_toolSelectModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_toolSelectModeAction, 2);

	_toolPickModeAction = _toolModeMenu->addAction(tr("Pick mode"));
	_toolPickModeAction->setIcon(QIcon(":/painterTools/images/color-picker-black.png"));
	_toolPickModeAction->setStatusTip(tr("Set color picking mode"));
	connect(_toolPickModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	modeMapper->setMapping(_toolPickModeAction, 2);

	connect(modeMapper, SIGNAL(mapped(int)), this, SLOT(setToolMode(int)));

	m_statusBarTimer = new QTimer(this);
	connect(m_statusBarTimer, SIGNAL(timeout()), this, SLOT(updateStatusBar()));
	m_statusInfo = new QLabel(this);
    m_statusInfo->hide();

	// Set Background Color Toolbar
	

	connect(ui->actionBackground_Dlg, SIGNAL(triggered()), this, SLOT(setBackgroundColor()));
	

    Core::ICore::instance()->mainWindow()->statusBar()->addPermanentWidget(m_statusInfo);

	m_undoStack = new QUndoStack(this);
}

void ZonePainterMainWindow::showEvent(QShowEvent *showEvent)
{
	QMainWindow::showEvent(showEvent);
	m_statusBarTimer->start(1000);
    m_statusInfo->show();
}

void ZonePainterMainWindow::hideEvent(QHideEvent *hideEvent)
{
	m_statusBarTimer->stop();
    m_statusInfo->hide();
	QMainWindow::hideEvent(hideEvent);
}

void ZonePainterMainWindow::updateStatusBar()
{
	m_statusInfo->setText(QString("Tool Mode: Paint Mode"));
}

void ZonePainterMainWindow::setToolMode(int value)
{
	switch (value)
	{
	case 0:
		//Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Point);
		break;
	case 1:
		//Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Line);
		break;
	case 2:
		//Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Filled);
		break;
	}
}

void ZonePainterMainWindow::setToolMode()
{
	//switch (Modules::objView().getDriver()->getPolygonMode())
	//{
	//case NL3D::UDriver::Filled:
	//	Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Line);
	//	break;
	//case NL3D::UDriver::Line:
	//	Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Point);
	//	break;
	//case NL3D::UDriver::Point:
	//	Modules::objView().getDriver()->setPolygonMode (NL3D::UDriver::Filled);
	//	break;
	//}
}

void ZonePainterMainWindow::setBackgroundColor() {
	QColor color = QColorDialog::getColor(QColor(m_nelWidget->backgroundColor().R,
										  m_nelWidget->backgroundColor().G,
										  m_nelWidget->backgroundColor().B));
	if (color.isValid())
		m_nelWidget->setBackgroundColor(NLMISC::CRGBA(color.red(), color.green(), color.blue()));
}

void ZonePainterMainWindow::loadConfig() {
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("ZonePainter");

	QColor color;
	color = settings->value("BackgroundColor", QColor(80, 80, 80)).value<QColor>();
	settings->endGroup();
	m_nelWidget->setBackgroundColor(NLMISC::CRGBA(color.red(), color.green(), color.blue(), color.alpha()));
}

void ZonePainterMainWindow::saveConfig() {
	QSettings *settings = Core::ICore::instance()->settings();
	settings->beginGroup("ZonePainter" );

	QColor color(m_nelWidget->backgroundColor().R, m_nelWidget->backgroundColor().G, m_nelWidget->backgroundColor().B, m_nelWidget->backgroundColor().A);
	settings->setValue("BackgroundColor", color);

	settings->endGroup();
	settings->sync();
}

ZonePainterMainWindow::~ZonePainterMainWindow()
{
    delete ui;
	delete m_nelWidget;
	delete m_painterDockWidget;
}

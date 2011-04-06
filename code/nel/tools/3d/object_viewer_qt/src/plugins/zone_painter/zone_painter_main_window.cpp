#include "zone_painter_main_window.h"
#include "ui_zone_painter_main_window.h"

#include "qnel_widget.h"
#include "painter_dock_widget.h"

ZonePainterMainWindow::ZonePainterMainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ZonePainterMainWindow)
{
    ui->setupUi(this);
	m_nelWidget = new NLQT::QNLWidget(this);
	setCentralWidget(m_nelWidget);

	// Set up dock widget(s) and toolbar.
	m_painterDockWidget = new PainterDockWidget(this);
	addDockWidget(Qt::RightDockWidgetArea, m_painterDockWidget);
	m_painterDockWidget->setVisible(true);

	// Insert tool modes
	_toolModeMenu = new QMenu(tr("Tool Mode"), ui->painterToolBar);
	_toolModeMenu->setIcon(QIcon(":/painterTools/images/draw-brush.png"));
	ui->painterToolBar->addAction(_toolModeMenu->menuAction());
	//connect(_renderModeMenu->menuAction(), SIGNAL(triggered()), this, SLOT(setRenderMode()));

	//QSignalMapper *modeMapper = new QSignalMapper(this);

	_toolPaintModeAction = _toolModeMenu->addAction(tr("Paint Mode"));
	_toolPaintModeAction->setIcon(QIcon(":/painterTools/images/draw-brush.png"));
	_toolPaintModeAction->setStatusTip(tr("Set paint mode"));
	//connect(_pointRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	//modeMapper->setMapping(_pointRenderModeAction, 0);

	_toolFillModeAction = _toolModeMenu->addAction(tr("Fill Mode"));
	_toolFillModeAction->setStatusTip(tr("Set fill mode"));
	_toolFillModeAction->setIcon(QIcon(":/painterTools/images/color-fill.png"));
	//connect(_lineRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	//modeMapper->setMapping(_lineRenderModeAction, 1);

	_toolSelectModeAction = _toolModeMenu->addAction(tr("Select mode"));
	_toolSelectModeAction->setIcon(QIcon(":/painterTools/images/go-jump-4.png"));
	_toolSelectModeAction->setStatusTip(tr("Set select mode"));
	//connect(_fillRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	//modeMapper->setMapping(_fillRenderModeAction, 2);

	_toolPickModeAction = _toolModeMenu->addAction(tr("Pick mode"));
	_toolPickModeAction->setIcon(QIcon(":/painterTools/images/color-picker-black.png"));
	_toolPickModeAction->setStatusTip(tr("Set color picking mode"));
	//connect(_fillRenderModeAction, SIGNAL(triggered()), modeMapper, SLOT(map()));
	//modeMapper->setMapping(_fillRenderModeAction, 2);

	//connect(modeMapper, SIGNAL(mapped(int)), this, SLOT(setRenderMode(int)));
	
}

ZonePainterMainWindow::~ZonePainterMainWindow()
{
    delete ui;
	delete m_nelWidget;
	delete m_painterDockWidget;
}

#include "painter_dock_widget.h"
#include "ui_painter_dock_widget.h"

#include "qnel_widget.h"

PainterDockWidget::PainterDockWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::PainterDockWidget)
{
    ui->setupUi(this);
}

PainterDockWidget::~PainterDockWidget()
{
    delete ui;
}

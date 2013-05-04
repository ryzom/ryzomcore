#ifndef PAINTER_DOCK_WIDGET_H
#define PAINTER_DOCK_WIDGET_H

#include <QDockWidget>

namespace Ui {
    class PainterDockWidget;
}

class PainterDockWidget : public QDockWidget
{
    Q_OBJECT

public:
    explicit PainterDockWidget(QWidget *parent = 0);
    ~PainterDockWidget();

private:
    Ui::PainterDockWidget *ui;
};

#endif // PAINTER_DOCK_WIDGET_H

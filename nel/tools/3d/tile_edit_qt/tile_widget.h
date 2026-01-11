#ifndef TILE_WIDGET_H
#define TILE_WIDGET_H

#include "tiles_model.h"
#include "ui_tile_widget_qt.h"

#define PIXMAP_MARGIN 5

class CTile_Widget : public QWidget
 {
     Q_OBJECT

 public:
	CTile_Widget(QWidget *parent = 0);
	void initWidget(const QPixmap&, const int, const QString&);

 private:
	 Ui::TileWidget ui;
 };

#endif

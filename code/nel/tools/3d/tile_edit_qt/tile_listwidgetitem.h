 
#ifndef TILE_WIDGET_H
#define TILE_WIDGET_H

#include <QListWidgetItem>
#include "ui_tile_widget_qt.h"

class CTile_ListWidgetItem : public QListWidgetItem
 {
 public:
	 CTile_ListWidgetItem ( QListWidget * parent, int type = Type ):QListWidgetItem(parent,type){};

	CTile_ListWidgetItem(QWidget *parent = 0);
	void initWidget(const QPixmap&, const QString&);

 private:
	 Ui::TileWidget ui;
	// Qpixmap tilePixmap;
	// QString tileLabel;
 };

#endif
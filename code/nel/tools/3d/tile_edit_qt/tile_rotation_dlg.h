#ifndef TILE_ROTATIONDLG_H
#define TILE_ROTATIONDLG_H


#include <QtGui/QtGui>
#include <QtGui/QDialog>
#include <QtGui/QButtonGroup>
#include "ui_tile_rotation_qt.h"

class CTile_rotation_dlg : public QDialog
{
	Q_OBJECT

public:
	static int getRotation(QWidget *parent, bool *ok = 0,Qt::WindowFlags f = 0);

	enum TileRotation
	{ 
		_0Rotation = 0,
		_90Rotation = 3,
		_180Rotation = 2,
		_270Rotation = 1
	};

	int getCheckedRotation() const {	return rotationButtonGroup->checkedId();	}

private:
	CTile_rotation_dlg(QWidget *parent = 0, Qt::WindowFlags f = 0);
	Ui::TileRotationDialog ui;
	QButtonGroup* rotationButtonGroup;
};

#endif

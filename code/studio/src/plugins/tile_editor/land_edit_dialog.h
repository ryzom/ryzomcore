#ifndef LAND_EDIT_DLG_H
#define LAND_EDIT_DLG_H


#include "ui_land_edit_dialog.h"
#include <QStringList>

class LandEditDialog : public QDialog, public Ui::LandEditDialog
{
	Q_OBJECT
public:
	LandEditDialog( QWidget *parent = NULL );
	~LandEditDialog();

	void getSelectedTileSets( QStringList &l ) const;
	void setSelectedTileSets( QStringList &l );

	void setTileSets( const QStringList &l );

private:
	void setupConnections();


private Q_SLOTS:
	void onOkClicked();
	void onCancelClicked();
	void onAddClicked();
	void onRemoveClicked();

};


#endif


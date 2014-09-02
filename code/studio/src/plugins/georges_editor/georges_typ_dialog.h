#ifndef GEORGES_TYP_DIALOG
#define GEORGES_TYP_DIALOG

#include "georges_dock_widget.h"
#include "ui_georges_typ_dialog.h"

class GeorgesTypDialogPvt;

class GeorgesTypDialog : public GeorgesDockWidget
{
	Q_OBJECT
public:
	GeorgesTypDialog( QWidget *parent = NULL );
	~GeorgesTypDialog();

	bool load( const QString &fileName );
	void write();

Q_SIGNALS:
	void modified();

private Q_SLOTS:
	void onAddClicked();
	void onRemoveClicked();

	void onItemChanged( QTreeWidgetItem *item, int column );
	void onModified();

private:
	void setupConnections();
	void log( const QString &msg );
	void loadTyp();

	Ui::GeorgesTypDialog m_ui;
	GeorgesTypDialogPvt *m_pvt;

	QString m_fileName;
};


#endif


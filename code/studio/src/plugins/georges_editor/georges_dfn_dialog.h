#ifndef GEORGES_DFN_DIALOG
#define GEORGES_DFN_DIALOG

#include "georges_dock_widget.h"
#include "ui_georges_dfn_dialog.h"

class GeorgesDFNDialogPvt;

class GeorgesDFNDialog : public GeorgesDockWidget
{
	Q_OBJECT
public:
	GeorgesDFNDialog( QWidget *parent = NULL );
	~GeorgesDFNDialog();

	bool load( const QString &fileName );
	void write();

Q_SIGNALS:
	void modified();

private Q_SLOTS:
	void onAddClicked();
	void onRemoveClicked();
	void onCurrentRowChanged( int row );

	void onValueChanged( const QString& key, const QString &value );

private:
	void setupConnections();

	Ui::GeorgesDFNDialog m_ui;
	GeorgesDFNDialogPvt *m_pvt;
};

#endif


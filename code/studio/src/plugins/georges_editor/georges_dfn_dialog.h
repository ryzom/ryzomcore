#ifndef GEORGES_DFN_DIALOG
#define GEORGES_DFN_DIALOG

#include "ui_georges_dfn_dialog.h"

class GeorgesDFNDialog : public QDockWidget
{
	Q_OBJECT
public:
	GeorgesDFNDialog( QWidget *parent = NULL );
	~GeorgesDFNDialog();

private Q_SLOTS:
	void onAddClicked();
	void onRemoveClicked();

private:
	void setupConnections();

	Ui::GeorgesDFNDialog m_ui;
};

#endif


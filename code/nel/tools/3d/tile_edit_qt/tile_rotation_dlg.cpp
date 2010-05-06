#include "tile_rotation_dlg.h"

CTile_rotation_dlg::CTile_rotation_dlg(QWidget *parent, Qt::WindowFlags flags)
     : QDialog(parent, flags)

{
	ui.setupUi(this);

	rotationButtonGroup = new QButtonGroup;
    
	rotationButtonGroup->addButton(ui._0PushButton, CTile_rotation_dlg::_0Rotation);
	rotationButtonGroup->addButton(ui._90PushButton, CTile_rotation_dlg::_90Rotation);
	rotationButtonGroup->addButton(ui._180PushButton, CTile_rotation_dlg::_180Rotation);
	rotationButtonGroup->addButton(ui._270PushButton, CTile_rotation_dlg::_270Rotation);
}


int CTile_rotation_dlg::getRotation(QWidget *parent, bool *ok, Qt::WindowFlags f)
{
    CTile_rotation_dlg dlg(parent, f);
    bool accepted = (dlg.exec() == QDialog::Accepted);
    if (ok)
        *ok = accepted;
    return dlg.getCheckedRotation();
}

#include <QtGui/qlistwidget.h>

#include "source_selection.h"

namespace TranslationManager
{
    

CSourceDialog::CSourceDialog(QWidget *parent): QDialog(parent)
{
	_ui.setupUi(this);
	// Set signal and slot for "OK Button"         
	connect(_ui.ok_button, SIGNAL(clicked()), this, SLOT(OkButtonClicked()));
    // Set signal and slot for "Cancel Button"
    connect(_ui.cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
    _ui.listWidget->setSortingEnabled(false);
}

void CSourceDialog::setSourceOptions(map<QListWidgetItem*,int> options)
{
    map<QListWidgetItem*,int>::iterator it;
    
    for(it = options.begin(); it != options.end(); ++it)
    {
        _ui.listWidget->addItem((*it).first);
    }
}

void CSourceDialog::OkButtonClicked()
{
    selected_item = _ui.listWidget->currentItem();
    reject();
}

}

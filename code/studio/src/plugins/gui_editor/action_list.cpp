#include "action_list.h"
#include "nel/gui/action_handler.h"
#include <vector>
#include <string>

ActionList::ActionList( QDialog *parent ) :
QDialog( parent )
{
	setupUi( this );
}

ActionList::~ActionList()
{
}

void ActionList::load()
{
	actionList->clear();

	NLGUI::CAHManager *am = NLGUI::CAHManager::getInstance();
	std::vector< std::string > handlers;
	am->getActionHandlers( handlers );

	std::vector< std::string >::const_iterator itr = handlers.begin();
	while( itr != handlers.end() )
	{
		actionList->addItem( itr->c_str() );
		++itr;
	}
}


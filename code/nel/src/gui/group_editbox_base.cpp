#include "nel/gui/group_editbox_base.h"

CGroupEditBoxBase *CGroupEditBoxBase::_CurrSelection = NULL;

CGroupEditBoxBase::CGroupEditBoxBase( const TCtorParam &param ) :
CInterfaceGroup( param )
{
	_RecoverFocusOnEnter = true;
}

CGroupEditBoxBase::~CGroupEditBoxBase()
{
}


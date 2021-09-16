#ifndef VIEW_POINTER_RYZOM_H
#define VIEW_POINTER_RYZOM_H


#include "nel/gui/view_pointer.h"

class CViewPointerRyzom : public CViewPointer
{
public:
	DECLARE_UI_CLASS( CViewPointerRyzom )
	CViewPointerRyzom( const TCtorParam &param );
	~CViewPointerRyzom();

	static void forceLinking();

private:
	bool drawResizer(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawRotate(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawScale(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawColorPicker(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawLink(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawBrowse(CCtrlBase* pCB, NLMISC::CRGBA col);
	bool drawPan(CCtrlBase* pCB, NLMISC::CRGBA col);

};



#endif



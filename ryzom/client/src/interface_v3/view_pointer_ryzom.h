#ifndef VIEW_POINTER_RYZOM_H
#define VIEW_POINTER_RYZOM_H


#include "nel/gui/view_pointer.h"

class CViewPointerRyzom : public CViewPointer
{
public:
	DECLARE_UI_CLASS( CViewPointerRyzom )
	CViewPointerRyzom( const TCtorParam &param );
	~CViewPointerRyzom() NL_OVERRIDE;

	static void forceLinking();

private:
	bool drawResizer(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawRotate(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawScale(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawColorPicker(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawLink(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawBrowse(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;
	bool drawPan(CCtrlBase* pCB, NLMISC::CRGBA col) NL_OVERRIDE;

};



#endif



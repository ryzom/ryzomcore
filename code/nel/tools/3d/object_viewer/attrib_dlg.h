// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#if !defined(AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_)
#define AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


#include "nel/misc/rgba.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/ps_attrib_maker.h"
#include "editable_range.h"
#include "popup_notify.h"
#include "particle_workspace.h"

namespace NL3D
{
	class CPSAttribMakerBase;
}

using NLMISC::CRGBA;



class CEditAttribDlg;


/////////////////////////////////////////////////////////////////////////////
// CAttribDlg dialog

class CAttribDlg : public CDialog, public IPopupNotify
{
// Construction
public:
	/** construct the dialog
	  * \param : valueID : an unique id for this dialog
	  * \param : enableConstantValue when false, only a scheme is available
	  */
	CAttribDlg(const std::string &valueID, CParticleWorkspace::CNode *ownerNode, bool enableConstantValue = true);   // standard constructor
	CAttribDlg::~CAttribDlg();
	

	/// must be called before init, disable constant value usage
	void disableConstantValue(void) { _EnableConstantValue = false; }

	// close window if being edited
	void closeEditWindow();

// Dialog Data
	//{{AFX_DATA(CAttribDlg)
	enum { IDD = IDD_ATTRIB_DLG };
	CButton	m_PutScheme;
	CButton	m_GetScheme;
	CButton	m_EditUserParam;
	CComboBox	m_SchemeInput;
	CStatic	m_CstValuePos;
	CStatic	m_NbCyclePos;
	CButton	m_AttrBitmap;
	CButton	m_ClampCtrl;
	CButton	m_EditScheme;
	CComboBox	m_UseScheme;
	CComboBox	m_Scheme;
	CString	m_AttribName;
	BOOL	m_Clamp;
	//}}AFX_DATA


	/// init the dialog with the given bitmap
	void init(HBITMAP bitmap, sint x, sint y, CWnd *pParent = NULL);

	// force to update dialog content
	void update();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAttribDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL	

public:

	/// enable the srcInput
	void enableSrcInput(bool enable = true) { _SrcInputEnabled = enable; }
	bool isSrcInputEnabled() const { return _SrcInputEnabled; }
	

	/** Disable the possibility to choose a scheme that has memory. (for example, a scheme for lifetime of a located has no sense
	  * because located have already some memory to store it)
	  */
	void enableMemoryScheme(bool enabled = true) { _DisableMemoryScheme = !enabled; }

	/** tells whether memory schemes are enables
	  * \see enableMemoryScheme()
	  */
	bool isMemorySchemeEnabled() const { return !_DisableMemoryScheme; }

	/// enable Nb Cycle tuning
	void	enableNbCycles(bool enabled = true) { _NbCycleEnabled = enabled; }
	bool    isNbCycleEnabled() const { return _NbCycleEnabled; }


public:
	// private usage (not private because accessed by a static function) : return the nbCycles parameter of the scheme (e.g the input multiplier).
	virtual float getSchemeNbCycles(void) const = 0;

	// private usage (not private because accessed by a static function) : set the nbCycles parameter of the scheme (e.g the input multiplier)
	virtual void setSchemeNbCycles(float nbCycles) = 0;

	BOOL EnableWindow( BOOL bEnable = TRUE );

// Implementation
protected:
	
	/// bool : true is src input are allowed
	bool _SrcInputEnabled;

	// true if constant values are allowed
	bool _EnableConstantValue;

	// change the dialog for constant values
	void cstValueUpdate();

	/// enable / disable the 'edit input' button, when input can be edited
	void inputValueUpdate(void);


	// toggle back from scheme to cst value
	virtual void resetCstValue(void) = 0;

	// change the dialog for scheme usage
	void schemeValueUpdate();

	// return true if a scheme is used
	virtual bool useScheme(void) const = 0;


	// get the number of attributes makers  (schemes) that are available
	virtual uint getNumScheme(void) const = 0;

	// get the text associated with an attribute maker
	virtual std::string getSchemeName(uint index) const = 0;

	// edit the current scheme. And return a window on it
	virtual CWnd *editScheme(void) = 0;

	// set the current scheme
	virtual void setCurrentScheme(uint index) = 0;

	// set the current scheme ptr
	virtual void setCurrentSchemePtr(NL3D::CPSAttribMakerBase *) = 0;

	// get the current scheme, -1 if the scheme is unknow (created outside the editor ?)
	virtual sint getCurrentScheme(void) const = 0;

	/// get a pointer on the current scheme base class
	virtual NL3D::CPSAttribMakerBase *getCurrentSchemePtr(void) const = 0;


	// this must return the right dialog for a constant value (created with nex)
	virtual CEditAttribDlg *createConstantValueDlg() = 0;


	// tells whether the scheme supports custom input
	virtual bool hasSchemeCustomInput(void) const = 0;
	// retrieve the scheme input id
	virtual NL3D::CPSInputType getSchemeInput(void) const = 0;
	// set the scheme input id
	virtual void setSchemeInput(const NL3D::CPSInputType &input) = 0;

	// tells whether the scheme input value is clamped or not
	virtual bool isSchemeClamped(void) const = 0;
	// clamp / unclamp the scheme
	virtual void clampScheme(bool clamped = true) = 0;
	// return true if clamping is supported
	virtual bool isClampingSupported(void) const = 0;

	/// inherited from IPopupNotify
	virtual void childPopupClosed(CWnd *child);


	// the dialog used to tune the nb cycles param (when available)
	CEditableRangeFloat *_NbCyclesDlg;	
	// this is equal to true when memory schemes are not permitted
	bool _DisableMemoryScheme;
	/// true to enable 'nb cycles' control
	bool _NbCycleEnabled;
	// wrapper to tune the number of cycles
	struct CNbCyclesWrapper : public IPSWrapperFloat
	{
			CAttribDlg *Dlg;
			float get(void) const { return Dlg->getSchemeNbCycles(); }
			void set(const float &v) { Dlg->setSchemeNbCycles(v); }
	} _NbCyclesWrapper;	
	// true when created, it is set to false once a constant, or scheme dialog has bee shown
	bool _FirstDrawing;
	// the dialog used to tune a constant value
	CEditAttribDlg *_CstValueDlg;

	// the current dialog for scheme edition
	CWnd		   *_SchemeEditionDlg;
	std::string		_ValueID;
	CParticleWorkspace::CNode *_Node;

	// Generated message map functions
	//{{AFX_MSG(CAttribDlg)
	afx_msg void OnSelchangeUseScheme();
	afx_msg void OnSelchangeScheme();
	afx_msg void OnEditScheme();
	afx_msg void OnSelchangeSchemeInput();
	afx_msg void OnClampAttrib();
	afx_msg void OnEditInput();
	afx_msg void OnGetScheme();
	afx_msg void OnPutScheme();
	afx_msg void OnDestroy();
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};








/**
 * a template class that helps to specialize the attrib maker edition dialog with various types 
 */
template <typename T> class CAttribDlgT : public CAttribDlg
{	
public:
	CAttribDlgT(const std::string &valueID, CParticleWorkspace::CNode *node) : CAttribDlg(valueID, node),
																			   _Wrapper(NULL),
																			   _SchemeWrapper(NULL)																			   
	{
	}		
	void setWrapper(IPSWrapper<T> *wrapper) { nlassert(wrapper); _Wrapper = wrapper; _Wrapper->OwnerNode = _Node; }
	void setSchemeWrapper(IPSSchemeWrapper<T> *schemeWrapper) { nlassert(schemeWrapper); _SchemeWrapper = schemeWrapper; _SchemeWrapper->OwnerNode = _Node; }
	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const = 0;	
	virtual std::string getSchemeName(uint index) const = 0;	
	virtual CWnd *editScheme(void) = 0;	
	virtual void setCurrentScheme(uint index) = 0;
	virtual sint getCurrentScheme(void) const  = 0;

	virtual void resetCstValue(void) 
	{ 
		_Wrapper->setAndUpdateModifiedFlag(_Wrapper->get()); // reuse current color 
	}

	virtual bool hasSchemeCustomInput(void) const { return _SchemeWrapper->getScheme()->hasCustomInput(); }
	virtual NL3D::CPSInputType getSchemeInput(void) const { return  _SchemeWrapper->getScheme()->getInput(); }	
	virtual void setSchemeInput(const NL3D::CPSInputType &input) { _SchemeWrapper->getScheme()->setInput(input); }


	virtual float getSchemeNbCycles(void) const { return _SchemeWrapper->getScheme()->getNbCycles(); }
	virtual void setSchemeNbCycles(float nbCycles) { _SchemeWrapper->getScheme()->setNbCycles(nbCycles); }


	
	virtual bool isSchemeClamped(void) const { return _SchemeWrapper->getScheme()->getClamping(); }
	virtual void clampScheme(bool clamped = true) { _SchemeWrapper->getScheme()->setClamping(clamped); }	
	virtual bool isClampingSupported(void) const { return _SchemeWrapper->getScheme()->isClampingSupported(); };
	virtual NL3D::CPSAttribMakerBase *getCurrentSchemePtr(void) const { return _SchemeWrapper->getScheme(); }
	virtual void setCurrentSchemePtr(NL3D::CPSAttribMakerBase *s) 
	{ 
		_SchemeWrapper->setSchemeAndUpdateModifiedFlag(NLMISC::safe_cast<NL3D::CPSAttribMaker<T> *>(s));
	}


protected:
	virtual bool useScheme(void) const
	{
		nlassert(_SchemeWrapper);
		return(_SchemeWrapper->getScheme() != NULL);
	}
public:
	// wrapper to set/get a constant float
	IPSWrapper<T> *_Wrapper;
	// wrapper to set/get a scheme
	IPSSchemeWrapper<T> *_SchemeWrapper	;	
};

/** an attribute editor specialized for float values
 */

class CAttribDlgFloat : public CAttribDlgT<float>, public CBoundCheckerFloat
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 *  \param minValue : the min value for the editable range dlg(for constant value)
	 *  \param maxValue : the min value for the editable range dlg (for constant value)
	 */
	CAttribDlgFloat(const std::string &valueID, CParticleWorkspace::CNode *node, float minValue = 0, float maxValue = 10);

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const;	
	virtual std::string getSchemeName(uint index) const;	
	virtual CWnd *editScheme(void);	
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;



protected:

	virtual CEditAttribDlg *createConstantValueDlg();
	// ID for the cst float value  edition dialog
	std::string _CstValueId;
	float _MinRange, _MaxRange;
};


/** an attribute editor specialized for unsigned int values
 */

class CAttribDlgUInt : public CAttribDlgT<uint32>, public CBoundCheckerUInt
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 *  \param minValue : the min value for the editable range dlg(for constant value)
	 *  \param maxValue : the min value for the editable range dlg (for constant value)
	 */
	CAttribDlgUInt(const std::string &valueID, CParticleWorkspace::CNode *node, uint32 minValue = 0, uint32 maxValue = 10);

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const;	
	virtual std::string getSchemeName(uint index) const;	
	virtual CWnd *editScheme(void);	
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;



protected:

	virtual CEditAttribDlg *createConstantValueDlg();
	// ID for the cst float value  edition dialog
	std::string _CstValueId;
	uint32 _MinRange, _MaxRange;
};



/** an attribute editor specialized for signed int values
 */

class CAttribDlgInt : public CAttribDlgT<sint32>, public CBoundCheckerInt
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 *  \param minValue : the min value for the editable range dlg(for constant value)
	 *  \param maxValue : the min value for the editable range dlg (for constant value)
	 */
	CAttribDlgInt(const std::string &valueID, CParticleWorkspace::CNode *node, sint32 minValue = 0, sint32 maxValue = 10);

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const;	
	virtual std::string getSchemeName(uint index) const;	
	virtual CWnd *editScheme(void);	
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;



protected:

	virtual CEditAttribDlg *createConstantValueDlg();
	// ID for the cst float value  edition dialog
	std::string _CstValueId;
	sint32 _MinRange, _MaxRange;
};




/** an attribute editor specialized for RGB values
 */

class CAttribDlgRGBA : public CAttribDlgT<CRGBA>
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 */
	CAttribDlgRGBA(const std::string &valueID, CParticleWorkspace::CNode *node);

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const;	
	virtual std::string getSchemeName(uint index) const;	
	virtual CWnd *editScheme(void);	
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;



protected:

	virtual CEditAttribDlg *createConstantValueDlg();
	// ID for the cst float value  edition dialog
	std::string _CstValueId;	
};


/** an attribute editor specialized for plane basis values
 */

class CAttribDlgPlaneBasis : public CAttribDlgT<NL3D::CPlaneBasis>
{	
public:
	/** ctor
	 *  \param valueID an unique id for the constant value editable range dialog
	 */
	CAttribDlgPlaneBasis(const std::string &valueID, CParticleWorkspace::CNode *node);

	
	// inherited from CAttribDlg
	virtual uint getNumScheme(void) const;	
	virtual std::string getSchemeName(uint index) const;	
	virtual CWnd *editScheme(void);	
	virtual void setCurrentScheme(uint index);
	virtual sint getCurrentScheme(void) const;



protected:

	virtual CEditAttribDlg *createConstantValueDlg();
	// ID for the cst float value  edition dialog
	std::string _CstValueId;	
};



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ATTRIB_DLG_H__DF35743E_C8B4_4218_9C42_A3DF4126BEF2__INCLUDED_)

// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2010-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// attrib_dlg.cpp : implementation file
//



#include "std_afx.h"
#include "object_viewer.h"
#include "attrib_dlg.h"
#include "editable_range.h"
#include "color_edit.h"
#include "basis_edit.h"
#include "value_blender_dlg.h"
#include "value_gradient_dlg.h"
#include "value_from_emitter_dlg.h"
#include "bin_op_dlg.h"
#include "edit_user_param.h"
#include "edit_spinner.h"
#include "edit_follow_path.h"
#include "scheme_bank_dlg.h"
#include "scheme_manager.h"
#include "choose_name.h"
#include "curve_edit.h"


#include "nel/3d/ps_attrib_maker.h"
#include "nel/3d/ps_float.h"
#include "nel/3d/ps_int.h"
#include "nel/3d/ps_color.h"
#include "nel/3d/ps_plane_basis.h"
#include "nel/3d/ps_plane_basis_maker.h"




/*static char trace_buf[200];
#define NL_TRACE sprintf(trace_buf, "%d", __LINE__); \
				::MessageBox(NULL, trace_buf, NULL, MB_OK);
*/

/////////////////////////////////////////////////////////////////////
// WRAPPERS to set / retrieve the NbCycles parameter of a scheme   //
/////////////////////////////////////////////////////////////////////
static float NbCyclesReader(void *lParam) { return ((CAttribDlg *) lParam)->getSchemeNbCycles(); }
static void NbCyclesWriter(float value, void *lParam) { ((CAttribDlg *) lParam)->setSchemeNbCycles(value); }



///////////////////////////////////////////
// GENERAL INTERFACE FOR BLENDER EDITION //
///////////////////////////////////////////


/**  T is the type to be edited (color, float, etc..),
 *   even if it is unused
 */

template <typename T> 
class CValueBlenderDlgClientT : public IValueBlenderDlgClient
{
	public:
		std::string Id; // the Id of each of the dialog (it will be followed by %1 or %2)
						 // must be filled by the user
		// the scheme being used. Must be set by the user
		NL3D::CPSValueBlendFuncBase<T> *SchemeFunc;

	protected:
		virtual CEditAttribDlg *createDialog(uint index, CParticleWorkspace::CNode *ownerNode)
		{
			std::string id = Id;
			if (index == 0) id += "%1"; else id += "%2";
			_ValueInfos[index].ValueIndex = index;
			_ValueInfos[index].SchemeFunc = SchemeFunc;
			_ValueInfos[index].OwnerNode = ownerNode;
			return newDialog(id, &_ValueInfos[index]);									
		}


		// construct a dialog with the given wrapper and id
		virtual CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<T> *wrapper) = 0;

		// inherited from IPSWrapper<T>
		struct COneValueInfo : public IPSWrapper<T>
		{
			// value 0 or 1 being edited
			uint ValueIndex;
			// the scheme being edited
			NL3D::CPSValueBlendFuncBase<T> *SchemeFunc;

			virtual T get(void) const 
			{ 
				T t1, t2;
				SchemeFunc->getValues(t1, t2);
				return ValueIndex == 0 ? t1 : t2;
			}			
			virtual void set(const T &value)
			{
				T t1, t2;
				SchemeFunc->getValues(t1, t2);
				if (ValueIndex == 0 ) t1 = value; else t2 = value;
				SchemeFunc->setValues(t1, t2);
			}
		};

		COneValueInfo _ValueInfos[2];
};


////////////////////////////////////////////
// GENERAL INTERFACE FOR GRADIENT EDITION //
////////////////////////////////////////////

/** This template generate an interface that is used with the gradient edition dialog
 *  T is the type to be edited (color, floet, etc..)
 */
 

template <typename T> 
class CValueGradientDlgClientT : public IValueGradientDlgClient, public IPSWrapper<T>
{
public:

	std::string Id; // the Id of each of the dialog (it will be followed by %1 or %2)
						 // must be filled by the user
	// the gradient being edited, must be filled by the instancier
	NL3D::CPSValueGradientFunc<T> *Scheme;
	// the gradient dialog, must be filled by the instancier
	CValueGradientDlg *GradDlg;
	// the difault value for new values creation. Must be filled by the instancier
	T DefaultValue;

	/// a function that can display a value in a gradient, with the given offset. Deriver must define this
	virtual void displayValue(CDC *dc, uint index, sint x, sint y)  = 0;

	
	/// inherited from IPSWrapper
	virtual T get(void) const { return Scheme->getValue(_CurrentEditedIndex); }
	virtual void set(const T &v)
	{
		T *tab = new T[Scheme->getNumValues()];
		Scheme->getValues(tab);
		tab[_CurrentEditedIndex] = v;
		Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
		delete[] tab;
		GradDlg->invalidateGrad();
	}
	
	/** must provide a dialog for the edition of one value (only ONE exist at a time)
	 * \param index the index of the value in the dialog
	 * \grad the dlg that called this method (deriver can ask a redraw then)
	 */
	virtual CEditAttribDlg *createDialog(uint index, CValueGradientDlg *grad, CParticleWorkspace::CNode *ownerNode)
	{					
		OwnerNode = ownerNode;
		_CurrentEditedIndex = index;
		return newDialog(Id, this);
	}

	/// create a new dialog with given id and wrapper
	virtual  CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<T> *wrapper) = 0;
	

	/// a function that can add, remove, or insert a new element in the gradient
	virtual void modifyGradient(TAction action, uint index)
	{
		
		T *tab = new T[Scheme->getNumValues() + 1]; // +1 is for the add / insert case
		Scheme->getValues(tab);

		switch(action)
		{
			case IValueGradientDlgClient::Add:
				tab[Scheme->getNumValues()] = DefaultValue;
				Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages());
			break;
			case IValueGradientDlgClient::Insert:
				::memmove(tab + (index + 1), tab + index, sizeof(T) * (Scheme->getNumValues() - index));
				tab[index] = DefaultValue;
				Scheme->setValues(tab, Scheme->getNumValues() + 1, Scheme->getNumStages());
			break;
			case IValueGradientDlgClient::Delete:
				::memmove(tab + index, tab + index + 1, sizeof(T) * (Scheme->getNumValues() - index - 1));
				Scheme->setValues(tab, Scheme->getNumValues() - 1, Scheme->getNumStages());
			break;
			case IValueGradientDlgClient::Up:
				nlassert(index > 0);
				std::swap(tab[index], tab[index - 1]);
				Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
			break;
			case IValueGradientDlgClient::Down:
				nlassert(index <  Scheme->getNumValues() - 1);
				std::swap(tab[index], tab[index + 1]);
				Scheme->setValues(tab, Scheme->getNumValues(), Scheme->getNumStages());
			break;
		}

		delete[] tab;
	}		
	virtual uint32 getSchemeSize(void) const { return Scheme->getNumValues(); }

	// get the number of interpolation step
	uint32 getNbSteps(void) const
	{
		return Scheme->getNumStages();
	}

	// set the number of interpolation steps
	void setNbSteps(uint32 value)
	{
		Scheme->setNumStages(value);	
	}

protected:
		// index of the value OF the current dialog that exist
		uint32 _CurrentEditedIndex;	
};



/////////////////////////////////////////////////////////////////////////////
// CAttribDlg dialog


//*************************************************************************************************************
CAttribDlg::CAttribDlg(const std::string &valueID, CParticleWorkspace::CNode *ownerNode, bool enableConstantValue /* = true*/)
	 : _CstValueDlg(NULL),
	   _Node(ownerNode),
	   _FirstDrawing(true),
	   _EnableConstantValue(enableConstantValue),
	   _DisableMemoryScheme(false),
	   _SchemeEditionDlg(NULL), 
	   _NbCycleEnabled(true), 
	   _NbCyclesDlg(NULL), 
	   _ValueID(valueID), 
	   _SrcInputEnabled(true)
{
	//{{AFX_DATA_INIT(CAttribDlg)
	m_AttribName = _T("");
	m_Clamp = FALSE;
	//}}AFX_DATA_INIT	
}


//*************************************************************************************************************
void CAttribDlg::closeEditWindow()
{
	childPopupClosed(NULL);
}


//*************************************************************************************************************
BOOL CAttribDlg::EnableWindow( BOOL bEnable)
{
	if (_CstValueDlg)
	{
		_CstValueDlg->EnableWindow(bEnable);
	}

	if (_NbCyclesDlg)
	{
		_NbCyclesDlg->EnableWindow(bEnable);
	}
	m_UseScheme.EnableWindow(bEnable);
	m_AttrBitmap.EnableWindow(bEnable);

	if (useScheme())
	{
		m_Scheme.EnableWindow(bEnable);
		m_SchemeInput.EnableWindow(hasSchemeCustomInput() ? bEnable : FALSE);
		m_EditScheme.EnableWindow(bEnable);
		m_GetScheme.EnableWindow(bEnable);
		m_PutScheme.EnableWindow(bEnable);
		m_ClampCtrl.EnableWindow(bEnable);
	}
	else
	{		
		m_ClampCtrl.EnableWindow(FALSE);
	}

	UpdateData(FALSE);

	return CDialog::EnableWindow(bEnable);
}

//*************************************************************************************************************
CAttribDlg::~CAttribDlg()	
{
	if (_NbCyclesDlg)
	{
		_NbCyclesDlg->DestroyWindow();
		delete _NbCyclesDlg;
	}
	if (_CstValueDlg)
	{
		_CstValueDlg->DestroyWindow();		
		delete _CstValueDlg;	
	}
}

//*************************************************************************************************************
void CAttribDlg::update()
{
	_FirstDrawing = true;
	if (useScheme())
	{
		
		schemeValueUpdate();
	}
	else
	{
		
		nlassert(_EnableConstantValue);
		cstValueUpdate();
	}
}

//*************************************************************************************************************
void CAttribDlg::init(HBITMAP bitmap, sint x, sint y, CWnd *pParent)
{

	Create(IDD_ATTRIB_DLG, pParent);
	RECT r, ro;
	GetClientRect(&r);
	
	m_AttrBitmap.SendMessage(BM_SETIMAGE, IMAGE_BITMAP, (LPARAM) bitmap);
	MoveWindow(x, y, r.right, r.bottom);

	m_NbCyclePos.GetWindowRect(&r);
	GetWindowRect(&ro);

	if (_NbCycleEnabled)
	{
		_NbCyclesDlg = new CEditableRangeFloat(_ValueID + "%%NB_CYCLE_INFO", _Node, 0.1f, 10.1f);
		_NbCyclesDlg->init(r.left - ro.left, r.top - ro.top, this);
	}

	// fill the combo box with the list of available scheme
	m_Scheme.InitStorage(getNumScheme(), 32); // 32 char per string pre-allocated

	for (uint k = 0; k < getNumScheme(); ++k)
	{	

		m_Scheme.InsertString(k, nlUtf8ToTStr(getSchemeName(k)));

	}

	update();	

	if (!_EnableConstantValue)
	{
		m_UseScheme.ShowWindow(SW_HIDE);
	}

	if (!_NbCyclesDlg)
	{
		GetDlgItem(IDC_INPUT_MULTIPLIER_TXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_CLAMP_ATTRIB)->ShowWindow(SW_HIDE);		
	}

	if (!_SrcInputEnabled)
	{
		GetDlgItem(IDC_SRC_INPUT_TXT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_EDIT_INPUT)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SCHEME_INPUT)->ShowWindow(SW_HIDE);
	}
	inputValueUpdate();
	ShowWindow(SW_SHOW);
}

//*************************************************************************************************************
void CAttribDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAttribDlg)
	DDX_Control(pDX, IDC_PUT_SCHEME, m_PutScheme);
	DDX_Control(pDX, IDC_GET_SCHEME, m_GetScheme);
	DDX_Control(pDX, IDC_EDIT_INPUT, m_EditUserParam);
	DDX_Control(pDX, IDC_SCHEME_INPUT, m_SchemeInput);
	DDX_Control(pDX, IDC_CONSTANT_VALUE_POS, m_CstValuePos);
	DDX_Control(pDX, IDC_ATTRIB_NB_CYCLES, m_NbCyclePos);
	DDX_Control(pDX, IDC_ATTR_BITMAP, m_AttrBitmap);
	DDX_Control(pDX, IDC_CLAMP_ATTRIB, m_ClampCtrl);
	DDX_Control(pDX, IDC_EDIT_SCHEME, m_EditScheme);
	DDX_Control(pDX, IDC_USE_SCHEME, m_UseScheme);
	DDX_Control(pDX, IDC_SCHEME, m_Scheme);
	DDX_Check(pDX, IDC_CLAMP_ATTRIB, m_Clamp);
	//}}AFX_DATA_MAP
}

//*************************************************************************************************************
void CAttribDlg::inputValueUpdate(void)
{
	if (useScheme() && getSchemeInput().InputType == NL3D::CPSInputType::attrUserParam)
	{
		m_EditUserParam.EnableWindow(TRUE);
	}
	else
	{
		m_EditUserParam.EnableWindow(FALSE);
	}
}

//*************************************************************************************************************
void CAttribDlg::cstValueUpdate()
{
	if (!_FirstDrawing && !useScheme()) return;	

	m_ClampCtrl.EnableWindow(FALSE);
	if (_NbCyclesDlg)
	{
		_NbCyclesDlg->EnableWindow(FALSE);
		_NbCyclesDlg->emptyDialog();
	}
	GetDlgItem(IDC_INPUT_MULTIPLIER_TXT)->EnableWindow(FALSE);
	m_EditScheme.EnableWindow(FALSE);
	m_PutScheme.EnableWindow(FALSE);
	m_GetScheme.EnableWindow(FALSE);

	m_EditScheme.ShowWindow(SW_HIDE);
	m_GetScheme.ShowWindow(SW_HIDE);
	m_PutScheme.ShowWindow(SW_HIDE);

	m_Scheme.EnableWindow(FALSE);
	m_Scheme.ShowWindow(SW_HIDE);
	m_SchemeInput.EnableWindow(FALSE);
	m_SchemeInput.ShowWindow(SW_HIDE);

	if (!_FirstDrawing) resetCstValue();			



	m_UseScheme.SetCurSel(0);
	_CstValueDlg = createConstantValueDlg();
	CRect r, ro;
	m_CstValuePos.GetWindowRect(&r);
	GetWindowRect(&ro);
	_CstValueDlg->init(r.left - ro.left, r.top - ro.top, this);
	UpdateData(FALSE);

	_FirstDrawing = false;
}

//*************************************************************************************************************
void CAttribDlg::schemeValueUpdate()
{
	//if (!_FirstDrawing && useScheme()) return;		
	if (_CstValueDlg)
	{
		_CstValueDlg->DestroyWindow();
		delete _CstValueDlg;
		_CstValueDlg = NULL;
	}	
	m_EditScheme.EnableWindow(TRUE);
	m_GetScheme.EnableWindow(TRUE);
	m_PutScheme.EnableWindow(TRUE);
	m_EditScheme.ShowWindow(SW_SHOW);
	m_GetScheme.ShowWindow(SW_SHOW);
	m_PutScheme.ShowWindow(SW_SHOW);	
	m_Scheme.EnableWindow(TRUE);
	m_Scheme.ShowWindow(SW_SHOW);
	m_SchemeInput.EnableWindow(TRUE);
	m_SchemeInput.ShowWindow(SW_SHOW);
	m_UseScheme.SetCurSel(1);
	sint k = getCurrentScheme();

	if (k == -1) // unknow scheme ...
	{
		setCurrentScheme(0);
		k = 0;
	}
	m_Scheme.SetCurSel(k);
	if (hasSchemeCustomInput())
	{
		m_SchemeInput.EnableWindow();
		m_SchemeInput.SetCurSel((uint) getSchemeInput().InputType);
		inputValueUpdate();
		if (_NbCyclesDlg)
		{
			_NbCyclesDlg->EnableWindow(TRUE);
		}
		m_ClampCtrl.EnableWindow(isClampingSupported());
		GetDlgItem(IDC_INPUT_MULTIPLIER_TXT)->EnableWindow(isClampingSupported());
	}
	else
	{
		m_SchemeInput.EnableWindow(FALSE);
		m_SchemeInput.SetCurSel(0);
		if (_NbCyclesDlg)
		{
			_NbCyclesDlg->EnableWindow(FALSE);
		}
		m_ClampCtrl.EnableWindow(FALSE);
		GetDlgItem(IDC_INPUT_MULTIPLIER_TXT)->EnableWindow(FALSE);
	}	

	if (_NbCyclesDlg)
	{	
		_NbCyclesWrapper.OwnerNode = _Node;
		_NbCyclesDlg->setWrapper(&_NbCyclesWrapper);
		_NbCyclesWrapper.Dlg = this;	
		_NbCyclesDlg->updateRange();
		_NbCyclesDlg->updateValueFromReader();
	}	
	if (isClampingSupported())
	{
		m_Clamp = isSchemeClamped();
	}
	UpdateData(FALSE);
	_FirstDrawing = false;
}

//*************************************************************************************************************
void CAttribDlg::OnSelchangeUseScheme() 
{
	if (m_UseScheme.GetCurSel() == 0)
	{
		cstValueUpdate();
	}
	else
	{
		schemeValueUpdate();
	}
}



//*************************************************************************************************************
void CAttribDlg::OnSelchangeScheme() 
{
	UpdateData();
	setCurrentScheme(m_Scheme.GetCurSel());	
	schemeValueUpdate();
}

//*************************************************************************************************************
void CAttribDlg::OnEditScheme() 
{
	_SchemeEditionDlg  = editScheme();
	if (_SchemeEditionDlg)
	{
		EnableWindow(FALSE);
	}
}

//*************************************************************************************************************
void CAttribDlg::childPopupClosed(CWnd *child)
{
	EnableWindow(TRUE);
	if (!_SchemeEditionDlg) return;
	_SchemeEditionDlg->DestroyWindow();
	delete _SchemeEditionDlg;
	_SchemeEditionDlg = NULL;	
}

//*************************************************************************************************************
void CAttribDlg::OnGetScheme() 
{
	CSchemeBankDlg sbd(getCurrentSchemePtr()->getType(), this);
	if (sbd.DoModal() == IDOK && sbd.getSelectedScheme())
	{
		setCurrentSchemePtr(sbd.getSelectedScheme()->clone());	
		_FirstDrawing = true;
		schemeValueUpdate();
	}
}

//*************************************************************************************************************
void CAttribDlg::OnPutScheme() 
{
	CChooseName cn("new scheme", this);
	if (cn.DoModal() == IDOK)
	{
		SchemeManager.insertScheme(cn.getName(), getCurrentSchemePtr()->clone());
	}
}

//*************************************************************************************************************
void CAttribDlg::OnSelchangeSchemeInput() 
{
	UpdateData();
	NL3D::CPSInputType it;
	it.InputType = (NL3D::CPSInputType::TInputType) m_SchemeInput.GetCurSel(); 
	if (it.InputType == NL3D::CPSInputType::attrUserParam)
	{
		it.UserParamNum = 0;
	}	
	setSchemeInput(it);
	inputValueUpdate();
}

//*************************************************************************************************************
void CAttribDlg::OnClampAttrib() 
{
	UpdateData();
	clampScheme(m_Clamp ? true : false /* avoid performance warning */);
}

//*************************************************************************************************************
void CAttribDlg::OnEditInput() 
{
	switch (getSchemeInput().InputType)
	{
		case NL3D::CPSInputType::attrUserParam:
		{
			CEditUserParam ep(getSchemeInput().UserParamNum);
			if (ep.DoModal() == IDOK)
			{
				NL3D::CPSInputType it =  getSchemeInput();
				it.UserParamNum = ep.getUserParamIndex();
				setSchemeInput(it);
			}
		} 
		break;
	}	
}

BEGIN_MESSAGE_MAP(CAttribDlg, CDialog)
	//{{AFX_MSG_MAP(CAttribDlg)
	ON_CBN_SELCHANGE(IDC_USE_SCHEME, OnSelchangeUseScheme)
	ON_CBN_SELCHANGE(IDC_SCHEME, OnSelchangeScheme)
	ON_BN_CLICKED(IDC_EDIT_SCHEME, OnEditScheme)
	ON_CBN_SELCHANGE(IDC_SCHEME_INPUT, OnSelchangeSchemeInput)
	ON_BN_CLICKED(IDC_CLAMP_ATTRIB, OnClampAttrib)
	ON_BN_CLICKED(IDC_EDIT_INPUT, OnEditInput)
	ON_BN_CLICKED(IDC_GET_SCHEME, OnGetScheme)
	ON_BN_CLICKED(IDC_PUT_SCHEME, OnPutScheme)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAttribDlg message handlers



////////////////////////////////////
// CAttribDlgFloat implementation //
////////////////////////////////////

	//////////////////////////////////////////////////////////
	// FLOAT BLENDER EDITION INTERFACE						//
	//////////////////////////////////////////////////////////
	class CFloatBlenderDlgClient : public CValueBlenderDlgClientT<float>
	{
		public:
			CEditAttribDlg *newDialog(const std::string &id, IPSWrapperFloat *wrapper) 
			{ 
				CEditableRangeFloat *erf = new CEditableRangeFloat(id, wrapper->OwnerNode, MinRange, MaxRange);				
				erf->setWrapper(wrapper);
				BoundChecker.duplicateBoundChecker(*erf);
				return erf;
			}
			CBoundCheckerFloat BoundChecker;
			float MinRange, MaxRange;
	};

	//////////////////////////////////////////////////////////
	// FLOAT GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////

	//*************************************************************************************************************
	class CFloatGradientDlgWrapper : public CValueGradientDlgClientT<float>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			
			CString out;
			out.Format(_T("%g"),  Scheme->getValue(index) );
			dc->TextOut(x + 10, y + 4, out);
		}
		CEditAttribDlg *newDialog(const std::string &id, IPSWrapperFloat *wrapper) 
		{ 
			CEditableRangeFloat *erf = new CEditableRangeFloat(id, wrapper->OwnerNode, MinRange, MaxRange);
			erf->setWrapper(wrapper);
			BoundChecker.duplicateBoundChecker(*erf);
			return erf;
		}
		CBoundCheckerFloat BoundChecker;
		float MinRange, MaxRange;
	};



	//*************************************************************************************************************
	CAttribDlgFloat::CAttribDlgFloat(const std::string &valueID, CParticleWorkspace::CNode *node, float minRange, float maxRange)
				:  CAttribDlgT<float>(valueID, node), _MinRange(minRange), _MaxRange(maxRange)
	{
		_CstValueId = valueID;			
	}

	//*************************************************************************************************************
	CEditAttribDlg *CAttribDlgFloat::createConstantValueDlg()
	{
		CEditableRangeFloat *erf = new CEditableRangeFloat(_CstValueId, _Node, _MinRange, _MaxRange);
		erf->setWrapper(_Wrapper);		
		duplicateBoundChecker(*erf);
		return erf;
	}

	//*************************************************************************************************************
	uint CAttribDlgFloat::getNumScheme(void) const
	{	
		return _DisableMemoryScheme ? 3 : 5;		
	}

	//*************************************************************************************************************
	std::string CAttribDlgFloat::getSchemeName(uint index) const
	{
		const char *types[] = { "value blender", "values gradient", "curve", "value computed from emitter", "binary operator"};
		nlassert(index < 5);
		return std::string(types[index]);		
	}

	//*************************************************************************************************************
	CWnd *CAttribDlgFloat::editScheme(void)
	{
		NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<NL3D::CPSFloatBlender *>(scheme)) 
		{				
			CFloatBlenderDlgClient *myInterface = new CFloatBlenderDlgClient;
			this->duplicateBoundChecker(myInterface->BoundChecker);
			myInterface->MinRange = _MinRange;
			myInterface->MaxRange = _MaxRange;
			myInterface->Id = _CstValueId+ std::string("%%FLOAT_BLENDER");
			myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<float, 64> *) scheme)->_F;			
			CValueBlenderDlg *vb = new CValueBlenderDlg(myInterface, true, this, this, _Node);
			vb->init(this);
			return vb;
					
		}
		if (dynamic_cast<NL3D::CPSFloatGradient *>(scheme)) 
		{
			CFloatGradientDlgWrapper *wrapper = new CFloatGradientDlgWrapper;
			this->duplicateBoundChecker(wrapper->BoundChecker);
			wrapper->MinRange = _MinRange;
			wrapper->MaxRange = _MaxRange;
			wrapper->Scheme = &(((NL3D::CPSFloatGradient *) (_SchemeWrapper->getScheme()) )->_F);
			CValueGradientDlg *gd = new CValueGradientDlg(wrapper, _Node, true, this, this);
			wrapper->GradDlg = gd;
			wrapper->DefaultValue = 0.f;
			wrapper->Id = _CstValueId+ std::string("%%FLOAT GRADIENT");			
			gd->init(this);
			return gd;
		}
		if (dynamic_cast<NL3D::CPSFloatMemory *>(scheme)) 
		{
			CAttribDlgFloat *adf = new CAttribDlgFloat(_CstValueId, _Node, _MinRange, _MaxRange);
			this->duplicateBoundChecker(*adf);
			CValueFromEmitterDlgT<float> *vfe = new CValueFromEmitterDlgT<float>( (NL3D::CPSFloatMemory *)(scheme),
																				  adf,
																				  this,
																				  m_AttrBitmap.GetBitmap()
																				 );			
			vfe->init(this);
			return vfe;
		}
		if (dynamic_cast<NL3D::CPSFloatBinOp *>(scheme)) 
		{
			CAttribDlgFloat *ad[2] = { NULL, NULL};
			for (uint k = 0; k <2; ++k)
			{
				ad[k] = new CAttribDlgFloat(_CstValueId, _Node, _MinRange, _MaxRange);
				this->duplicateBoundChecker(*ad[k]);
			}
			CBinOpDlgT<float> *bod = new CBinOpDlgT<float>( (NL3D::CPSFloatBinOp *)(scheme),
															(CAttribDlgT<float> **) ad,
															this,
															m_AttrBitmap.GetBitmap());	
			bod->init(this);
			return bod;
		}

		if (dynamic_cast<NL3D::CPSFloatCurve *>(scheme)) 
		{
			CurveEdit *ce = new CurveEdit(&(dynamic_cast<NL3D::CPSFloatCurve *>(scheme)->_F), _Node, this, this);
			ce->init(this);
			return ce;			
		}

		return NULL;
	}

	//*************************************************************************************************************
	sint CAttribDlgFloat::getCurrentScheme(void) const
	{

		const NL3D::CPSAttribMaker<float> *scheme = _SchemeWrapper->getScheme();		
		if (dynamic_cast<const NL3D::CPSFloatBlender *>(scheme))  return 0;		
		if (dynamic_cast<const NL3D::CPSFloatGradient *>(scheme)) return 1;		
		if (dynamic_cast<const NL3D::CPSFloatCurve *>(scheme)) return 2;
		if (dynamic_cast<const NL3D::CPSFloatMemory *>(scheme)) return 3;		
		if (dynamic_cast<const NL3D::CPSFloatBinOp *>(scheme)) return 4;
		
		return -1;
	}


	//*************************************************************************************************************
	void CAttribDlgFloat::setCurrentScheme(uint index)
	{
		nlassert(index < 5);


		NL3D::CPSAttribMaker<float> *scheme = NULL;

		switch (index)
		{
			case 0:
				scheme = new NL3D::CPSFloatBlender(_MinRange, _MaxRange);
			break;
			case 1:	
			{
				static const float values[2] = { 0.1f, 1.f };
				scheme = new NL3D::CPSFloatGradient(values, 2, 16, 1.f);	
			}
			break;
			case 2:
			{				
				NL3D::CPSFloatCurve *curve = new NL3D::CPSFloatCurve;
				curve->_F.setNumSamples(128);
				curve->_F.addControlPoint(NL3D::CPSFloatCurveFunctor::CCtrlPoint(0, 0.5f));
				curve->_F.addControlPoint(NL3D::CPSFloatCurveFunctor::CCtrlPoint(1, 0.5f));				
				scheme = curve;
			}
			break;
			case 3:
				scheme = new NL3D::CPSFloatMemory;
				((NL3D::CPSAttribMakerMemory<float> *) scheme)->setScheme(new NL3D::CPSFloatBlender(_MinRange, _MaxRange));
			break;
			case 4 :
				scheme = new NL3D::CPSFloatBinOp;
				((NL3D::CPSFloatBinOp *) scheme)->setArg(0, new NL3D::CPSFloatBlender);
				((NL3D::CPSFloatBinOp *) scheme)->setArg(1, new NL3D::CPSFloatBlender);
			break;

			default:	
			break;
		}

		if (scheme)
		{
			_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
		}
	}


////////////////////////////////////
// CAttribDlgUInt implementation //
////////////////////////////////////


	class CUIntBlenderDlgClient : public CValueBlenderDlgClientT<uint32>
	{
		public:
			CEditAttribDlg *newDialog(const std::string &id, IPSWrapperUInt *wrapper) 
			{ 
				CEditableRangeUInt *eru = new CEditableRangeUInt(id, wrapper->OwnerNode, MinRange, MaxRange);
				eru->setWrapper(wrapper);
				BoundChecker.duplicateBoundChecker(*eru);
				return eru;
			}
			CBoundCheckerUInt BoundChecker;
			uint32 MinRange, MaxRange;
	};

	//////////////////////////////////////////////////////////
	// UINT GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	//*************************************************************************************************************
	class CUIntGradientDlgWrapper : public CValueGradientDlgClientT<uint32>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			
			CString out;
			out.Format(_T("%d"),  Scheme->getValue(index) );
			dc->TextOut(x + 10, y + 4, out);
		}
		CEditAttribDlg *newDialog(const std::string &id, IPSWrapperUInt *wrapper) 
		{ 
			CEditableRangeUInt *eru = new CEditableRangeUInt(id, wrapper->OwnerNode, MinRange, MaxRange);
			eru->setWrapper(wrapper);
			BoundChecker.duplicateBoundChecker(*eru);
			return eru;
		}
		CBoundCheckerUInt BoundChecker;
		uint32 MinRange, MaxRange;
	};


	//*************************************************************************************************************
	CAttribDlgUInt::CAttribDlgUInt(const std::string &valueID, CParticleWorkspace::CNode *node, uint32 minRange, uint32 maxRange)
				:  CAttribDlgT<uint32>(valueID, node), _MinRange(minRange), _MaxRange(maxRange)			  
	{
		_CstValueId = valueID;			
	}

	//*************************************************************************************************************
	CEditAttribDlg *CAttribDlgUInt::createConstantValueDlg()
	{
		CEditableRangeUInt *erf = new CEditableRangeUInt(_CstValueId, _Node, _MinRange, _MaxRange);
		erf->setWrapper(_Wrapper);
		duplicateBoundChecker(*erf);
		return erf;
	}

	//*************************************************************************************************************
	uint CAttribDlgUInt::getNumScheme(void) const
	{

		return _DisableMemoryScheme ? 2 : 4;
	}

	//*************************************************************************************************************
	std::string CAttribDlgUInt::getSchemeName(uint index) const
	{
		const char *types[] = { "value blender", "values gradient", "values computed from emitter", "binary operator" };
		nlassert(index < 4);
		return std::string(types[index]);		
	}

	//*************************************************************************************************************
	CWnd *CAttribDlgUInt::editScheme(void)
	{
		const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSUIntBlender *>(scheme)) 
		{				
			CUIntBlenderDlgClient *myInterface = new CUIntBlenderDlgClient ;
			duplicateBoundChecker(myInterface->BoundChecker);
			myInterface->MinRange = _MinRange;
			myInterface->MaxRange = _MaxRange;
			myInterface->Id = _CstValueId+ std::string("%%UINT_BLENDER");
			myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<uint32, 64> *) scheme)->_F;
			
			CValueBlenderDlg *vb = new CValueBlenderDlg(myInterface, true, this, this, _Node);
			vb->init(this);
			return vb;
		}
		if (dynamic_cast<const NL3D::CPSUIntGradient *>(scheme)) 
		{
			CUIntGradientDlgWrapper *wrapper = new CUIntGradientDlgWrapper;
			this->duplicateBoundChecker(wrapper->BoundChecker);
			wrapper->MinRange = _MinRange;
			wrapper->MaxRange = _MaxRange;
			wrapper->Scheme = &(((NL3D::CPSUIntGradient *) (_SchemeWrapper->getScheme()) )->_F);
			CValueGradientDlg *gd = new CValueGradientDlg(wrapper, _Node, true, this, this);
			wrapper->GradDlg = gd;
			wrapper->DefaultValue = 0;
			wrapper->Id = _CstValueId+ std::string("%%UINT GRADIENT");
			gd->init(this);
			return gd;
		}
		if (dynamic_cast<const NL3D::CPSUIntMemory *>(scheme)) 
		{

			CAttribDlgUInt *adu = new CAttribDlgUInt(_CstValueId, _Node, _MinRange, _MaxRange);
			this->duplicateBoundChecker(*adu);
			CValueFromEmitterDlgT<uint32> *vfe = new CValueFromEmitterDlgT<uint32>( (NL3D::CPSUIntMemory *)(scheme),
																					adu,
																					this,
																					m_AttrBitmap.GetBitmap());			
			vfe->init(this);
			return vfe;
		}
		if (dynamic_cast<const NL3D::CPSUIntBinOp *>(scheme)) 
		{
			CAttribDlgUInt *ad[2] = { NULL, NULL};
			for (uint k = 0; k <2; ++k)
			{
				ad[k] = new CAttribDlgUInt(_CstValueId, _Node, _MinRange, _MaxRange);
				this->duplicateBoundChecker(*ad[k]);
			}
			CBinOpDlgT<uint32> *bod = new CBinOpDlgT<uint32>( (NL3D::CPSUIntBinOp *)(scheme),
																(CAttribDlgT<uint32> **) ad,
																this,
																m_AttrBitmap.GetBitmap());	
			bod->init(this);
			return bod;
		}
		return NULL;
	}

	//*************************************************************************************************************
	sint CAttribDlgUInt::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<uint32> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSUIntBlender *>(scheme))  return 0;		
		if (dynamic_cast<const NL3D::CPSUIntGradient *>(scheme)) return 1;		
		if (dynamic_cast<const NL3D::CPSUIntMemory *>(scheme))   return 2;		
		if (dynamic_cast<const NL3D::CPSUIntBinOp *>(scheme))	 return 3;		
		return -1;
	}

	//*************************************************************************************************************
	void CAttribDlgUInt::setCurrentScheme(uint index)
	{
		nlassert(index < 4);


		NL3D::CPSAttribMaker<uint32> *scheme = NULL;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSUIntBlender(_MinRange, _MaxRange);
			break;
			case 1 :
				scheme = new NL3D::CPSUIntGradient;
			break;
			case 2 :
				scheme = new NL3D::CPSUIntMemory;
				((NL3D::CPSAttribMakerMemory<uint32> *) scheme)->setScheme(new NL3D::CPSUIntBlender(_MinRange, _MaxRange) );
			break;
			case 3 :
				scheme = new NL3D::CPSUIntBinOp;
				((NL3D::CPSUIntBinOp *) scheme)->setArg(0, new NL3D::CPSUIntBlender);
				((NL3D::CPSUIntBinOp *) scheme)->setArg(1, new NL3D::CPSUIntBlender);
			break;
			default:	
			break;
		}

		if (scheme)
		{
			_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
		}
	}


////////////////////////////////////
// CAttribDlgInt implementation //
////////////////////////////////////

	class CIntBlenderDlgClient : public CValueBlenderDlgClientT<sint32>
	{
		public:
			CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<sint32> *wrapper) 
			{ 
				CEditableRangeInt *eri = new CEditableRangeInt(id, wrapper->OwnerNode, MinRange, MaxRange);
				eri->setWrapper(wrapper);
				BoundChecker.duplicateBoundChecker(*eri);
				return eri;
			}
			CBoundCheckerInt BoundChecker;
			sint32 MinRange, MaxRange;
	};


	//////////////////////////////////////////////////////////
	// INT GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	//*************************************************************************************************************
	class CIntGradientDlgWrapper : public CValueGradientDlgClientT<sint32>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			
			CString out;
			out.Format(_T("%d"),  Scheme->getValue(index) );
			dc->TextOut(x + 10, y + 4, out);
		}
		CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<sint32> *wrapper) 
		{ 
			CEditableRangeInt *eri = new CEditableRangeInt(id, wrapper->OwnerNode, MinRange, MaxRange);
			eri->setWrapper(wrapper);
			BoundChecker.duplicateBoundChecker(*eri);
			return eri;
		}
		CBoundCheckerInt BoundChecker;
		sint32 MinRange, MaxRange;
	};


	//*************************************************************************************************************
	CAttribDlgInt::CAttribDlgInt(const std::string &valueID, CParticleWorkspace::CNode *node, sint32 minRange, sint32 maxRange)
				:  CAttribDlgT<sint32>(valueID, node), _MinRange(minRange), _MaxRange(maxRange)			  
	{
		_CstValueId = valueID;		
	}

	//*************************************************************************************************************
	CEditAttribDlg *CAttribDlgInt::createConstantValueDlg()
	{
		CEditableRangeInt *erf = new CEditableRangeInt(_CstValueId, _Node, _MinRange, _MaxRange);
		erf->setWrapper(_Wrapper);
		duplicateBoundChecker(*erf);
		return erf;
	}

	//*************************************************************************************************************
	uint CAttribDlgInt::getNumScheme(void) const
	{
		return _DisableMemoryScheme ? 2 : 4;
	}

	//*************************************************************************************************************
	std::string CAttribDlgInt::getSchemeName(uint index) const
	{

		const char *types[] = { "value exact blender", "values gradient", "values computed from emitter", "binary operator" };
		nlassert(index < 4);
		return std::string(types[index]);
	}

	//*************************************************************************************************************
	CWnd *CAttribDlgInt::editScheme(void)
	{
		const NL3D::CPSAttribMaker<sint32> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSIntBlender *>(scheme)) 
		{				
			CIntBlenderDlgClient *myInterface = new CIntBlenderDlgClient;
			this->duplicateBoundChecker(myInterface->BoundChecker);
			myInterface->MinRange = _MinRange;
			myInterface->MaxRange = _MaxRange;
			myInterface->Id = _CstValueId+ std::string("%%INT_BLENDER");
			myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<sint32, 64> *) scheme)->_F;
			
			CValueBlenderDlg *vb = new CValueBlenderDlg(myInterface, true, this, this, _Node);
			vb->init(this);
			return vb;
		}
		if (dynamic_cast<const NL3D::CPSIntGradient *>(scheme)) 
		{
			CIntGradientDlgWrapper *wrapper = new CIntGradientDlgWrapper;
			this->duplicateBoundChecker(wrapper->BoundChecker);
			wrapper->MinRange = _MinRange;
			wrapper->MaxRange = _MaxRange;
			wrapper->Scheme = &(((NL3D::CPSIntGradient *) (_SchemeWrapper->getScheme()) )->_F);
			CValueGradientDlg *gd = new CValueGradientDlg(wrapper, _Node, true, this, this);		
			wrapper->GradDlg = gd;
			wrapper->DefaultValue = 0;
			wrapper->Id = _CstValueId+ std::string("%%INT GRADIENT");
			gd->init(this);
			return gd;						
		}
		if (dynamic_cast<const NL3D::CPSIntMemory *>(scheme)) 
		{
			CAttribDlgInt *adi = new CAttribDlgInt(_CstValueId, _Node, _MinRange, _MaxRange);
			this->duplicateBoundChecker(*adi);
			CValueFromEmitterDlgT<sint32> *vfe = new CValueFromEmitterDlgT<sint32>((NL3D::CPSIntMemory *) _SchemeWrapper->getScheme(),
																			adi, 
																			this,
																			m_AttrBitmap.GetBitmap() );			
			vfe->init(this);
			return vfe;
		}
	/*	if (dynamic_cast<const NL3D::CPSIntMemory *>(scheme)) 
		{
			CValueFromEmitterDlgT<float, CAttribDlgFloat> vfe( (NL3D::CPSFloatMemory *)(scheme), std::string("UINT SCHEME"), m_AttrBitmap.GetBitmap());			
			vfe.DoModal();
		}*/
		if (dynamic_cast<const NL3D::CPSIntBinOp *>(scheme)) 
		{
			CAttribDlgInt *ad[2] = { NULL, NULL};
			for (uint k = 0; k <2; ++k)
			{
				ad[k] = new CAttribDlgInt(_CstValueId, _Node, _MinRange, _MaxRange);
				this->duplicateBoundChecker(*ad[k]);
			}
			CBinOpDlgT<sint32> *bod = new CBinOpDlgT<sint32>( (NL3D::CPSIntBinOp *)(scheme),
															  (CAttribDlgT<sint32> **) ad,
															  this,
															  m_AttrBitmap.GetBitmap());	
			bod->init(this);
			return bod;
		}
		return NULL;
	}

	//*************************************************************************************************************
	sint CAttribDlgInt::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<sint32> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSIntBlender *>(scheme)) return 0;		
		if (dynamic_cast<const NL3D::CPSIntGradient *>(scheme)) return 1;		
		if (dynamic_cast<const NL3D::CPSIntMemory *>(scheme)) return 2;		
		if (dynamic_cast<const NL3D::CPSIntBinOp *>(scheme)) return 3;		
		return -1;
	}


	//*************************************************************************************************************
	void CAttribDlgInt::setCurrentScheme(uint index)
	{
		nlassert(index < 4);


		NL3D::CPSAttribMaker<sint32> *scheme = NULL;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSIntBlender;
			break;
			case 1 :
				scheme = new NL3D::CPSIntGradient;
			break;
			case 2 :
				scheme = new NL3D::CPSIntMemory;
				((NL3D::CPSAttribMakerMemory<sint32> *) scheme)->setScheme(new NL3D::CPSIntBlender(_MinRange, _MaxRange)); 
			break;
			case 3 :
				scheme = new NL3D::CPSIntBinOp;
				((NL3D::CPSIntBinOp *) scheme)->setArg(0, new NL3D::CPSIntBlender);
				((NL3D::CPSIntBinOp *) scheme)->setArg(1, new NL3D::CPSIntBlender);
			break;
			default:	
			break;
		}

		if (scheme)
		{
			_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
		}
	}




///////////////////////
// CRGBA attributes  //
///////////////////////
	
	class CRGBABlenderDlgClient : public CValueBlenderDlgClientT<NLMISC::CRGBA>
	{
		public:
			CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<NLMISC::CRGBA> *wrapper) 
			{ 
				CColorEdit *ce = new CColorEdit;
				ce->setWrapper(wrapper);
				return ce;
			}
	};


	//////////////////////////////////////////////////////////
	// COLOR GRADIENT EDITION INTERFACE						//
	//////////////////////////////////////////////////////////


	//*************************************************************************************************************
	class CColorGradientDlgWrapper : public CValueGradientDlgClientT<CRGBA>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
			CRGBA col = Scheme->getValue(index);

			RECT r;

			r.left = x + 10;
			r.top = y + 10;
			r.right = x + 53;
			r.bottom = y + 29;

			CBrush b;
			b.CreateSolidBrush(RGB(col.R, col.G, col.B));
			dc->FillRect(&r, &b);	
			b.DeleteObject();

			b.CreateSolidBrush(RGB(0, 0, 0));
			CGdiObject *old = dc->SelectObject(&b);	
			r.top = y + 10; r. bottom = y + 29;
			r.right = x + 53; r.left = x + 10;
			dc->FrameRect(&r, &b);
			dc->SelectObject(old);
			b.DeleteObject();
		}
		CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<NLMISC::CRGBA> *wrapper) 
		{ 
			CColorEdit *ce = new CColorEdit;
			ce->setWrapper(wrapper);
			return ce;
		}
	};



	////////////////////////////

	//*************************************************************************************************************
	CAttribDlgRGBA::CAttribDlgRGBA(const std::string &valueID, CParticleWorkspace::CNode *node)  : CAttribDlgT<CRGBA>(valueID, node)
	{
	}

	//*************************************************************************************************************
	uint CAttribDlgRGBA::getNumScheme(void) const
	{
		return _DisableMemoryScheme ? 3 : 5;
	}

	//*************************************************************************************************************
	std::string CAttribDlgRGBA::getSchemeName(uint index) const
	{
		const char *types[] = { "color sampled blender", "color gradient", "color exact blender", "values computed from emitter", "binary operator" };
		nlassert(index < 5);
		return std::string(types[index]);
	}


	//*************************************************************************************************************
	CWnd *CAttribDlgRGBA::editScheme(void)
	{	
		const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) 
		{			
			CRGBABlenderDlgClient *myInterface = new CRGBABlenderDlgClient;
			myInterface->Id = std::string("RGBA_BLENDER");
			myInterface->SchemeFunc = & ((NL3D::CPSValueBlenderSample<CRGBA, 64> *) scheme)->_F;			
			CValueBlenderDlg *vb = new CValueBlenderDlg(myInterface, true, this, this, _Node);
			vb->init(this);
			return vb;
		}
		if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) 
		{
			CColorGradientDlgWrapper *wrapper = new CColorGradientDlgWrapper;
			wrapper->Scheme = &(((NL3D::CPSColorGradient *) (_SchemeWrapper->getScheme()) )->_F);
			CValueGradientDlg *gd = new CValueGradientDlg(wrapper, _Node, true, this, this);		
			wrapper->GradDlg = gd;
			wrapper->DefaultValue = CRGBA::White;
			wrapper->Id = std::string("RGBA_GRADIENT");
			gd->init(this);
			return gd;			
		}
		if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme)) 
		{
			return NULL;
		}

		if (dynamic_cast<const NL3D::CPSColorMemory *>(scheme)) 
		{
			CAttribDlgRGBA *ad = new CAttribDlgRGBA(_CstValueId, _Node);			
			CValueFromEmitterDlgT<CRGBA> *vfe = new CValueFromEmitterDlgT<CRGBA>( (NL3D::CPSColorMemory *)(scheme),
																					ad,
																					this,
																					m_AttrBitmap.GetBitmap());			
			vfe->init(this);
			return vfe;
		}

		if (dynamic_cast<const NL3D::CPSColorBinOp *>(scheme)) 
		{
			CAttribDlgRGBA *ad[2] = { NULL, NULL};
			for (uint k = 0; k <2; ++k)
			{
				ad[k] = new CAttribDlgRGBA(_CstValueId, _Node);	
			}
			CBinOpDlgT<CRGBA> *bod = new CBinOpDlgT<CRGBA>( (NL3D::CPSColorBinOp *)(scheme),
															(CAttribDlgT<CRGBA> **) ad,
															this,
															m_AttrBitmap.GetBitmap());	
			bod->init(this);
			return bod;
		}		
		return NULL;
	}

	//*************************************************************************************************************
	void CAttribDlgRGBA::setCurrentScheme(uint index)
	{
		nlassert(index < 5);

		NL3D::CPSAttribMaker<CRGBA> *scheme = NULL;

		switch (index)
		{
			case 0 :
				scheme = new NL3D::CPSColorBlender;
			break;
			case 1 :
				scheme = new NL3D::CPSColorGradient(NL3D::CPSColorGradient::_DefaultGradient, 2, 16, 1.f);
			break;
			case 2 :
				scheme = new NL3D::CPSColorBlenderExact;
			break;
			case 3 :
				scheme = new NL3D::CPSColorMemory;
				((NL3D::CPSAttribMakerMemory<CRGBA> *) scheme)->setScheme(new NL3D::CPSColorBlender);
			break;
			case 4 :
				scheme = new NL3D::CPSColorBinOp;
				((NL3D::CPSColorBinOp *) scheme)->setArg(0, new NL3D::CPSColorBlender);
				((NL3D::CPSColorBinOp *) scheme)->setArg(1, new NL3D::CPSColorBlender);
			break;
			default:	
			break;
		}

		if (scheme)
		{
			_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
		}
	}

	//*************************************************************************************************************
	sint CAttribDlgRGBA::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<CRGBA> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSColorBlender *>(scheme)) return 0;		
		if (dynamic_cast<const NL3D::CPSColorGradient *>(scheme)) return 1;		
		if (dynamic_cast<const NL3D::CPSColorBlenderExact *>(scheme)) return 2;		
		if (dynamic_cast<const NL3D::CPSColorMemory *>(scheme)) return 3;		
		if (dynamic_cast<const NL3D::CPSColorBinOp *>(scheme)) return 4;		
		return -1;
	}

	//*************************************************************************************************************
	CEditAttribDlg *CAttribDlgRGBA::createConstantValueDlg()
	{
		CColorEdit *ce = new CColorEdit;
		ce->setWrapper(_Wrapper);
	
		return ce;
	}



/////////////////////////////
// plane basis attributes  //
/////////////////////////////

	//////////////////////////////////////////////////////////
	// PLANE BASIS GRADIENT EDITION INTERFACE				//
	//////////////////////////////////////////////////////////


	class CPlaneBasisGradientDlgWrapper : public CValueGradientDlgClientT<NL3D::CPlaneBasis>
	{
	public:	
		/// a function that can display a value in a gradient, with the given offset. Deriver must define this
		void displayValue(CDC *dc, uint index, sint x, sint y)
		{		
				
			NLMISC::CRGBA c1[] ={ NLMISC::CRGBA::Black, NLMISC::CRGBA::Black, NLMISC::CRGBA::Black };
			NLMISC::CRGBA c2[] ={ NLMISC::CRGBA::Green, NLMISC::CRGBA::Green, NLMISC::CRGBA::Red };

		
			// read plane basis
			NL3D::CPlaneBasis pb =  Scheme->getValue(index);

			CPoint center(x + 20, y + 25);
			
			NLMISC::CMatrix m;			
			m.setRot(pb.X, pb.Y, pb.X ^ pb.Y);
			DrawBasisInDC(center, 12, m, *dc, c2);
		
		}
		CEditAttribDlg *newDialog(const std::string &id, IPSWrapper<NL3D::CPlaneBasis> *wrapper) 
		{ 
			CBasisEdit *be = new CBasisEdit;
			be->setWrapper(wrapper);
			return be;
		}
	};
	
	//*************************************************************************************************************
	CAttribDlgPlaneBasis::CAttribDlgPlaneBasis(const std::string &valueID, CParticleWorkspace::CNode *node)  : CAttribDlgT<NL3D::CPlaneBasis>(valueID, node)
	{
	}

	uint CAttribDlgPlaneBasis::getNumScheme(void) const
	{
		return _DisableMemoryScheme ? 3 : 5;
	}

	std::string CAttribDlgPlaneBasis::getSchemeName(uint index) const
	{
		const char *types[] = { "basis gradient", "follow path", "spinner", "values computed from emitter", "binary operator" };
		nlassert(index < 5);
		return std::string(types[index]);		
	}

	//*************************************************************************************************************
	CWnd *CAttribDlgPlaneBasis::editScheme(void)
	{	
		NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme();	
		if (dynamic_cast<NL3D::CPSPlaneBasisGradient *>(scheme)) 
		{
			CPlaneBasisGradientDlgWrapper *wrapper = new CPlaneBasisGradientDlgWrapper;
			wrapper->Scheme = &(((NL3D::CPSPlaneBasisGradient *) (_SchemeWrapper->getScheme()) )->_F);
			CValueGradientDlg *gd = new CValueGradientDlg(wrapper, _Node, true, this, this);		
			wrapper->GradDlg = gd;
			wrapper->DefaultValue = NL3D::CPlaneBasis(NLMISC::CVector::K);
			wrapper->Id = std::string("PLANE_BASIS_GRADIENT");
			gd->init(this);
			return gd;			
		}

		if (dynamic_cast<NL3D::CPSPlaneBasisFollowSpeed *>(scheme)) 
		{
			CEditFollowPath *efp = new CEditFollowPath((NL3D::CPSPlaneBasisFollowSpeed *) scheme, _Node, this, this);
			efp->init(this);
			return efp;
		}

		if (dynamic_cast<NL3D::CPSPlaneBasisMemory *>(scheme)) 
		{
			CAttribDlgPlaneBasis *ad = new CAttribDlgPlaneBasis(_CstValueId, _Node);			
			CValueFromEmitterDlgT<NL3D::CPlaneBasis> *vfe = new CValueFromEmitterDlgT<NL3D::CPlaneBasis>
															( (NL3D::CPSPlaneBasisMemory *)(scheme),
															  ad,
															  this,
															  m_AttrBitmap.GetBitmap());			
			vfe->init(this);
			return vfe;
		}

		if (dynamic_cast<NL3D::CPSPlaneBasisBinOp *>(scheme)) 
		{
			CAttribDlgPlaneBasis *ad[2] = { NULL, NULL};
			for (uint k = 0; k <2; ++k)
			{
				ad[k] = new CAttribDlgPlaneBasis(_CstValueId, _Node);				
			}
			CBinOpDlgT<NL3D::CPlaneBasis> *bod = new CBinOpDlgT<NL3D::CPlaneBasis>( (NL3D::CPSPlaneBasisBinOp *)(scheme),
																					(CAttribDlgT<NL3D::CPlaneBasis> **) ad,
																					this,
																					m_AttrBitmap.GetBitmap());	
			bod->init(this);
			return bod;
		}
		
		if (dynamic_cast<NL3D::CPSBasisSpinner *>(scheme)) 
		{
			CEditSpinner *es = new CEditSpinner(static_cast<NL3D::CPSBasisSpinner *>(scheme), _Node, this, this);
			es->init(this);
			return es;
		}
		return NULL;
	}

	//*************************************************************************************************************
	void CAttribDlgPlaneBasis::setCurrentScheme(uint index)
	{
		nlassert(index < 5);

		NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = NULL;

		switch (index)
		{	
			case 0:	
				scheme = new NL3D::CPSPlaneBasisGradient;
			break;
			case 1:	
				scheme = new NL3D::CPSPlaneBasisFollowSpeed;
			break;
			case 2:
				scheme = new NL3D::CPSBasisSpinner;
				static_cast<NL3D::CPSBasisSpinner *>(scheme)->_F.setNumSamples(16);
			break;
			case 3:	
				scheme = new NL3D::CPSPlaneBasisMemory;
				((NL3D::CPSAttribMakerMemory<NL3D::CPlaneBasis> *) scheme)->setScheme(new NL3D::CPSPlaneBasisFollowSpeed);
				if (_Node)
				{
					_Node->setModified(true);
				}
			break;
			case 4 :
				scheme = new NL3D::CPSPlaneBasisBinOp;
				((NL3D::CPSPlaneBasisBinOp *) scheme)->setArg(0, new NL3D::CPSPlaneBasisFollowSpeed);
				((NL3D::CPSPlaneBasisBinOp *) scheme)->setArg(1, new NL3D::CPSPlaneBasisFollowSpeed);
			break;			
			default:	
			break;
		}

		if (scheme)
		{
			_SchemeWrapper->setSchemeAndUpdateModifiedFlag(scheme);
		}
	}

	//*************************************************************************************************************
	sint CAttribDlgPlaneBasis::getCurrentScheme(void) const
	{
		const NL3D::CPSAttribMaker<NL3D::CPlaneBasis> *scheme = _SchemeWrapper->getScheme();	

		if (dynamic_cast<const NL3D::CPSPlaneBasisGradient *>(scheme)) return 0;		
		if (dynamic_cast<const NL3D::CPSPlaneBasisFollowSpeed *>(scheme)) return 1;		
		if (dynamic_cast<const NL3D::CPSBasisSpinner *>(scheme)) return 2;		
		if (dynamic_cast<const NL3D::CPSPlaneBasisMemory *>(scheme)) return 3;		
		if (dynamic_cast<const NL3D::CPSPlaneBasisBinOp *>(scheme)) return 4;		
		
		return -1;
	}

	//*************************************************************************************************************
	CEditAttribDlg *CAttribDlgPlaneBasis::createConstantValueDlg()
	{
		CBasisEdit *ce = new CBasisEdit;
		ce->setWrapper(_Wrapper);
		return ce;
	}

	//*************************************************************************************************************
	void CAttribDlg::OnDestroy() 
	{
		if (_SchemeEditionDlg)
		{
			CWnd *wnd = _SchemeEditionDlg;
			_SchemeEditionDlg = NULL;
			wnd ->DestroyWindow();
			delete wnd ;		
		}
		CDialog::OnDestroy();
			
	}

	//*************************************************************************************************************
	void CAttribDlg::OnClose() 
	{	
		CDialog::OnClose();
	}

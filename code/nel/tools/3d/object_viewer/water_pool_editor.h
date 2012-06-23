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



#if !defined(AFX_WATER_POOL_EDITOR_H__E6DAABC1_AD21_42EB_9DA4_A0AFBAE7C47C__INCLUDED_)
#define AFX_WATER_POOL_EDITOR_H__E6DAABC1_AD21_42EB_9DA4_A0AFBAE7C47C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif


namespace NL3D
{
	class CWaterPoolManager;
}

#include "nel/3d/water_height_map.h"
#include "ps_wrapper.h"
#include "editable_range.h"


class CWaterPoolEditor : public CDialog
{
// Construction
public:
	/// ctor
	CWaterPoolEditor(NL3D::CWaterPoolManager *wpm, CWnd* pParent = NULL);

	/// dtor
	~CWaterPoolEditor();

// Dialog Data
	//{{AFX_DATA(CWaterPoolEditor)
	enum { IDD = IDD_WATER_POOL };
	CListBox	m_PoolList;
	BOOL	m_AutomaticWavesGeneration;
	BOOL	m_BordersOnly;
	int		m_MapSize;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWaterPoolEditor)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CEditableRangeFloat *_DampingDlg;
	CEditableRangeFloat *_FilterWeightDlg;
	CEditableRangeFloat *_WaterUnitSizeDlg;
	CEditableRangeFloat *_ImpulsionStrenghtDlg;
	CEditableRangeFloat *_WavePeriodDlg;
	CEditableRangeUInt  *_WaveImpulsionRadiusDlg;
	CEditableRangeFloat *_PropagationTimeDlg;





	struct CDampingWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { nlassert(Whm); return Whm->getDamping(); }
		void  set(const float &v) { nlassert(Whm); Whm->setDamping(v); }
		CDampingWrapper() : Whm(NULL) {}
	} _DampingWrapper;

	struct CWaterUnitSizeWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { nlassert(Whm); return Whm->getUnitSize(); }
		void  set(const float &v) { nlassert(Whm); Whm->setUnitSize(v); }
		CWaterUnitSizeWrapper() : Whm(NULL) {}
	} _WaterUnitSizeWrapper;

	struct CFilterWeightWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { nlassert(Whm); return Whm->getFilterWeight(); }
		void  set(const float &v) { nlassert(Whm); Whm->setFilterWeight(v); }
		CFilterWeightWrapper() : Whm(NULL) {}
	} _FilterWeightWrapper;

	struct CImpulsionStrenghtWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { nlassert(Whm); return Whm->getWaveIntensity(); }
		void  set(const float &v) 
		{ 
			nlassert(Whm); Whm->setWaves(v, Whm->getWavePeriod(), Whm->getWaveImpulsionRadius(), Whm->getBorderWaves()); 
		}
		CImpulsionStrenghtWrapper() : Whm(NULL) {}
	} _ImpulsionStrenghtWrapper;

	struct CWavePeriodWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { nlassert(Whm); return Whm->getWavePeriod(); }
		void  set(const float &v) 
		{ 
			nlassert(Whm); 
			Whm->setWaves(Whm->getWaveIntensity(), v, Whm->getWaveImpulsionRadius(), Whm->getBorderWaves()); 
		}
		CWavePeriodWrapper() : Whm(NULL) {}
	}   _WavePeriodWrapper;

	struct CWaveImpulsionRadiusWrapper : IPSWrapperUInt
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		uint32 get(void) const	  { nlassert(Whm); return Whm->getWaveImpulsionRadius(); }
		void  set(const uint32 &v) 
		{ 
			nlassert(Whm); 
			Whm->setWaves(Whm->getWaveIntensity(), Whm->getWavePeriod(), v, Whm->getBorderWaves()); 
		}
		CWaveImpulsionRadiusWrapper() : Whm(NULL) {}
	}   _WaveImpulsionRadiusWrapper;

	struct CPropagationTimeWrapper : IPSWrapperFloat
	{
		NL3D::CWaterHeightMap *Whm; // the water height map being edited	
		float get(void) const	  { return Whm->getPropagationTime(); }
		void  set(const float &v) { Whm->setPropagationTime(v); }				
		CPropagationTimeWrapper() : Whm(NULL) {}
	}   _PropagationTimeWrapper;


	/// clear 'n' fill the pool  list
	void fillPoolList();

	/// update the wrapper to point the current selected pool
	void updateWrappers();

	void updateWaveParams();

	/// update checkbox and list bow from the current wave
	void updateWaveControls();

	/// get the
	uint32 CWaterPoolEditor::getCurrentPoolID();

	/// get the current pool being edited
	NL3D::CWaterHeightMap &getCurrentPool();

	/** Add a pool of the given ID (but doesn't update the dialog)
	  * \return position of insertion in the list
	  */
	int addPool(uint32 ID);

	void updateMapSizeCtrl();

	
	

	NL3D::CWaterPoolManager *_Wpm; // the water pool manager


	// Generated message map functions
	//{{AFX_MSG(CWaterPoolEditor)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangePoolList();
	afx_msg void OnAutomaticWavesGeneration();
	afx_msg void OnBordersOnly();
	afx_msg void OnAddPool();
	afx_msg void OnDeletePool();
	afx_msg void OnSelchangeMapSize();
	afx_msg void OnLoadPool();
	afx_msg void OnSavePool();
	afx_msg void OnRenamePool();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_WATER_POOL_EDITOR_H__E6DAABC1_AD21_42EB_9DA4_A0AFBAE7C47C__INCLUDED_)

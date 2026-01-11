// SoundPage.cpp : implementation file
//

#include "stdafx.h"
#include "source_sounds_builder.h"
#include "source_sounds_builderDlg.h"
#include "SoundPage.h"

#include "../src/sound/driver/buffer.h"
#include "../src/sound/audio_mixer_user.h"

#include "nel/sound/u_listener.h"

#include <nel/misc/common.h>
#include <nel/misc/vector.h>
using namespace NLMISC;

#include <math.h>

using namespace std;


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define CONE_X 340
#define CONE_Y 280
#define CONE_R 30


uint XCenter;
uint YCenter;
uint Radius;


const char *PriorityStr [NbSoundPriorities] = { "Highest", "High", "Medium", "Low" };


//----------------------------------------------------------
// Name: ConvertLinearSliderPosToLogScale()
// Desc: Converts a linear slider position to a quasi logrithmic scale
//-----------------------------------------------------------------------------
float ConvertLinearSliderPosToLogScale( LONG lSliderPos )
{
    if( lSliderPos > 0 && lSliderPos <= 10 )
    {
        return lSliderPos*0.01f;
    }
    else if( lSliderPos > 10 && lSliderPos <= 20 )
    {
        return (lSliderPos-10)*0.1f;
    }
    else if( lSliderPos > 20 && lSliderPos <= 30 )
    {
        return (lSliderPos-20)*1.0f;
    }
    else if( lSliderPos > 30 && lSliderPos <= 40 )
    {
        return (lSliderPos-30)*10.0f;
    }

    return 0.0f;
}




//-----------------------------------------------------------------------------
// Name: ConvertLinearSliderPosToLogScale()
// Desc: Converts a quasi logrithmic scale to a slider position
//-----------------------------------------------------------------------------
LONG ConvertLogScaleToLinearSliderPosTo( FLOAT fValue )
{
    if( fValue > 0.0f && fValue <= 0.1f )
    {
        return (LONG)(fValue/0.01f);
    }
    else if( fValue > 0.1f && fValue <= 1.0f )
    {
        return (LONG)(fValue/0.1f) + 10;
    }
    else if( fValue > 1.0f && fValue <= 10.0f )
    {
        return (LONG)(fValue/1.0f) + 20;
    }
    else if( fValue > 10.0f && fValue <= 100.0f )
    {
        return (LONG)(fValue/10.0f) + 30;
    }

    return 0;
}



/////////////////////////////////////////////////////////////////////////////
// CSoundPage dialog


CSoundPage::CSoundPage(CWnd* pParent /*=NULL*/)
	: CDialog(CSoundPage::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSoundPage)
	m_Filename = _T("");
	m_Gain = 1.0f;
	m_Pos3D = FALSE;
	m_MinDist = 1.0f;
	m_MaxDist = 1000000.0f;
	m_InnerAngleDeg = 360;
	m_OuterAngleDeg = 360;
	m_OuterGain = 1.0f;
	m_Looped = FALSE;
	m_Stereo = _T("");
	m_Pitch = 1.0f;
	m_Looping = false;
	m_SoundName = _T("");
	//}}AFX_DATA_INIT

	_CurrentSound = NULL;
	_Tree = NULL;
	_Source = NULL;
}


/*
 *
 */
BOOL CSoundPage::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// Load driver
	CWaitCursor waitcursor;
	_AudioMixer = UAudioMixer::createAudioMixer();
	try
	{
		_AudioMixer->init();
		//_AudioMixer->getListener()->setPos( CVector(0.0f,0.0001f,0.0f) );
	}
	catch( Exception& e )
	{
		CString s;
		s.Format( "No sound driver: %s\n\nSound playback will be unavailable" , e.what() );
		AfxMessageBox( s );
		_AudioMixer = NULL;
	}

	// Fill priorities
	uint i;
	for ( i=0; i!=NbSoundPriorities; ++i )
	{
		m_Priority.AddString( PriorityStr[i] );
	}

	// We want to load nss files even if the corresponding waves are missing
	CSound::allowMissingWave( true );

	// Cone drawing: make it work with normal and big fonts (depending on system settings)
	CRect parentrect, sliderrect;
	this->GetWindowRect( &parentrect );
	GetDlgItem( IDC_SliderOuterAngle )->GetWindowRect( &sliderrect );
	XCenter = sliderrect.right - parentrect.left + 15 + CONE_R;
	YCenter = sliderrect.top - parentrect.top + 10;
	Radius = CONE_R;

	waitcursor.Restore();

	_NameFont = new CFont();
	LOGFONT logfont;
	GetFont()->GetLogFont( &logfont );
	logfont.lfWeight = FW_BOLD;
	_NameFont->CreateFontIndirect( &logfont );
	
	GetDlgItem( IDC_SoundName )->SetFont( _NameFont );

	((CSliderCtrl*)GetDlgItem( IDC_SliderGain ))->SetRange( 0, 40 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderPitch ))->SetRange( 0, 40 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderMinDist ))->SetRange( 0, 1000 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderMaxDist ))->SetRange( 0, 1000 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderInnerAngle ))->SetRange( 0, 360 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterAngle ))->SetRange( 0, 360 );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterGain ))->SetRange( 0, 40 );

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void CSoundPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSoundPage)
	DDX_Control(pDX, IDC_CbPriority, m_Priority);
	DDX_Text(pDX, IDC_EditFilename, m_Filename);
	DDX_Text(pDX, IDC_EditGain, m_Gain);
	DDV_MinMaxFloat(pDX, m_Gain, 0.f, 1.f);
	DDX_Check(pDX, IDC_Pos3D, m_Pos3D);
	DDX_Text(pDX, IDC_EditMinDist, m_MinDist);
	DDV_MinMaxFloat(pDX, m_MinDist, 0.f, 1.e+006f);
	DDX_Text(pDX, IDC_EditMaxDist, m_MaxDist);
	DDV_MinMaxFloat(pDX, m_MaxDist, 0.f, 1.e+006f);
	DDX_Text(pDX, IDC_EditInnerAngle, m_InnerAngleDeg);
	DDV_MinMaxUInt(pDX, m_InnerAngleDeg, 0, 360);
	DDX_Text(pDX, IDC_EditOuterAngle, m_OuterAngleDeg);
	DDV_MinMaxUInt(pDX, m_OuterAngleDeg, 0, 360);
	DDX_Text(pDX, IDC_EditOuterGain, m_OuterGain);
	DDV_MinMaxFloat(pDX, m_OuterGain, 0.f, 1.f);
	DDX_Check(pDX, IDC_Looped, m_Looped);
	DDX_Text(pDX, IDC_Stereo, m_Stereo);
	DDX_Text(pDX, IDC_EditPitch, m_Pitch);
	DDV_MinMaxFloat(pDX, m_Pitch, 1.e-011f, 1.f);
	DDX_Check(pDX, IDC_Looping, m_Looping);
	DDX_Text(pDX, IDC_SoundName, m_SoundName);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSoundPage, CDialog)
	//{{AFX_MSG_MAP(CSoundPage)
	ON_BN_CLICKED(IDC_Pos3D, OnPos3D)
	ON_BN_CLICKED(IDC_ChooseFile, OnChooseFile)
	ON_BN_CLICKED(IDC_Remove, OnRemove)
	ON_BN_CLICKED(IDC_PlaySound, OnPlaySound)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_Looped, OnLooped)
	ON_EN_CHANGE(IDC_EditInnerAngle, OnChangeEditInnerAngle)
	ON_EN_CHANGE(IDC_EditOuterAngle, OnChangeEditOuterAngle)
	ON_WM_PAINT()
	ON_EN_CHANGE(IDC_EditOuterGain, OnChangeEditOuterGain)
	ON_WM_HSCROLL()
	ON_EN_CHANGE(IDC_EditMinDist, OnChangeEditMinDist)
	ON_EN_CHANGE(IDC_EditMaxDist, OnChangeEditMaxDist)
	ON_BN_CLICKED(IDC_ButtonHelp, OnButtonHelp)
	ON_EN_CHANGE(IDC_EditGain, OnChangeEditGain)
	ON_BN_CLICKED(IDC_ButtonTestOuterGain, OnButtonTestOuterGain)
	ON_EN_CHANGE(IDC_EditPitch, OnChangeEditPitch)
	ON_BN_CLICKED(IDC_Cancel, OnCancel)
	ON_BN_CLICKED(IDC_Home, OnHome)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/*
 *
 */
const char *StereoToCStr( CSound* snd, bool *st )
{
	static const char empty [] = "";
	static const char mono [] = "Mono";
	static const char stereo [] = "Stereo";
	if ( snd->getBuffer() == NULL )
	{
		*st = false;
		return empty;
	}
	else
	{
		if ( snd->getBuffer()->isStereo() )
		{
			*st = true;
			return stereo;
		}
		else
		{
			*st = false;
			return mono;
		}
	}
}


/*
 *
 */
void		CSoundPage::UpdateStereo()
{
	bool stereo;
	m_Stereo = StereoToCStr( _CurrentSound, &stereo );
	if ( stereo )
	{
		m_Pos3D = false;
	}
	GetDlgItem( IDC_Pos3D )->EnableWindow( ! stereo );
}


/*
 *
 */
void		CSoundPage::getPropertiesFromSound()
{
	m_SoundName = _CurrentSound->getName().c_str();
	m_Filename = _CurrentSound->getFilename().c_str();
	m_Gain = _CurrentSound->getGain();
	m_Pitch = _CurrentSound->getPitch();
	m_Priority.SetCurSel( _CurrentSound->getPriority() );
	m_Looping = _CurrentSound->getLooping();
	m_Pos3D = _CurrentSound->isDetailed();
	((CSliderCtrl*)GetDlgItem( IDC_SliderGain ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_Gain*100.0f ) );
	((CSliderCtrl*)GetDlgItem( IDC_SliderPitch ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_Pitch*100.0f ) );
	UpdateStereo();

	if ( m_Pos3D )
	{
		m_MinDist = _CurrentSound->getMinDistance();
		m_MaxDist = _CurrentSound->getMaxDistance();
		m_InnerAngleDeg = (uint)radToDeg( _CurrentSound->getConeInnerAngle() );
		m_OuterAngleDeg = (uint)radToDeg( _CurrentSound->getConeOuterAngle() );
		m_OuterGain = _CurrentSound->getConeOuterGain();
	}
	((CSliderCtrl*)GetDlgItem( IDC_SliderMinDist ))->SetPos( (int)m_MinDist );
	((CSliderCtrl*)GetDlgItem( IDC_SliderMaxDist ))->SetPos( (int)m_MaxDist );
	((CSliderCtrl*)GetDlgItem( IDC_SliderInnerAngle ))->SetPos( m_InnerAngleDeg );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterAngle ))->SetPos( m_OuterAngleDeg );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterGain ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_OuterGain*100.0f ) );
	UpdateData( false );

	Invalidate();
	OnPos3D(); // enable/disable 3d properties
}


/*
 *
 */
void CSoundPage::UpdateCurrentSound()
{
	CString name = ((CSource_sounds_builderDlg*)GetOwner())->SoundName( _HItem );
	if ( ! m_Pos3D )
	{
		_CurrentSound->setProperties( string(name), string(m_Filename), m_Gain, m_Pitch, (TSoundPriority)(m_Priority.GetCurSel()), m_Looping!=0, m_Pos3D!=0 );
	}
	else
	{
		_CurrentSound->setProperties( string(name), string(m_Filename), m_Gain, m_Pitch, (TSoundPriority)(m_Priority.GetCurSel()), m_Looping!=0, m_Pos3D!=0,
			m_MinDist, m_MaxDist, degToRad((float)m_InnerAngleDeg), degToRad((float)m_OuterAngleDeg), m_OuterGain );
	}
	// Argument checking is already done by the dialog wizard
}




/////////////////////////////////////////////////////////////////////////////
// CSoundPage message handlers


/*
 *
 */
void CSoundPage::setCurrentSound( CSound *sound, HTREEITEM hitem )
{
	_CurrentSound = sound;
	_HItem = hitem;
	if ( _Source != NULL )
	{
	 	_Source->stop();
	}
	m_Looped = false;
	UpdateData( false );
}


/*
 *
 */
void CSoundPage::OnPos3D() 
{
	UpdateData( true );
	GetDlgItem( IDC_Pos3DGroup )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_MinDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_MaxDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_ConeInnerAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_ConeOuterAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_ConeOuterGain )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditMinDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditMaxDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditInnerAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditOuterAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_EditOuterGain )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_SliderMinDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_SliderMaxDist )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_SliderInnerAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_SliderOuterAngle )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_SliderOuterGain )->EnableWindow( m_Pos3D );
	GetDlgItem( IDC_ButtonTestOuterGain )->EnableWindow( m_Pos3D );

	DrawCones();
}


/*
 *
 */
void CSoundPage::apply()
{
	if ( _CurrentSound )
	{
		if ( _Source != NULL )
		{
			_Source->stop();
		}

		UpdateData( true );

		UpdateCurrentSound();

		//(static_cast<CSource_sounds_builderDlg*>(GetOwner()))->setModified();

		nlassert( _Tree );

		if ( !m_Filename.empty() )
		{
			CString s = ((CSource_sounds_builderDlg*)GetOwner())->SoundName( _HItem ) + " (" + m_Filename + ")";
			_Tree->SetItemText( _HItem, s );
		}

		//_Tree->SelectItem( NULL );

		GetOwner()->SetFocus();
	}
}


/*
 *
 */
void CSoundPage::rename( CString s )
{
	// Quick way to do it (should be simplified)
	
	getPropertiesFromSound();
	nlassert( _Tree );
	_Tree->SetItemText( _HItem, s );
	UpdateCurrentSound();

	m_SoundName = s;
	UpdateData( false );
}


/*
 * Reverts the changes
 */
void CSoundPage::OnCancel()
{
	getPropertiesFromSound();
}


/*
 * Unselects the current sound
 */
void CSoundPage::cancel()
{
	if ( _Source != NULL )
	{
		_Source->stop();
	}

	_CurrentSound = NULL;
}


/*
 *
 */
void CSoundPage::OnHome() 
{
	nlassert( _Tree );
	_Tree->SelectItem( _Tree->GetRootItem() );
	GetOwner()->SetFocus();
}


/*
 *
 */
void CSoundPage::OnChooseFile() 
{
	// Prompt filename
	CFileDialog opendlg( true, "wav", "", OFN_OVERWRITEPROMPT, "PCM Wave files (*.wav)|*.wav||", this );
	if ( opendlg.DoModal()==IDOK )
	{
		UpdateData( true );
		m_Filename = opendlg.GetFileName();
		try 
		{
			loadSound();
		}
		catch ( Exception& e )
		{
			CString s;
			s.Format( "%s", e.what() );
			AfxMessageBox( s );
		}
		UpdateData( false );
	}
}


/*
 *
 */
bool CSoundPage::loadSound()
{
	UpdateCurrentSound();
	nlassert( _CurrentSound );
	if ( (m_Filename != "") && (_AudioMixer != NULL) )
	{
		_CurrentSound->loadBuffer( string(m_Filename) );
		UpdateStereo();
		return true;
	}
	else
	{
		return false;
	}
}


/*
 *
 */
void CSoundPage::OnRemove() 
{
	removeSound();
}


/*
 *
 */
void CSoundPage::removeSound()
{
	/*if ( AfxMessageBox( "Are you sure to remove the current sound from the list ?", MB_YESNO | MB_ICONQUESTION ) == IDYES )
	{*/
	if ( _CurrentSound )
	{
		nlassert( _Tree );
		_Tree->DeleteItem( _HItem );
	}
	/*}*/
}


/*
 *
 */
void CSoundPage::Play( bool outsidecone ) 
{
	CWaitCursor waitcursor;

	// Load sound
	try 
	{
		if ( loadSound() )
		{
			UpdateData( false );
			
			// Play source
			if ( _Source == NULL )
			{
				_Source = _AudioMixer->createSource( _CurrentSound );
			}
			else
			{
				_Source->stop();
				_Source->setSound( _CurrentSound );
			}
			_Source->setLooping( m_Looped!=0 );
			if ( outsidecone )
			{
				// The listener is just behind the source
				_Source->setPos( CVector(0.0f,0.1f,0.0f) );
				_Source->setDirection( CVector(0.0f,1.0f,0.0f) ); // directional
			}
			else
			{
				// The listener is at the source pos
				_Source->setPos( CVector(0.0f,0.0f,0.0f) );
				_Source->setDirection( CVector(0.0f,0.0f,0.0f) ); // non-directional
			}
			_Source->play();
		}

		waitcursor.Restore();
	}
	catch ( Exception& e )
	{
		waitcursor.Restore();

		CString s;
		s.Format( "%s", e.what() );
		AfxMessageBox( s );
	}
}


/*
 *
 */
void CSoundPage::OnPlaySound()
{
	UpdateData( true );
	m_Looped = false;
	Play( false );
}


/*
 *
 */
void CSoundPage::OnLooped() 
{
	UpdateData( true );
	if ( m_Looped )
	{
		Play( false );
	}
	else
	{
		if ( _Source != NULL )
		{
			_Source->stop();
		}
	}
}


/*
 *
 */
void CSoundPage::OnButtonTestOuterGain() 
{
	UpdateData( true );
	m_Looped = false;
	Play( true );
}


/*
 *
 */
void CSoundPage::OnClose() 
{
	if ( _AudioMixer != NULL )
	{
		if ( _Source != NULL )
		{
			delete _Source;
		}
		delete _AudioMixer;
	}
}


/*
 *
 */
void CSoundPage::DrawCones()
{
	if ( m_InnerAngleDeg > m_OuterAngleDeg )
	{
		GetDlgItem( IDC_BadCone )->SetWindowText( "Inner > Outer !" );
	}
	else
	{
		GetDlgItem( IDC_BadCone )->SetWindowText( "" );
	}

	CRect rect( XCenter-Radius, YCenter-Radius, XCenter+Radius+1, YCenter+Radius+1 );
	InvalidateRect( &rect, true );
}


/*
 *
 */
void CSoundPage::OnChangeEditGain() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderGain ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_Gain*100.0f ) );
}


/*
 *
 */
void CSoundPage::OnChangeEditPitch() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderPitch ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_Pitch*100.0f ) );
}


/*
 *
 */
void CSoundPage::OnChangeEditMinDist() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderMinDist ))->SetPos( (int)m_MinDist );
}


/*
 *
 */
void CSoundPage::OnChangeEditMaxDist() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderMaxDist ))->SetPos( (int)m_MaxDist );
}


/*
 *
 */
void CSoundPage::OnChangeEditInnerAngle() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderInnerAngle ))->SetPos( m_InnerAngleDeg );
	DrawCones();
}


/*
 *
 */
void CSoundPage::OnChangeEditOuterAngle() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterAngle ))->SetPos( m_OuterAngleDeg );
	DrawCones();
}


/*
 *
 */
void CSoundPage::OnChangeEditOuterGain() 
{
	UpdateData( true );
	((CSliderCtrl*)GetDlgItem( IDC_SliderOuterGain ))->SetPos( ConvertLogScaleToLinearSliderPosTo( m_OuterGain*100.0f) );
	DrawCones();
}


/*
 *
 */
void CSoundPage::OnHScroll( UINT nSBCode, UINT nPos, CScrollBar* pScrollBar )
{
	CSliderCtrl *slider = (CSliderCtrl*)pScrollBar; // MFC sucks

	nPos = slider->GetPos();
	{
		if ( slider == GetDlgItem( IDC_SliderGain ) )
		{
			m_Gain = ConvertLinearSliderPosToLogScale( nPos ) / 100.0f;
			if ( _Source != NULL )
			{
				_Source->setGain( m_Gain );
			}
		}
		else if ( slider == GetDlgItem( IDC_SliderPitch ) )
		{
			m_Pitch = ConvertLinearSliderPosToLogScale( nPos ) / 100.0f;
			if ( _Source != NULL )
			{
				_Source->setPitch( m_Pitch );
			}
		}
		else if ( slider == GetDlgItem( IDC_SliderMinDist ) )
		{
			m_MinDist = (float)nPos;
		}
		else if ( slider == GetDlgItem( IDC_SliderMaxDist ) )
		{
			m_MaxDist = (float)nPos;
		}
		else if ( slider == GetDlgItem( IDC_SliderInnerAngle ) )
		{
			m_InnerAngleDeg = nPos;
		}
		else if ( slider == GetDlgItem( IDC_SliderOuterAngle ) )
		{
			m_OuterAngleDeg = nPos;
		}
		else if ( slider == GetDlgItem( IDC_SliderOuterGain ) )
		{
			m_OuterGain = ConvertLinearSliderPosToLogScale( nPos ) / 100.0f;
		}
		UpdateData( false );
		DrawCones();
	}
}


/*
 *
 */
void CSoundPage::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	float innerangle = degToRad((float)m_InnerAngleDeg);
	float outerangle = degToRad((float)m_OuterAngleDeg);

	/*// Erase background (done by InvalidateRect())
	CRect rect( XCenter-Radius, YCenter-Radius, XCenter+Radius+1, YCenter+Radius+1 );
	CBrush brush;
	brush.CreateSolidBrush( GetSysColor( COLOR_BTNFACE ) );
	dc.FillRect( &rect, &brush );*/

	if ( m_Pos3D )
	{
		COLORREF innercolor = RGB(255,0,0);
		uint8 c = (uint)(255.0f * (1.0f - (float)(ConvertLogScaleToLinearSliderPosTo( m_OuterGain*100.0f ))/40.0f )); // linear, not logarithmic
		COLORREF outercolor = RGB(255,c,c);
		COLORREF tcolor = RGB(255,c/1.5,c/1.5); // not progressive
		if ( m_OuterGain == 0.0f ) // change white to background color
		{
			outercolor = GetSysColor( COLOR_BTNFACE );
		}

		uint dx = (uint)(Radius*sin(outerangle/2.0f));
		uint y = YCenter-(uint)(Radius*cos(outerangle/2.0f));

		// Outside
		CPen outpen( PS_SOLID, 1, outercolor );
		CBrush outbrush;
		outbrush.CreateSolidBrush( outercolor );
		dc.SelectObject( &outpen );
		dc.SelectObject( &outbrush );
		dc.Pie( XCenter-Radius, YCenter-Radius, XCenter+Radius+1, YCenter+Radius+1,	XCenter-dx, y, XCenter+dx, y );

		// Transition
		if ( (dx != 0) || (outerangle > 3.14) )
		{
			CPen tpen( PS_SOLID, 1, tcolor );
			CBrush tbrush;
			tbrush.CreateSolidBrush( tcolor );
			dc.SelectObject( &tpen );
			dc.SelectObject( &tbrush );
			dc.Pie( XCenter-Radius, YCenter-Radius, XCenter+Radius+1, YCenter+Radius+1,	XCenter+dx, y, XCenter-dx, y );
		}

		dx = (uint)(Radius*sin(innerangle/2.0f));
		y = YCenter-(uint)(Radius*cos(innerangle/2.0f));

		// Inner
		if ( (dx != 0) || (innerangle > 3.14) )
		{
			CPen inpen( PS_SOLID, 1, innercolor );
			CBrush inbrush;
			inbrush.CreateSolidBrush( innercolor );
			dc.SelectObject( &inpen );
			dc.SelectObject( &inbrush );
			dc.Pie( XCenter-Radius, YCenter-Radius, XCenter+Radius+1, YCenter+Radius+1,	XCenter+dx, y, XCenter-dx, y );
		}
	}
}


/*
 *
 */
BOOL CSoundPage::DestroyWindow() 
{
	delete _NameFont;

	return CDialog::DestroyWindow();
}


/*
 *
 */
void CSoundPage::OnButtonHelp() 
{
	MessageBox( "\
Gain: Range: [0, 1]. A gain factor is logarithmic ; 1.0 means no attenuation (full volume) ; 0.5 \
means an attenuation of 6 dB ; 0 means silence.\n\n\
Pitch: Range: ]0, 1]. 1.0 means normal ; dividing by 2 means pitching one octave down. Pitching up \
is not supported. 0 is not a legal value.\n\n\
Min Dist: Distance threshold below which gain is clamped (does not increase anymore).\n\
Unit: meters. Range: 0 - 1000000 (the maximum value 1e+006 is considered as infinite).\n\n\
Max Dist: Distance threshold above which gain is clamped (does not decrease anymore).\n\
Unit: meters. Range: 0 - 1000000 m (the maximum value 1e+006 is considered as infinite).\n\n\
Cone Inner Angle: Inside angle of the sound cone where the main gain is applied. \
The default of 360 means that the inner angle covers the entire world, which is equivalent to \
an omnidirectional source.\n\
Unit: degrees. Range: 0 - 360.\n\n\
Cone Outer Angle: Outer angle of the sound cone where the outer gain is applied \
to the main gain. The default of 360 means that the outer angle covers the entire world. If \
the inner angle is also 360, then there is no transition zone for angle-dependent (progressive) \
attenuation.\n\
Unit: degress. Range: 0 - 360.\n\n\
Cone Outer Gain: The factor with which the main gain is multiplied to determine the effective \
gain outside the cone defined by the outer angle. To test the outer gain, you can play the \
sound source as if you were outside the cone.\n\
Range: 0 - 1.\
", "Help about types" );	
}


#include "stdafx.h"

#include "nel/3d/tile_bank.h"
#include "nel/3d/texture_mem.h"
#include "nel/3d/texture_file.h"
#include "nel/misc/events.h"
#include "paint_undo.h"

using namespace NL3D;
using namespace NLMISC;

// Background
extern COLORREF backGround;

// User color 1 and 2
extern COLORREF color1;
extern COLORREF color2;
extern float opa1;
extern float opa2;
extern float hard1;
extern float hard2;

// A bank bitmap container
class CTileSetCont
{
public:
	// Create a container with a bank
	void build (CTileBank& bank, uint tileSet);

	// Some array for this tileset
	CSmartPtr<CTextureFile>				MainBitmap;
	CSmartPtr<CTextureFile>				GroupBitmap[NL3D_CTILE_NUM_GROUP];
	CSmartPtr<CTextureFile>				DisplaceBitmap[CTileSet::CountDisplace];

	// Groups precalc
	std::vector<uint>					GroupTile128[NL3D_CTILE_NUM_GROUP];
	std::vector<uint>					GroupTile256[NL3D_CTILE_NUM_GROUP];
};


// A tileset bitmap container
class CBankCont
{
public:
	// Create a container with a bank
	CBankCont (CTileBank& bank, HINSTANCE hInstance);

	// Array of tileset container
	std::vector <CTileSetCont>		TileSet;

	// Pointers on global bitmap
	CSmartPtr<CTextureMem>				_smallBitmap;
	CSmartPtr<CTextureMem>				mediumBitmap;
	CSmartPtr<CTextureMem>				largeBitmap;
	CSmartPtr<CTextureMem>				_256Bitmap;
	CSmartPtr<CTextureMem>				_128Bitmap;
	CSmartPtr<CTextureMem>				_0Bitmap;
	CSmartPtr<CTextureMem>				_1Bitmap;
	CSmartPtr<CTextureMem>				_2Bitmap;
	CSmartPtr<CTextureMem>				_3Bitmap;
	CSmartPtr<CTextureMem>				_4Bitmap;
	CSmartPtr<CTextureMem>				_5Bitmap;
	CSmartPtr<CTextureMem>				_6Bitmap;
	CSmartPtr<CTextureMem>				_7Bitmap;
	CSmartPtr<CTextureMem>				_8Bitmap;
	CSmartPtr<CTextureMem>				_9Bitmap;
	CSmartPtr<CTextureMem>				_10Bitmap;
	CSmartPtr<CTextureMem>				_11Bitmap;
	CSmartPtr<CTextureMem>				allBitmap;
	CSmartPtr<CTextureMem>				lightBitmap;
	CSmartPtr<CTextureMem>				lockBitmap;
	CSmartPtr<CTextureMem>				orientedBitmap;
	CSmartPtr<CTextureMem>				nothingBitmap;
	CSmartPtr<CTextureMem>				regularBitmap;
	CSmartPtr<CTextureMem>				goofyBitmap;

	// Handle on 
	HCURSOR								HCur;
	HCURSOR								HInspect;
	HCURSOR								HFill;
	HCURSOR								HTrick;

	// Undo manager
	CTileUndo							Undo;
};

// Open a pick color dialog and select a color
void chooseAColor ();

// Set background color
void setBackgroundColor ();

// Keys
enum PainterKeysType
{
	Select=0,
	Pick,
	Fill0,
	Fill1,
	Fill2,
	Fill3,
	MModeTile,
	MModeColor,
	MModeDisplace,
	ToggleColor,
	SizeUp,
	SizeDown,
	ToggleTileSize,
	GroupUp,
	GroupDown,
	BackgroundColor,
	ToggleArrows,
	HardnessUp,
	HardnessDown,
	OpacityUp,
	OpacityDown,
	Zouille,
	AutomaticLighting,
	SelectColorBrush,
	ToggleColorBrushMode,
	LockBorders,
	ZoomIn,
	ZoomOut,
	GetState,
	KeyCounter
};

// Def Keys
extern uint PainterKeys[KeyCounter];

// Light settings
extern CVector	LightDirection;
extern CRGBA	LightDiffuse;
extern CRGBA	LightAmbiant;
extern float	LightMultiply;
extern float	ZoomSpeed;

// Light settings


// Keys
extern const char* PainterKeysName[KeyCounter];

// Load ini file

void LoadKeyCfg ();
void LoadVarCfg ();

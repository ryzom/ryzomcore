#include "stdafx.h"
#include "nel_patch_paint.h"

#include "paint_ui.h"
#include "resource.h"
#include "nel/3d/texture_file.h"
#include "nel/3d/texture_mem.h"
#include "nel/misc/config_file.h"

#define REGKEY_EDIT_PATCH "Software\\Nevrax\\Ryzom\\edit_patch"

/*-------------------------------------------------------------------*/

// Def Keys
uint PainterKeys[KeyCounter]=
{
	KeySPACE,
	KeyP,
	KeyF5,
	KeyF6,
	KeyF7,
	KeyF8,
	KeyF1,
	KeyF2,
	KeyF3,
	KeyX,
	KeyINSERT,
	KeyDELETE,
	KeyW,
	KeyF,
	KeyV,
	KeyB,
	KeyC,
	KeyPRIOR,
	KeyNEXT,
	KeyHOME,
	KeyEND,
	KeyF11,
	KeyA,
	KeyS,
	KeyQ,
	KeyL,
	Key1,
	Key2,
	KeyI,
	KeyF10,
};

// Keys
const char* PainterKeysName[KeyCounter]=
{
	"Select",
	"Pick",
	"Fill0",
	"Fill1",
	"Fill2",
	"Fill3",
	"ModeTile",
	"ModeColor",
	"ModeDisplace",
	"ToggleColor",
	"SizeUp",
	"SizeDown",
	"ToggleTileSize",
	"GroupUp",
	"GroupDown",
	"BackgroundColor",
	"ToggleArrows",
	"HardnessUp",
	"HardnessDown",
	"OpacityUp",
	"OpacityDown",
	"Zouille",
	"AutomaticLighting",
	"SelectColorBrush",
	"ToggleColorBrushMode",
	"LockBorders",
	"ZoomIn",
	"ZoomOut",
	"GetState",
	"ResetPatch",
};

// Light settings
CVector		LightDirection (1, 1, -1);
CRGBA		LightDiffuse (255,255,255);
CRGBA		LightAmbiant (0,0,0);
float		LightMultiply = 1;
float		ZoomSpeed = 300;

// Load ini file

void LoadKeyCfg ();
void LoadVarCfg ();

/*-------------------------------------------------------------------*/

extern const unsigned char _small[];
extern const unsigned int _smallSize;
extern const unsigned char medium[];
extern const unsigned int mediumSize;
extern const unsigned char large[];
extern const unsigned int largeSize;
extern const unsigned char _256[];
extern const unsigned int _256Size;
extern const unsigned char _128[];
extern const unsigned int _128Size;
extern const unsigned char _0[];
extern const unsigned int _0Size;
extern const unsigned char _1[];
extern const unsigned int _1Size;
extern const unsigned char _2[];
extern const unsigned int _2Size;
extern const unsigned char _3[];
extern const unsigned int _3Size;
extern const unsigned char _4[];
extern const unsigned int _4Size;
extern const unsigned char _5[];
extern const unsigned int _5Size;
extern const unsigned char _6[];
extern const unsigned int _6Size;
extern const unsigned char _7[];
extern const unsigned int _7Size;
extern const unsigned char _8[];
extern const unsigned int _8Size;
extern const unsigned char _9[];
extern const unsigned int _9Size;
extern const unsigned char _10[];
extern const unsigned int _10Size;
extern const unsigned char _11[];
extern const unsigned int _11Size;
extern const unsigned char all[];
extern const unsigned int allSize;
extern const unsigned char light[];
extern const unsigned int lightSize;
extern const unsigned char lock[];
extern const unsigned int lockSize;
extern const unsigned char oriented[];
extern const unsigned int orientedSize;
extern const unsigned char nothing[];
extern const unsigned int nothingSize;
extern const unsigned char regular[];
extern const unsigned int regularSize;
extern const unsigned char goofy[];
extern const unsigned int goofySize;

/*-------------------------------------------------------------------*/

COLORREF backGround=0x808080;
COLORREF color1=0xffffff;
COLORREF color2=0x0;
float opa1=1.f;
float opa2=1.f;
float hard1=1.f;
float hard2=1.f;

/*-------------------------------------------------------------------*/

void CTileSetCont::build (CTileBank& bank, uint tileSet)
{
	// TileSet ref
	CTileSet* set=bank.getTileSet (tileSet);
	
	// Find a main bitmap with a valid name
	if (set->getNumTile128())
	{
		// Get the name
		std::string fileName=bank.getAbsPath()+bank.getTile (set->getTile128(0))->getRelativeFileName (CTile::diffuse);

		// Valid name?
		if (fileName!="")
		{
			// Create it
			MainBitmap=new CTextureFile (fileName);
		}
	}

	// Build group bitmaps
	for (int group=0; group<NL3D_CTILE_NUM_GROUP; group++)
	{
		int tile;

		// Look for a 128 tile in this group
		for (tile=0; tile<set->getNumTile128(); tile++)
		{
			// Tile pointer
			CTile* pTile=bank.getTile (set->getTile128 (tile));

			// Look for a tile of the group
			if (pTile->getGroupFlags ()&(1<<group))
			{
				// Get the name
				std::string fileName=bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse);

				// Valid name?
				if (fileName!="")
				{
					// Create it
					if (GroupBitmap[group]==NULL)
						GroupBitmap[group]=new CTextureFile (fileName);

					// Add to the group list
					GroupTile128[group].push_back (tile);
				}
			}
		}

		// Look for a 256 tile in this group
		for (tile=0; tile<set->getNumTile256(); tile++)
		{
			// Tile pointer
			CTile* pTile=bank.getTile (set->getTile256 (tile));

			// Look for a tile of the group
			if (pTile->getGroupFlags ()&(1<<group))
			{
				// Get the name
				std::string fileName=bank.getAbsPath()+pTile->getRelativeFileName (CTile::diffuse);

				// Valid name?
				if (fileName!="")
				{
					// Create it
					if (GroupBitmap[group]==NULL)
						GroupBitmap[group]=new CTextureFile (fileName);

					// Add to the group list
					GroupTile256[group].push_back (tile);
				}
			}
		}
	}

	// Current index
	bool dmwarn = false;
	for (uint displace=0; displace<CTileSet::CountDisplace; displace++)
	{
		uint dispTile = set->getDisplacementTile((CTileSet::TDisplacement)displace);

		if (bank.getDisplacementMapCount() <= dispTile)
		{
			if (!dmwarn)
			{
				dmwarn = true;
				MessageBox(NULL, "Tile bank not loaded, or bad tile bank. Missing a displacement tile. Use the tile bank utility to load the correct tilebank.", "NeL Patch Paint", MB_OK | MB_ICONWARNING);
			}
			continue; // with next displace
		}

		// Get the name
		std::string fileName = bank.getDisplacementMap(dispTile);
		if (fileName=="EmptyDisplacementMap")
			fileName="";

		// Valid name?
		if (fileName!="")
		{
			// Create it
			DisplaceBitmap[displace]=new CTextureFile (bank.getAbsPath()+fileName);
			DisplaceBitmap[displace]->loadGrayscaleAsAlpha (false);
		}
	}
}

/*-------------------------------------------------------------------*/

CBankCont::CBankCont (CTileBank& bank, HINSTANCE hInstance)
{
	// Allocate bitmaps
	_smallBitmap	=	new CTextureMem ((uint8*)_small, _smallSize, false);
	mediumBitmap	=	new CTextureMem ((uint8*)medium, mediumSize, false);
	largeBitmap		=	new CTextureMem ((uint8*)large, largeSize, false);
	_256Bitmap		=	new CTextureMem ((uint8*)_256, _256Size, false);
	_128Bitmap		=	new CTextureMem ((uint8*)_128, _128Size, false);
	_0Bitmap		=	new CTextureMem ((uint8*)_0, _0Size, false);
	_1Bitmap		=	new CTextureMem ((uint8*)_1, _1Size, false);
	_2Bitmap		=	new CTextureMem ((uint8*)_2, _2Size, false);
	_3Bitmap		=	new CTextureMem ((uint8*)_3, _3Size, false);
	_4Bitmap		=	new CTextureMem ((uint8*)_4, _4Size, false);
	_5Bitmap		=	new CTextureMem ((uint8*)_5, _5Size, false);
	_6Bitmap		=	new CTextureMem ((uint8*)_6, _6Size, false);
	_7Bitmap		=	new CTextureMem ((uint8*)_7, _7Size, false);
	_8Bitmap		=	new CTextureMem ((uint8*)_8, _8Size, false);
	_9Bitmap		=	new CTextureMem ((uint8*)_9, _9Size, false);
	_10Bitmap		=	new CTextureMem ((uint8*)_10, _10Size, false);
	_11Bitmap		=	new CTextureMem ((uint8*)_11, _11Size, false);
	allBitmap		=	new CTextureMem ((uint8*)all, allSize, false);
	lightBitmap		=	new CTextureMem ((uint8*)light, lightSize, false);
	lockBitmap		=	new CTextureMem ((uint8*)lock, lockSize, false);
	orientedBitmap	=	new CTextureMem ((uint8*)oriented, orientedSize, false);
	nothingBitmap	=	new CTextureMem ((uint8*)nothing, nothingSize, false);
	regularBitmap	=	new CTextureMem ((uint8*)regular, regularSize, false);
	goofyBitmap		=	new CTextureMem ((uint8*)goofy, goofySize, false);

	// Resize the tileset array
	TileSet.resize (bank.getTileSetCount());

	// For each tileSet, build the cont
	for (int tileSet=0; tileSet<bank.getTileSetCount(); tileSet++)
		TileSet[tileSet].build (bank, tileSet);

	// Load cursors
	HInspect = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_INSPECT));
	HCur = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_PICK_COLOR));
	HFill = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_FILL));
	HTrick = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_TRICK));
}

/*-------------------------------------------------------------------*/

void getColors (COLORREF *array)
{
	// Get the custom colors
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_EDIT_PATCH, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		for (uint i=0; i<16; i++)
		{
			DWORD len=4;
			DWORD type;
			char regName[100];
			smprintf (regName, 100, "Color%d", i);
			RegQueryValueEx (hKey, regName, 0, &type, (LPBYTE)(array+i), &len);
		}
		RegCloseKey (hKey);
	}
}

/*-------------------------------------------------------------------*/

void setColors (const COLORREF *array)
{
	// Set background color
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_EDIT_PATCH, &hKey)==ERROR_SUCCESS)
	{
		for (uint i=0; i<16; i++)
		{
			DWORD len=4;
			char regName[100];
			smprintf (regName, 100, "Color%d", i);
			RegSetValueEx (hKey, regName, 0, REG_DWORD, (LPBYTE)(array+i), 4);
		}
		RegCloseKey (hKey);
	}
}

/*-------------------------------------------------------------------*/

// Open a pick color dialog and select a color
void chooseAColor ()
{
	// Call the color picker dialog
	static COLORREF arrayColor[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	getColors (arrayColor);

	// Reset the struct
	CHOOSECOLOR cc;
	memset (&cc, 0, sizeof(CHOOSECOLOR));

	// Fill the struct
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.rgbResult=color1;
	cc.lpCustColors=arrayColor;
	cc.Flags=CC_RGBINIT|CC_ANYCOLOR|CC_FULLOPEN;

	// Open it
	if (ChooseColor (&cc))
	{
		// Set the color
		color1=cc.rgbResult;
		setColors (arrayColor);
	}
}

/*-------------------------------------------------------------------*/

// Set background color
void setBackgroundColor ()
{
	// Call the color picker dialog
	static COLORREF arrayColor[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	getColors (arrayColor);
	
	// Reset the struct
	CHOOSECOLOR cc;
	memset (&cc, 0, sizeof(CHOOSECOLOR));

	// Fill the struct
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.rgbResult=backGround;
	cc.lpCustColors=arrayColor;
	cc.Flags=CC_RGBINIT|CC_ANYCOLOR|CC_FULLOPEN;

	// Open it
	if (ChooseColor (&cc))
	{
		// Set the color
		backGround=cc.rgbResult;
		setColors (arrayColor);
	}
}

/*-------------------------------------------------------------------*/
extern HINSTANCE hInstance;
void LoadKeyCfg ()
{
	// Path of the dll
	HMODULE hModule = hInstance;
	if (hModule)
	{
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		if (res)
		{
			// split path
			char drive[256];
			char dir[256];
			_splitpath (sModulePath, drive, dir, NULL, NULL);

			// Make a new path
			char cgfPath[256];
			_makepath (cgfPath, drive, dir, "keys", ".cfg");

	
			CConfigFile cf;

			// Load and parse "test.txt" file
			cf.load (cgfPath);
			
			// For each keys
			for (uint key=0; key<KeyCounter; key++)
			{
				// go
				try
				{
					// Get the foo variable (suppose it's a string variable)
					CConfigFile::CVar &value= cf.getVar (PainterKeysName[key]);

					// Get value
					PainterKeys[key]=value.asInt ();
				}
				catch (EConfigFile &e)
				{
					// Something goes wrong... catch that
					const char* what=e.what();
				}
			}
		}
	}
}

/*-------------------------------------------------------------------*/
extern HINSTANCE hInstance;
void LoadVarCfg ()
{
	// Path of the dll
	HMODULE hModule = hInstance;
	if (hModule)
	{
		char sModulePath[256];
		int res=GetModuleFileName(hModule, sModulePath, 256);
		if (res)
		{
			// split path
			char drive[256];
			char dir[256];
			_splitpath (sModulePath, drive, dir, NULL, NULL);

			// Make a new path
			char cgfPath[256];
			_makepath (cgfPath, drive, dir, "keys", ".cfg");
	
			CConfigFile cf;

			// Load and parse "test.txt" file
			cf.load (cgfPath);
			
			// go
			try
			{
				// Get the light direction variable
				CConfigFile::CVar &light_direction= cf.getVar ("LightDirection");
				if (light_direction.size () == 3)
				{
					// Copy the light direction
					LightDirection.x = light_direction.asFloat (0);
					LightDirection.y = light_direction.asFloat (1);
					LightDirection.z = light_direction.asFloat (2);
				}
			}
			catch (EConfigFile &)
			{
			}

			try
			{
				// Get the light diffuse part
				CConfigFile::CVar &light_diffuse= cf.getVar ("LightDiffuse");
				if (light_diffuse.size () == 3)
				{
					LightDiffuse.R = light_diffuse.asInt (0);
					LightDiffuse.G = light_diffuse.asInt (1);
					LightDiffuse.B = light_diffuse.asInt (2);
				}
			}
			catch (EConfigFile &)
			{
			}

			try
			{
				// Get the light ambiant part
				CConfigFile::CVar &light_ambiant= cf.getVar ("LightAmbiant");
				if (light_ambiant.size () == 3)
				{
					LightAmbiant.R = light_ambiant.asInt (0);
					LightAmbiant.G = light_ambiant.asInt (1);
					LightAmbiant.B = light_ambiant.asInt (2);
				}
			}
			catch (EConfigFile &)
			{
			}

			try
			{
				// Get the light mulitply part
				CConfigFile::CVar &light_multiply= cf.getVar ("LightMultiply");
				LightMultiply = light_multiply.asFloat ();
			}
			catch (EConfigFile &)
			{
			}

			try
			{
				// Get the zoom speed
				CConfigFile::CVar &zoom_speed= cf.getVar ("ZoomSpeed");
				ZoomSpeed = zoom_speed.asFloat ();
			}
			catch (EConfigFile &)
			{
			}
		}
	}
}

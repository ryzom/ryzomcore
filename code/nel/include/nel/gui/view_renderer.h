// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef NL_VIEW_RENDERER_H
#define NL_VIEW_RENDERER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/rgba.h"
#include "nel/misc/uv.h"
#include "nel/misc/plane.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/u_material.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_driver.h"
#include "nel/3d/frustum.h"

namespace NLGUI
{

	// ***************************************************************************
	#define	VR_NUM_LAYER	32
	#define	VR_BIAS_LAYER	(VR_NUM_LAYER/2)
	#define	VR_LAYER_MAX	(VR_NUM_LAYER-VR_BIAS_LAYER-1)
	#define	VR_LAYER_MIN	(-VR_BIAS_LAYER)

	// ***************************************************************************
	/**
	 * class rendering the views
	 * All the quads of the interface are displayed in the following order
	 *
	 *  3--2
	 *  |  |
	 *  0--1
	 *
	 * \author Matthieu 'TrapII' Besson
	 * \author Nevrax France
	 * \date 2002
	 */
	class CViewRenderer
	{
	public:
		enum	TSystemTexture
		{
			QuantityCrossTexture= 0,
			DefaultBrickTexture,
			DefaultItemTexture,
			ItemPlanTexture,
			SkillTexture,
			ItemEnchantedTexture,
			DragCopyTexture,
			ItemWornedTexture,
			OutOfRangeTexture,
			RegenTexture,
			RegenBackTexture,
			GlowStarTexture,
			ItemLockedByOwnerTexture,
			NumSystemTextures,		
		};


	public:

		/** That class hold a texture id. It handle texture id destruction.
		*	Please use this class in your view and not sint32 to hold a texture id
		*/
		class CTextureId
		{
		public:

			// Default constructor
			CTextureId ()
			{
				_TextureId = -2;
			}

			// Destructor call deleteTextureId;
			~CTextureId ();

			// Set the texture id
			bool setTexture (const char *textureName, sint32 offsetX = 0, sint32 offsetY = 0, sint32 width = -1, sint32 height = -1,
							bool uploadDXTC=true, bool bReleasable=true);

			// Convert in texture id
			operator sint32 () const
			{
				return _TextureId;
			}

			void serial(NLMISC::IStream &f);

		private:
			sint32	_TextureId;
		};

	private:
		CViewRenderer();
		~CViewRenderer();

	public:
		static CViewRenderer* getInstance();

		/// setup the default values for everything
		void setup();

		/// init when TextContext and Driver are created
		void init();

		/// set the driver render states for the interface
		void setRenderStates ();

		/// Delete all textures and the like and reset the view renderer
		void reset();

		/// Release the resources of CViewRenderer, and delete the Singleton
		static void release();

		/// Retrieves the 3d driver we are using
		static NL3D::UDriver* getDriver();

		/// Sets the current TextContext.
		static void setTextContext( NL3D::UTextContext *textcontext );

		/// Sets the current driver
		static void setDriver( NL3D::UDriver *driver );

		/*
		 * setClipWindow : set the current clipping window
		 * (this window do not inherit properties from parent or whatever)
		 */
		void setClipWindow (sint32 x, sint32 y, sint32 w, sint32 h);

		/*
		 * getClipWindow : get the current clipping region
		 */
		void getClipWindow (sint32 &x, sint32 &y, sint32 &w, sint32 &h)
		{
			x = _ClipX;
			y = _ClipY;
			w = _ClipW;
			h = _ClipH;
		}

		/*
		 * true if the clipping region is empty: clipW or clipH is <=0
		 */
		bool isClipWindowEmpty() const {return _ClipW<=0 || _ClipH<=0;}

		/*
		 * checkNewScreenSize : check if the opengl screen size. This is SLOW !
		 * NB: if the window is minimized (w==h==0), then the old screen size is kept, and isMinimized() return true
		 */
		void checkNewScreenSize ();

		/*
		 * getScreenSize : get the screen window size changed (at last checkNewScreenSize called)
		 */
		void getScreenSize (uint32 &w, uint32 &h);

		/*
		 * get OOW / OOH
		 */
		void getScreenOOSize (float &oow, float &ooh);

		/*
		 * is the Screen minimized?
		 */
		bool isMinimized() const {return _IsMinimized;}

		/*
		 * drawBitmap : this is the interface with all the views
		 *
		 */
		void drawRotFlipBitmap (sint layerId, sint32 x, sint32 y, sint32 width, sint32 height, uint8 rot, bool flipv,
						sint32 nTxId, const NLMISC::CRGBA &col = NLMISC::CRGBA(255,255,255,255));

		/*
		 *	Draw a simple wired quad. No flushing is done as the draw is done instantly (usually for debug)
		 */
		void drawWiredQuad(sint32 x, sint32 y, sint32 width, sint32 height, NLMISC::CRGBA col = NLMISC::CRGBA::White);

		/*
		 *	Draw a simple filled quad. No flushing is done as the draw is done instantly (usually for debug)
		 */
		void drawFilledQuad(sint32 x, sint32 y, sint32 width, sint32 height, NLMISC::CRGBA col = NLMISC::CRGBA::White);

		/*
		 * drawBitmap : Tiled version
		 * \param tileOrigin 2 bits 1 - Left/Right (0/1) 2 - Bottom/Top (0/1) (0-BL)(1-BR)(2-TL)(3-TR)
		 *
		 */
		void drawRotFlipBitmapTiled (sint layerId, sint32 x, sint32 y, sint32 width, sint32 height, uint8 rot, bool flipv,
									 sint32 nTxId, uint tileOrigin, const NLMISC::CRGBA &col = NLMISC::CRGBA(255,255,255,255));



		/*
		 * drawBitmap : draw a bitmap roted by 90 degrees in CW 'rot times'
		 *				flipv is a boolean that indicates if there is a vertical flip
		 *				this is a 1:1 ratio so if texture is x long there are x pixels on screen
		 */
		void draw11RotFlipBitmap (sint layerId, sint32 x, sint32 y, uint8 rot, bool flipv, sint32 nTxId,
									const NLMISC::CRGBA &col = NLMISC::CRGBA(255,255,255,255));

		/** Draw an arbitrary quad (fast version) , possibly clipping it. Unlike draw11RotFlipBitmap & the like, texture is filtered here.
		  * quads are all batched in the same render layer
		  */
		void drawQuad(sint layerId, const NLMISC::CQuadUV &quadUV, sint32 nTxId,
					  NLMISC::CRGBA col, bool additif, bool filtered = true);
		/** Draw a set of untextured triangle in the given layer. all triangles of the same layer are batched
		  */
		void drawUnclippedTriangles(sint layerId, const std::vector<NLMISC::CTriangle> &tris, NLMISC::CRGBA col);

		/*
		 *	Draw a text
		 */
		void drawText (sint layerId, float x, float y, uint wordIndex, float xmin, float ymin, float xmax, float ymax, NL3D::UTextContext &textContext);

		/*
		 * loadTextures : load all textures associated with the interface
		 *				this function add a globaltexture to the vector of global textures
		 */
		bool loadTextures (const std::string &textureFileName, const std::string &uvFileName, bool uploadDXTC);

		/*
		 *	createTexture : create a texture for the interface, possibly from an externally created texture
		 *  If no external texture is given, then 'sGlobalTextureName' is the filename of the big texture
		 *  You should call deleteTexture when the texture is not used anymore
		 *	The method returns the texture id of the new texture
		 */
		sint32 createTexture (const std::string &sGlobalTextureName, // unique id identifying this big texture, (its filename if not externally created)
							  sint32 offsetX = 0,
							  sint32 offsetY = 0,
							  sint32 width = -1,
							  sint32 height = -1,
							  bool uploadDXTC=true,
							  bool bReleasable=true
							 );


		// change position of a sub-texture (inside its big texture) from the sub-texture filename
		void updateTexturePos(const std::string &texturefileName,
							  sint32 offsetX = 0,
							  sint32 offsetY = 0,
							  sint32 width = -1,
							  sint32 height = -1
							 );

		/** Add / change a global texture from an externally created texture
		  * \param defaultTexWidth width to used when CTextureId::createTexture is used without giving the width (e.g width = -1), useful for cropped textures
		  * \param defaultTexHeight height to used when CTextureId::createTexture is used without giving the height (e.g height = -1), useful for cropped textures
		  */

		void setExternalTexture(const std::string &sGlobalTextureName,
								NL3D::UTexture	*externalTexture = NULL,
								uint32			 externalTexWidth = 1,
								uint32			 externalTexHeight = 1,
								uint32			 defaultTexWidth = 1,
								uint32			 defaultTexHeight = 1
							   );

		/*
		 *	deleteTexture : create a texture for the interface
		 */
		void deleteTexture (sint32 textureId);

		// get a global texture pointer from its name
		NL3D::UTexture		*getGlobalTexture(const std::string &name);

		/*
		 * Flush all parsed view and computed strings to screen
		 */
		void flush ();

		/// Retrives a texture
		bool getTexture( NLMISC::CBitmap &bm, const std::string &name );

		/// Retrieve the texture names
		void getTextureNames( std::vector< std::string > &textures );

		/**
		 * get a texture file pointer from a string name. O(logN)
		 * \param id : the id of the texture
		 * \return a texture file pointer. -1 if not found or if sName is empty()
		 */
		sint32 getTextureIdFromName (const std::string &sName) const;
		std::string getTextureNameFromId (sint32 TxID);
		void getTextureSizeFromId (sint32 id, sint32 &width, sint32 &height);
		NLMISC::CRGBA getTextureColor(sint32 id, sint32 x, sint32 y);


		/**
		 * \return the texture associated with the param figur
		 */
		sint32 getFigurTextureId(uint index)
		{
			nlassert(index < 10);
			return _IndexesToTextureIds[index];
		}

		/**
		 * \return the texture for figur separator '-'
		 */
		sint32 getFigurSeparator() const { return _FigurSeparatorTextureId; }

		sint32 getFigurTextureW() const {return _WFigurTexture;}
		sint32 getFigurTextureH() const {return _HFigurTexture;}
		sint32 getFigurSeparatorW() const {return _WFigurSeparatorTexture;}
		sint32 getFigurSeparatorH() const {return _HFigurSeparatorTexture;}


		sint32 getFigurBlankTextureId ()
		{
			return _FigurBlankId;
		}
		sint32 getBlankTextureId ()
		{
			return _BlankId;
		}


		sint32 getTypoTextureW(char c);
		sint32 getTypoTextureH(char c);
		sint32 getTypoTextureId(char c);

		/// System Texture Manager. Used to avoid storing an id in each Ctrl for some texture code is aware of
		// @{
		sint32	getSystemTextureId(TSystemTexture e) const {return _SystemTextures[e].Id;}
		sint32	getSystemTextureW(TSystemTexture e) const {return _SystemTextures[e].W;}
		sint32	getSystemTextureH(TSystemTexture e) const {return _SystemTextures[e].H;}
		// @}

		/// For string rendering, get the RenderBuffer to the specified layer
		NL3D::URenderStringBuffer		*getStringRenderBuffer(sint layerId);


		/** Custom Rendering Interface
		 * Note that this function is EXTREMLY SLOW so it should be used with care
		 * This function flush the quad cache, clip the quad passed with current clipping region
		 * and draw (with drawQuedUV2) it with the material passed in parameter. There is no cache operation done.
		 * uv used are from (0,0) -> (1,1) for the 2 stages
		 */
		void drawCustom (sint32 x, sint32 y, sint32 width, sint32 height, NLMISC::CRGBA col, NL3D::UMaterial Mat);
		// Same but we can control uv mapping of first stage
		void drawCustom (sint32 x, sint32 y, sint32 width, sint32 height,
						const NLMISC::CUV &uv0Min, const NLMISC::CUV &uv0Max,
						NLMISC::CRGBA col, NL3D::UMaterial Mat);
		// Same but we can control uv mapping of 2 stages
		void drawCustom (sint32 x, sint32 y, sint32 width, sint32 height,
						const NLMISC::CUV &uv0Min, const NLMISC::CUV &uv0Max, const NLMISC::CUV &uv1Min, const NLMISC::CUV &uv1Max,
						NLMISC::CRGBA col, NL3D::UMaterial Mat);

		// **** World space interface methods

		/** Set the current Z in projCenter.z.
		 *	If you want to scale the window position, set a scale!=1. projCenter.x/y  is used as the
		 *	pivot (in window coordinates)
		 */
		void setInterfaceDepth (const NLMISC::CVector &projCenter, float scale);

		// Activate world space transformation
		void activateWorldSpaceMatrix (bool activate);

		// Set the screen to world space matrix
		void setWorldSpaceFrustum (const NL3D::CFrustum &cameraFrustum);

		// Get the current Frustum
		const NL3D::CFrustum &getFrustum () const
		{
			return _CameraFrustum;
		}

	private:
		/**
		 * init the map _IndexesToTextures
		 */
		void initIndexesToTextureIds ();
		void initTypo ();

		bool needClipping (const NLMISC::CQuad &q);

		void clip (NLMISC::CQuadColorUV &qout, const NLMISC::CQuadColorUV &qin, uint rot);
		void clip (NLMISC::CQuadColorUV2 &qout, const NLMISC::CQuadColorUV2 &qin);
	private:

		// A layer is a vector of Quads.
		class	CLayer
		{
		public:
			// unfiltered quads
			std::vector<NLMISC::CQuadColorUV>	Quads;
			uint32								NbQuads;
			std::vector<NLMISC::CTriangleColorUV>	Tris;
			// filtered alpha blended quads
			std::vector<NLMISC::CQuadColorUV>		FilteredAlphaBlendedQuads;
			// filtered alpha blended tris
			std::vector<NLMISC::CTriangleColorUV>	FilteredAlphaBlendedTris;
			// filtered additif blended quads
			std::vector<NLMISC::CQuadColorUV>		FilteredAdditifQuads;
			// filtered additif blended tris
			std::vector<NLMISC::CTriangleColorUV>	FilteredAdditifTris;

			CLayer()
			{
				NbQuads= 0;
			}
		};

		// SGlobalTexture is a texture that regroup other texture. We store also current quads to render
		struct SGlobalTexture
		{
			SGlobalTexture ()
			{
				FromGlobaleTexture = true;
			}
			uint32				Width, Height;
			uint32				DefaultWidth, DefaultHeight;
			NL3D::UTexture		*Texture;
			std::string			Name;
			bool				FromGlobaleTexture;
			// Array of layers
			CLayer				Layers[VR_NUM_LAYER];
		};

		// For each Layer, store a string Buffer
		NL3D::URenderStringBuffer		*_StringRBLayers[VR_NUM_LAYER];

		// For each Layer, tells if empty or not
		bool							_EmptyLayer[VR_NUM_LAYER];

		// SImage is one texture of the SGlobalTexture textures
		struct SImage
		{
			std::string		Name;
			SGlobalTexture	*GlobalTexturePtr;
			NLMISC::CUV		UVMin, UVMax;

			// Assign UV of this image to a quad depending on the flip and rot
			void setupQuadUV(bool flipv, uint8 rot, NLMISC::CQuadColorUV &dest);
		};

		// ***************************************************************************
		// \name Texture management
		// ***************************************************************************

		// SImage accessors
		SImage	*getSImage(sint32 textureId)
		{
			return &(*(_SImageIterators[textureId]));
		}


		// Add a SImage
		sint32 addSImage(const SImage &image)
		{
			uint i;
			for (i=0; i<_SImageIterators.size(); i++)
			{
				// Free ?
				if (_SImageIterators[i] == _SImages.end())
					break;
			}

			// Nothing free ?
			if (i == _SImageIterators.size())
				_SImageIterators.push_back(_SImages.end());

			_SImages.push_back(image);
			_SImageIterators[i] = _SImages.end();
			_SImageIterators[i]--;
			return (sint32)i;
		}

		// Remove a SImage
		void removeSImage(sint32 textureId)
		{
			// Allocated ?
			nlassert (_SImageIterators[textureId] != _SImages.end());

			// Remove the image
			_SImages.erase (_SImageIterators[textureId]);

			// Remove the index entry
			_SImageIterators[textureId] = _SImages.end();
		}

		typedef std::list<SGlobalTexture>	TGlobalTextureList;
		typedef std::list<SImage>			TSImageList;
		typedef std::vector<std::list<SImage>::iterator>	TSImageIterator;

		// List of global textures
		TGlobalTextureList			_GlobalTextures;

		// List of SImage
		TSImageList					_SImages;

		// Array used to convert a texture ID in _SImages iterator
		TSImageIterator				_SImageIterators;

		// ***************************************************************************

		typedef	std::map<std::string, uint>		TTextureMap;
		TTextureMap					_TextureMap;

		NL3D::UMaterial _Material;

		// Clip & screen system
		sint32 _ClipX, _ClipY, _ClipW, _ClipH;
		float _XMin, _XMax, _YMin, _YMax;

		sint32		_ScreenW, _ScreenH;
		float		_OneOverScreenW, _OneOverScreenH;
		bool		_IsMinimized;


		//map linking a uint to a bitmap. Used to display figurs
		std::vector<sint32> _IndexesToTextureIds;
		sint32				_FigurSeparatorTextureId;
		sint32 _FigurBlankId, _BlankId;
		sint32	_WFigurTexture;
		sint32	_HFigurTexture;
		sint32  _WFigurSeparatorTexture;
		sint32  _HFigurSeparatorTexture;
		NLMISC::CUV		_BlankUV;
		SGlobalTexture	*_BlankGlobalTexture;

		// System textures
		class	CSystemTexture
		{
		public:
			sint32	Id;
			sint32	W;
			sint32	H;

			CSystemTexture()
			{
				Id= -1;
				W= H= 0;
			}
		};
		CSystemTexture		_SystemTextures[NumSystemTextures];

		// Typo texture
		enum
		{
			NumTypoChar= 127,
		};
		sint32				_TypoCharToTextureIds[NumTypoChar];
		sint32				_TypoCharWs[NumTypoChar];
		sint32				_TypoH;


		void	addSystemTexture(TSystemTexture e, const char *s);
		void	initSystemTextures();

		/**
		 * put a new quad in the cache (call flush if texture different)
		 */
		void putQuadInLayer (SGlobalTexture &gt, sint layerId, const NLMISC::CQuadColorUV &qcoluv, uint rot);

		// World space interface methods
		void worldSpaceTransformation (NLMISC::CQuadColorUV &qcoluv);

		bool				_WorldSpaceTransformation;	// Transform into world space
		float				_CurrentZ;					// Current z used for the scene
		NL3D::CFrustum		_CameraFrustum;				// Transform from screen space to world space
		NLMISC::CMatrix		_WorldSpaceMatrix;			// Matrix to be applied for world space transformation
		bool				_WorldSpaceScale;
		
		
		static CViewRenderer *instance;
		static NL3D::UDriver *driver;
		static NL3D::UTextContext *textcontext;

	public:
		static NL3D::UTextContext* getTextContext(){ return textcontext; }

		/// Set of hw cursor images
		static std::set< std::string > *hwCursors;
		static float hwCursorScale;

	};


}

#endif // NL_VIEW_RENDERER_H

/* End of view_renderer.h */

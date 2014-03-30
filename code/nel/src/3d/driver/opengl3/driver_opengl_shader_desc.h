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

#ifndef SHADER_DESC
#define SHADER_DESC

#include "nel/misc/types_nl.h"

namespace NL3D
{
	class CVertexProgram;
	class CPixelProgram;

	struct SShaderPair
	{
		SShaderPair()
		{
			vp = NULL;
			pp = NULL;
		}

		~SShaderPair()
		{
			vp = NULL;
			pp = NULL;
		}

		bool empty() const{
			if ((vp == NULL) && (pp == NULL))
				return true;
			else
				return false;
		}

		CVertexProgram *vp;
		CPixelProgram  *pp;
	};

	class CShaderDesc
	{
	public:

		enum TShaderType
		{
			Normal,
			Bump_unused,
			UserColor,
			LightMap,
			Specular,
			Caustics_unused,
			PerPixelLighting,
			PerPixelNoSpecular,
			Cloud,
			Water
		};
		
		enum TFogMode
		{
			Exponential,
			Exponential2,
			Linear
		};

		enum TLightMode
		{
			Nolight,
			Directional,
			Point,
			Spot
		};

		CShaderDesc() {
			for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
				texEnvMode[ i ] = 0;

			for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; i++)
				lightMode[ i ] = Nolight;
			
			for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
				useTextureStage[ i ] = false;
			useFirstTextureCoordSet = false;
			noTextures = true;
			
			features = None;
			shaderType = Normal;
			vbFlags = 0;
			nlightmaps = 0;
			alphaTestTreshold = 0.5f;
			fogMode = Linear;
			pointLight = false;
		}

		~CShaderDesc() {
		}

		bool operator==(const CShaderDesc &o) const
		{
			if (noTextures != o.noTextures)
				return false;

			if (shaderType != o.shaderType)
				return false;

			if (vbFlags != o.vbFlags)
				return false;

			if (nlightmaps != o.nlightmaps)
				return false;

			if (features != o.features)
				return false;

			if ((features & AlphaTest) != 0)
			{
				if (alphaTestTreshold > o.alphaTestTreshold + 0.0001f)
					return false;

				if (alphaTestTreshold < o.alphaTestTreshold - 0.0001f)
					return false;
			}

			if (fogEnabled())
			{
				if (fogMode != o.fogMode)
					return false;
			}

			if (shaderType == Normal)
			{
				if (useFirstTextureCoordSet != o.useFirstTextureCoordSet)
					return false;

				for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
					if (texEnvMode[ i ] != o.texEnvMode[ i ])
						return false;

				for (int i = 0; i < IDRV_MAT_MAXTEXTURES; i++)
					if (useTextureStage[ i ] != o.useTextureStage[ i ])
						return false;
			}

			if (lightingEnabled())
			{
				for (int i = 0; i < NL_OPENGL3_MAX_LIGHT; i++)
					if (lightMode[ i ] != o.lightMode[ i ])
						return false;
			}
		
			return true;
		}

		void setTexEnvMode(uint32 index, uint32 mode) { texEnvMode[ index ] = mode; }
		
		void setUseFirstTexCoords(bool b) { useFirstTextureCoordSet = b; }
		bool getUseFirstTexCoords() const{ return useFirstTextureCoordSet; }

		void setUseTexStage(uint8 stage, bool b) { useTextureStage[ stage ] = b; }
		bool getUseTexStage(uint8 stage) const{ return useTextureStage[ stage ]; }

		void setNoTextures(bool b) { noTextures = b; }
		bool useTextures() const{ return !noTextures; }

		void setVBFlags(uint32 flags) { vbFlags = flags; }
		bool hasVBFlags(uint32 flags) const{
			if ((vbFlags & flags) != 0)
				return true;
			else
				return false;
		}

		void setShaderType(uint32 type) { shaderType = type; }
		void setNLightMaps(uint32 n) { nlightmaps = n; }
		
		void setAlphaTest(bool b)
		{
			if (b)
				features |= AlphaTest;
			else
				features &= ~AlphaTest;
		}

		void setAlphaTestThreshold(float t) { alphaTestTreshold = t; }

		void setFog(bool b)
		{
			if (b)
				features |= Fog;
			else
				features &= ~Fog;
		}

		bool fogEnabled() const
		{
			if ((features & Fog) != 0)
				return true;
			else
				return false;
		}

		void setFogMode(TFogMode mode) { fogMode = mode; }

		uint32 getFogMode() const{ return fogMode; }

		void setLighting(bool b)
		{
			if (b)
				features |= Lighting;
			else
				features &= ~Lighting;
		}

		bool lightingEnabled() const
		{
			if ((features & Lighting) != 0)
				return true;
			else
				return false;
		}

		void setLight(int idx, TLightMode mode)
		{
			lightMode[ idx ] = mode;
			if (mode == Point)
				pointLight = true;
		}

		TLightMode getLight(int idx) const{ return lightMode[ idx ]; }

		bool hasPointLight() const{ return pointLight; }

		void setShaders(SShaderPair sp) { shaderPair = sp; }
		SShaderPair getShaders() const{ return shaderPair; }

	private:

		enum TShaderFeatures
		{
			None      = 0,
			AlphaTest = 1,
			Fog       = 2,
			Lighting  = 4
		};

		uint32 features;
		uint32 texEnvMode[ IDRV_MAT_MAXTEXTURES ];
		bool useTextureStage[ IDRV_MAT_MAXTEXTURES ];
		bool useFirstTextureCoordSet;
		bool noTextures;
		uint32 vbFlags;
		uint32 shaderType;
		uint32 nlightmaps;
		float alphaTestTreshold;
		uint32 fogMode;
		TLightMode lightMode[ NL_OPENGL3_MAX_LIGHT ];
		bool pointLight;

		SShaderPair shaderPair;
	};
}

#endif


// Old Lightmap process


void SortFaceByTextureName(vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, j;
	int nNbFace = AllFaces.size();

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		pT = pMB->Materials[AllFaces[j]->MaterialId].getTexture(0);
		pTF = dynamic_cast<CTextureFile *>(pT);
		string namej = "Default";
		if( pTF != NULL )
			namej = pTF->getFileName();
		if( namei < namej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

// TextureNames is an array which indicates the number of faces that follows which have the same texture name
void ComputeAreaOfTextureName(vector<sint32> &TextureNames, vector<CMesh::CFace*> &AllFaces, CMesh::CMeshBuild *pMB)
{
	int i, nNbFace = AllFaces.size();
	TextureNames.resize(nNbFace);
	ITexture *pT = pMB->Materials[AllFaces[0]->MaterialId].getTexture(0);
	CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
	string CurrentName = "Default";
	sint32 lastface = 0, nNbTexName = 0;
	if( pTF != NULL )
		CurrentName = pTF->getFileName();
	for( i = 0; i < nNbFace; ++i )
	{
		ITexture *pT = pMB->Materials[AllFaces[i]->MaterialId].getTexture(0);
		CTextureFile *pTF = dynamic_cast<CTextureFile *>(pT);
		string namei = "Default";
		if( pTF != NULL )
			namei = pTF->getFileName();
		if( ( namei != CurrentName ) || ( i == (nNbFace-1) ) )
		{
			CurrentName = namei;
			TextureNames[nNbTexName] = i-lastface;
			nNbTexName++;
			lastface = i;
		}
	}
	TextureNames[nNbTexName-1] += 1;
	TextureNames.resize( nNbTexName );
}

void SortFaceByMaterialId( int offset, int nNbFace, vector<CMesh::CFace*> &AllFaces )
{
	int i, j;
	
	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	if( AllFaces[offset+i]->MaterialId < AllFaces[offset+j]->MaterialId )
	{
		CMesh::CFace *pFaceTemp = AllFaces[offset+i];
		AllFaces[offset+i] = AllFaces[offset+j];
		AllFaces[offset+j] = pFaceTemp;
	}
}

void SortFaceByTextureSurface( int offset, int nNbFace, vector<CMesh::CFace*> &AllFaces )
{
	int i, j;

	for( i = 0; i < nNbFace-1; ++i )
	for( j = i+1; j < nNbFace; ++j )
	if( AllFaces[i]->MaterialId == AllFaces[j]->MaterialId )
	{
		// Texture surface of the i face = .5*|(u1-u0)*(v2-v0)-(v1-v0)*(u2-u0)|
		// in fact this is lightmap mapping surface
		double surfacei = 0.5*fabs(
		(AllFaces[i]->Corner[1].Uvws[1].U - AllFaces[i]->Corner[0].Uvws[1].U)*
		(AllFaces[i]->Corner[2].Uvws[1].V - AllFaces[i]->Corner[0].Uvws[1].V)-
		(AllFaces[i]->Corner[1].Uvws[1].V - AllFaces[i]->Corner[0].Uvws[1].V)*
		(AllFaces[i]->Corner[2].Uvws[1].U - AllFaces[i]->Corner[0].Uvws[1].U) );
		double surfacej = 0.5*fabs(
		(AllFaces[j]->Corner[1].Uvws[1].U - AllFaces[j]->Corner[0].Uvws[1].U)*
		(AllFaces[j]->Corner[2].Uvws[1].V - AllFaces[j]->Corner[0].Uvws[1].V)-
		(AllFaces[j]->Corner[1].Uvws[1].V - AllFaces[j]->Corner[0].Uvws[1].V)*
		(AllFaces[j]->Corner[2].Uvws[1].U - AllFaces[j]->Corner[0].Uvws[1].U) );
		if( surfacei < surfacej )
		{
			CMesh::CFace *pFaceTemp = AllFaces[i];
			AllFaces[i] = AllFaces[j];
			AllFaces[j] = pFaceTemp;
		}
	}
}

bool isInTriangle(double x, double y, double xt1, double yt1, double xt2, double yt2, double xt3, double yt3)
{
	// Test vecteur T1X avec T1T2
	double sign1 = ((xt2-xt1)*(y-yt1) - (yt2-yt1)*(x-xt1));
	// Test vecteur T2X avec T2T3
	double sign2 = ((xt3-xt2)*(y-yt2) - (yt3-yt2)*(x-xt2));
	// Test vecteur T3X avec T3T1
	double sign3 = ((xt1-xt3)*(y-yt3) - (yt1-yt3)*(x-xt3));
	if( (sign1 <= 0.0)&&(sign2 <= 0.0)&&(sign3 <= 0.0) )
		return true;
	if( (sign1 >= 0.0)&&(sign2 >= 0.0)&&(sign3 >= 0.0) )
		return true;
	return false;
}

// Segment line intersection P1P2 and P3P4
bool segmentIntersection(double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4)
{
	double denominator = (y4-y3)*(x2-x1) - (x4-x3)*(y2-y1);
	double k = ((x4-x3)*(y1-y3) - (y4-y3)*(x1-x3) ) / denominator;
	if( (k<0.0) || (k>1.0) ) return false;
	k = ( (x2-x1)*(y1-y3) - (y2-y1)*(x1-x3) ) / denominator;
	if( (k<0.0) || (k>1.0) ) return false;
	return true;
}

void CreatePiece( vector<uint8>& Piece, sint32& nSizeX, sint32& nSizeY, sint32 &nPosX, sint32 &nPosY,
				 float lumx1, float lumy1,
				 float lumx2, float lumy2, 
				 float lumx3, float lumy3, uint8 nCol )
{
	double minx, miny;
	double maxx, maxy;
	int j,k;

	if( nCol == 0 )
		nCol = 1;
	minx = lumx1;
	if( minx > lumx2 ) minx = lumx2;
	if( minx > lumx3 ) minx = lumx3;
	maxx = lumx1;
	if( maxx < lumx2 ) maxx = lumx2;
	if( maxx < lumx3 ) maxx = lumx3;
	miny = lumy1;
	if( miny > lumy2 ) miny = lumy2;
	if( miny > lumy3 ) miny = lumy3;
	maxy = lumy1;
	if( maxy < lumy2 ) maxy = lumy2;
	if( maxy < lumy3 ) maxy = lumy3;

	// Put the piece in the new basis (nPosX,nPosY)
	nPosX = ((sint32)floor(minx-0.5));
	nPosY = ((sint32)floor(miny-0.5));

	lumx1 -= nPosX; lumy1 -= nPosY;
	lumx2 -= nPosX; lumy2 -= nPosY;
	lumx3 -= nPosX;	lumy3 -= nPosY;

	nSizeX = 1 + ((sint32)floor(maxx+0.5)) - ((sint32)floor(minx-0.5));
	nSizeY = 1 + ((sint32)floor(maxy+0.5)) - ((sint32)floor(miny-0.5));
	Piece.resize( nSizeX*nSizeY );
	for( j = 0; j < nSizeX*nSizeY; ++j )
		Piece[j] = 0;

// The square interact with the triangle if an edge of the square is cut by an edge of the triangle
// Or the square is in the triangle
	
	for( j = 0; j < nSizeY-1; ++j )
	for( k = 0; k < nSizeX-1; ++k )
	{
		// Is the square (j,k) is interacting with the triangle
		// This means : The square contains a point of the triangle (can be done for the 3 points)
		//              The triangle contains a point of the square
		// If so then we have to turn on all the 4 pixels of the square
		if( isInTriangle(k+0.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangle(k+1.5,j+0.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangle(k+0.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) ||
			isInTriangle(k+1.5,j+1.5,lumx1,lumy1,lumx2,lumy2,lumx3,lumy3) )
		{
			Piece[k   + j    * nSizeX] = nCol;
			Piece[1+k + j    * nSizeX] = nCol;
			Piece[k   + (1+j)* nSizeX] = nCol;
			Piece[1+k + (1+j)* nSizeX] = nCol;
		}

		if( segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+0.5, j+0.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+1.5, j+0.5, lumx3, lumy3, lumx1, lumy1) ||

			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx1, lumy1, lumx2, lumy2) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx2, lumy2, lumx3, lumy3) ||
			segmentIntersection(k+1.5, j+1.5, k+0.5, j+1.5, lumx3, lumy3, lumx1, lumy1) )
		{
			Piece[k   + j    * nSizeX] = nCol;
			Piece[1+k + j    * nSizeX] = nCol;
			Piece[k   + (1+j)* nSizeX] = nCol;
			Piece[1+k + (1+j)* nSizeX] = nCol;
		}

	}
	// For all the points of the triangle update the square
	Piece[((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + ((sint32)(lumy1-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx1-0.5)) + (1+((sint32)(lumy1-0.5)))*nSizeX] = nCol;

	Piece[((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + ((sint32)(lumy2-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx2-0.5)) + (1+((sint32)(lumy2-0.5)))*nSizeX] = nCol;
	
	Piece[((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + ((sint32)(lumy3-0.5))*nSizeX] = nCol;
	Piece[((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
	Piece[1+((sint32)(lumx3-0.5)) + (1+((sint32)(lumy3-0.5)))*nSizeX] = nCol;
}

void ResizeBitmap( vector<uint8> &vBitmap, sint32 &nSizeX, sint32 &nSizeY, sint32 nNewSizeX, sint32 nNewSizeY )
{
	vector<uint8> vImgTemp;
	int i, j;

	vImgTemp.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vImgTemp[i] = 0;

	for( j = 0; j < min(nSizeY,nNewSizeY); ++j )
	for( i = 0; i < min(nSizeX,nNewSizeX); ++i )
	{
		vImgTemp[i+j*nNewSizeX] = vBitmap[i+j*nSizeX];
	}

	vBitmap.resize(nNewSizeX*nNewSizeY);
	for( i = 0; i < nNewSizeX*nNewSizeY; ++i )
		vBitmap[i] = vImgTemp[i];

	nSizeX = nNewSizeX;
	nSizeY = nNewSizeY;
}

bool PutPieceInLightMap( vector<uint8>& Piece, sint32 nPieceSizeX, sint32 nPieceSizeY, 
						 vector<uint8>& LightMap, sint32 nLightMapSizeX, sint32 nLightMapSizeY,
						 sint32 &nNewPosX, sint32 &nNewPosY )
{
	sint32 i, j, a, b;
	bool bGoodPosition;

	if( nPieceSizeX > nLightMapSizeX ) return false;
	if( nPieceSizeY > nLightMapSizeY ) return false;

	// For all position test if the piece can get in
	for( j = 0; j < (nLightMapSizeY-nPieceSizeY); ++j )
	for( i = 0; i < (nLightMapSizeX-nPieceSizeX); ++i )
	{
		bGoodPosition = true;
		for( b = 0; b < nPieceSizeY; ++b )
		{
			for( a = 0; a < nPieceSizeX; ++a )
			{
				if( Piece[a+b*nPieceSizeX] != 0 )
					if( LightMap[(i+a)+(j+b)*nLightMapSizeX] != 0 )
					{
						bGoodPosition = false;
						break;
					}
			}
			if( bGoodPosition == false )
				break;
		}
		if( bGoodPosition )
		{
			// Write the piece in the lightmap !!!
			for( b = 0; b < nPieceSizeY; ++b )
			{
				for( a = 0; a < nPieceSizeX; ++a )
				{
					if( Piece[a+b*nPieceSizeX] != 0 )
						LightMap[(i+a)+(j+b)*nLightMapSizeX] = Piece[a+b*nPieceSizeX];
				}
			}
			nNewPosX = i;
			nNewPosY = j;
			return true;
		}
	}

	return false;
}

void MapFace( CMesh::CFace *pFace, vector<CVector> &Vertices, double rRatio )
{
	CVector V01 = Vertices[pFace->Corner[1].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector V02 = Vertices[pFace->Corner[2].Vertex] - Vertices[pFace->Corner[0].Vertex];
	CVector n = V01 ^ V02;
	n.normalize();
	
	// Quantize the normal

	// Table of unitary vector with relevant direction to map The I vector represent the plane normal
	// and the J,K vector the U,V vector
	CMatrix QuantizationTbl[3]; 
	QuantizationTbl[0].identity();
	QuantizationTbl[1].identity(); QuantizationTbl[1].rotateZ(Pi/2);
	QuantizationTbl[2].identity(); QuantizationTbl[2].rotateY(Pi/2);

	float fMax = 0.0f;
	int pos = 0;
	for( int i = 0; i < 3; ++i )
	{
		if( fMax < fabs(QuantizationTbl[i].getI()*n) )
		{
			fMax = fabs(QuantizationTbl[i].getI()*n);
			pos = i;
		}
	}

	// Map with the i_th vector from the quantization table
	// Projection of the 3 vertices of the triangle on the plane 
	// defined by the quantized vector (as the plane normal) and the origin (as a point in the plane)

	// This is equivalent to a base changement with annulation of the I vector
	CMatrix invMat = QuantizationTbl[pos].inverted();
	CVector newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[0].Vertex]);
	pFace->Corner[0].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[0].Uvws[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[1].Vertex]);
	pFace->Corner[1].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[1].Uvws[1].V = newPtinUVBasis.z / rRatio;
	newPtinUVBasis = invMat.mulPoint(Vertices[pFace->Corner[2].Vertex]);
	pFace->Corner[2].Uvws[1].U = newPtinUVBasis.y / rRatio;
	pFace->Corner[2].Uvws[1].V = newPtinUVBasis.z / rRatio;	
}

CMatrix getObjectToWorldMatrix( CMesh::CMeshBuild *pMB )
{
	CMatrix m1, m2, m3, m4, m5;
	m1.identity();
	m1.setPos( pMB->DefaultPivot );
	m1.invert();

	m2.identity();
	m2.scale( pMB->DefaultScale );

	m3.identity();
	m3.setRot( pMB->DefaultRotQuat );

	m4.identity();
	m4.setPos( pMB->DefaultPivot );

	m5.identity();
	m5.setPos( pMB->DefaultPos );

	m1 = m5*m4*m3*m2*m1;
	
	return m1;
}

float getUVDist( CUV& UV1, CUV& UV2 )
{
	return sqrt( (UV2.U - UV1.U)*(UV2.U - UV1.U) + (UV2.V - UV1.V)*(UV2.V - UV1.V) );
}

struct SLightBuild
{
	enum EType { LightAmbient, LightPoint, LightDir, LightSpot };
	EType Type;
	CVector Position;					// Applicable to LightPoint and LightSpot
	CVector Direction;					// Applicable to LightSpot and LightDir
	float rRadiusMin, rRadiusMax;		// Applicable to LightPoint and LightSpot
	CRGBA Ambient;
	CRGBA Diffuse;
	CRGBA Specular;
	SLightBuild()
	{
		Type = LightPoint;
		Position = CVector(0.0, 0.0, 0.0);
		Direction = CVector(1.0, 0.0, 0.0);
		rRadiusMin = 1.0f;
		rRadiusMax = 2.0f;
		Ambient = CRGBA(0, 0, 0, 0);
		Diffuse = CRGBA(0, 0, 0, 0);
		Specular = CRGBA(0, 0, 0, 0);
	}
};




void getLightBuildList(std::vector<SLightBuild>& vectLight, TimeValue tvTime, Interface& ip, INode*node=NULL )
{
	if( node == NULL )
		node = ip.GetRootNode();

	// Get a pointer on the object's node
    Object *obj = node->EvalWorldState(tvTime).obj;

	// Check if there is an object
	if (obj)
	{
		// Get a GenLight from the node
		if (obj->SuperClassID()==LIGHT_CLASS_ID)
		{
			GenLight *maxLight = (GenLight *) obj;

			bool deleteIt=false;
			if (obj != maxLight) 
				deleteIt = true;

			SLightBuild nelLight;

			// Eval the light state fot this tvTime
			Interval valid=NEVER;
			LightState ls;

			if (maxLight->EvalLightState(tvTime, valid, &ls)==REF_SUCCEED)
			{
				// Set the light mode
				switch (maxLight->Type())
				{
					case OMNI_LIGHT:
						nelLight.Type = SLightBuild::EType::LightPoint;
					break;
					case TSPOT_LIGHT:
					case FSPOT_LIGHT:
						nelLight.Type = SLightBuild::EType::LightSpot;
					break;
					case DIR_LIGHT:
					case TDIR_LIGHT:
						nelLight.Type = SLightBuild::EType::LightDir;
					break;
					default:
						// Not initialized
					break;
				}

				// *** Set the light color

				// Get the color
				CRGBA nelColor;
				Point3 maxColor = maxLight->GetRGBColor(tvTime);

				// Mul by multiply
				CRGBAF nelFColor;
				nelFColor.R = maxColor.x;
				nelFColor.G = maxColor.y;
				nelFColor.B = maxColor.z;
				nelFColor.A = 1.f;
				nelFColor   *= maxLight->GetIntensity(tvTime);
				nelColor = nelFColor;

				// Affect the ambiant color ?
				nelLight.Ambient = nelColor;
				nelLight.Diffuse = CRGBA (0,0,0);
				nelLight.Specular = CRGBA (0,0,0);
				if (maxLight->GetAmbientOnly())
				{
					nelLight.Ambient = nelColor;
				}
				else
				{
					// Affect the diffuse color ?
					if( maxLight->GetAffectDiffuse() )
						nelLight.Diffuse = nelColor;
					// Affect the specular color ?
					if (maxLight->GetAffectSpecular())
						nelLight.Specular = nelColor;
				}

				// Set the light position
				Point3 pos = node->GetNodeTM(tvTime).GetTrans ();
				CVector position;
				position.x=pos.x;
				position.y=pos.y;
				position.z=pos.z;

				// Set the position
				nelLight.Position = position;

				// Set the light direction
				CVector direction;
				INode* target = node->GetTarget ();
				if (target)
				{
					// Get the position of the target
					Point3 posTarget=target->GetNodeTM (tvTime).GetTrans ();
					CVector positionTarget;
					positionTarget.x=posTarget.x;
					positionTarget.y=posTarget.y;
					positionTarget.z=posTarget.z;

					// Direction
					direction=positionTarget-position;
					direction.normalize ();
				}
				else	// No target
				{
					// Get orientation of the source as direction
					CMatrix nelMatrix;
					CExportNel::convertMatrix (nelMatrix, node->GetNodeTM(tvTime));

					// Direction is -Z
					direction=-nelMatrix.getK();
					direction.normalize ();
				}

				// Set the direction
				nelLight.Direction = direction;
				/*
				// Set spot light information
				nelLight.setCutoff ((float)(NLMISC::Pi*maxLight.GetFallsize(tvTime)/180.f/2.f));

				// Compute the exponent value
				float angle=(float)(NLMISC::Pi*maxLight.GetHotspot(tvTime)/(2.0*180.0));
				nelLight.setupSpotExponent (angle);

				// *** Set attenuation

				if (maxLight.GetUseAtten())
					nelLight.setupAttenuation (maxLight.GetAtten (tvTime, ATTEN_START), maxLight.GetAtten (tvTime, ATTEN_END));
				else
					nelLight.setNoAttenuation ();
				*/
				if (maxLight->GetUseAtten())
				{
					nelLight.rRadiusMin = maxLight->GetAtten (tvTime, ATTEN_START);
					nelLight.rRadiusMax = maxLight->GetAtten (tvTime, ATTEN_END);
				}
				// Add the light in the list
				vectLight.push_back (nelLight);
			}
			if( deleteIt )
				delete maxLight;
		}
	}

	// Recurse sub node
	for (int i=0; i<node->NumberOfChildren(); i++)
		getLightBuildList(vectLight, tvTime, ip, node->GetChildNode(i));
}

void getLightBuilds( vector<SLightBuild> &lights, TimeValue tvTime, Interface& ip )
{
	SLightBuild amb;

	amb.Type = SLightBuild::EType::LightAmbient;
	amb.Ambient.R = (uint8)(ip.GetAmbient( tvTime, FOREVER ).x*255);
	amb.Ambient.G = (uint8)(ip.GetAmbient( tvTime, FOREVER ).y*255);
	amb.Ambient.B = (uint8)(ip.GetAmbient( tvTime, FOREVER ).z*255);
	amb.Ambient.A = 255;
	amb.Specular = amb.Diffuse = CRGBA(0,0,0,0);
	lights.push_back( amb );
	getLightBuildList( lights, tvTime, ip );

}


// -----------------------------------------------------------------------------------------------
// -----------------------------------------------------------------------------------------------
bool CNelExport::exportScene(std::vector<INode*>& vectNode)
{
	// All inputs are transfered throught the global theExportSceneStruct structure

	// Export the Instance Group (aka scene description)
	// *************************
	if( theExportSceneStruct.bExportInstanceGroup )
		exportInstanceGroup( theExportSceneStruct.sExportInstanceGroup, vectNode );

	// Export all the shapes
	// *********************
	// If lighting enabled we export the shape after lightmapping modification
	if( theExportSceneStruct.bExportShapes && (!theExportSceneStruct.bExportLighting) )
	{
		// Get time
		TimeValue tvTime = theCNelExport.ip->GetTime();

		// Get node count
		int nNumSelNode = vectNode.size();

		// Save all selected objects
		for (int nNode=0; nNode<nNumSelNode; nNode++)
		{
			// Get the node
			INode* pNode=vectNode[nNode];

			// It is a zone ?
			if (RPO::isZone (*pNode, tvTime))
			{
				// Nothing to do if this is a zone
			}
			// Try to export a mesh
			else if (CExportNel::isMesh (*pNode, tvTime))
			{
				// Build the name of the file to export
				char sSavePath[512];
				strcpy( sSavePath, theExportSceneStruct.sExportShapes.c_str() );
				if(theExportSceneStruct.sExportShapes[theExportSceneStruct.sExportShapes.size()-1] != '\\' )
					strcat( sSavePath, "\\" );
				strcat( sSavePath, pNode->GetName() );
				strcat( sSavePath, ".shape" );

				// Export the mesh
				if (!theCNelExport.exportMesh (sSavePath, *pNode, *theCNelExport.ip, tvTime))
				{
					// Error message
					char sErrorMsg[512];
					sprintf (sErrorMsg, "Error exporting the mesh %s in the file\n%s", pNode->GetName(), sSavePath);
					MessageBox (theCNelExport.ip->GetMAXHWnd(), sErrorMsg, "NeL export scene", MB_OK|MB_ICONEXCLAMATION );
				}
			}
		}
	}

	// Export the lighting (aka compute all light maps)
	// *******************
	#define RATIOLIGHTMAP	0.05		// 0.5 meters in real world represent 1 Lumel in UV world
	#define MAXLIGHTMAPSIZE 1024
	if( theExportSceneStruct.bExportLighting )
	{
		// 1 -> Build all MeshBuild structure
		// ----------------------------------
		vector< pair < CMesh::CMeshBuild*,INode* > > AllMeshBuilds;
		TimeValue tvTime = theCNelExport.ip->GetTime();
		int nNumSelNode = vectNode.size();
		int nNbMesh = 0;
		int nNode, i, j, k;
		// Count number of mesh in the selection
		for( nNode = 0; nNode < nNumSelNode; ++nNode )
		{
			INode* pNode = vectNode[nNode];
			if( ! RPO::isZone( *pNode, tvTime ) )
			if( CExportNel::isMesh( *pNode, tvTime ) )
				nNbMesh++;
		}
		AllMeshBuilds.resize(nNbMesh);
		nNbMesh = 0;
		// Create all the MeshBuild used to place lightmaps and to raytrace lightmaps
		for( nNode=0; nNode < nNumSelNode; ++nNode )
		{
			INode* pNode = vectNode[nNode];
			if( ! RPO::isZone( *pNode, tvTime ) )
			if( CExportNel::isMesh( *pNode, tvTime ) )
			{
				AllMeshBuilds[nNbMesh].second = pNode;
				AllMeshBuilds[nNbMesh].first = CExportNel::createMeshBuild( *pNode, tvTime );
				nNbMesh++;
			}
		}
		// 2 -> Place lightmap for all MeshBuild (this operation does not need interaction between MeshBuilds)
		for( nNode=0; nNode < nNbMesh; ++nNode )
		{
			// First order face by Material and by texture surface
			CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
			CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
			vector<CMesh::CFace*> AllFaces;
			vector<CVector> AllVertices; // All vertices in world space
			vector<sint32> AreaTextureNames; // Number of faces with the same texture name
			sint32 nNbFace = pMB->Faces.size(), nNbVertex = pMB->Vertices.size();
			sint32 offset, nNewPosX, nNewPosY;
			// Make a vector of pointer to all the faces of the MeshBuild
			AllFaces.resize(nNbFace);
			for( i = 0; i < nNbFace; ++i )
				AllFaces[i] = &pMB->Faces[i];
			AllVertices.resize(nNbVertex);
			for( i = 0; i < nNbVertex; ++i )
				AllVertices[i] = MBMatrix * pMB->Vertices[i];
			// Bubble sort pointer to the faces (Material sorting by texture name)
			SortFaceByTextureName( AllFaces, pMB );
			// Get the number of faces in Allfaces for each different texture name
			ComputeAreaOfTextureName( AreaTextureNames, AllFaces, pMB );

			// For all faces that has the same texture name
			offset = 0;
			for( int nTexName = 0; nTexName < AreaTextureNames.size(); ++nTexName )
			{
				nNbFace = AreaTextureNames[nTexName];
				// Bubble sort pointer to the faces (Material sorting)
				SortFaceByMaterialId( offset, nNbFace, AllFaces );
				// Sort by texture surface if the faces have the same material
				SortFaceByTextureSurface( offset, nNbFace, AllFaces );
				offset += nNbFace;
			}
			
			// Here is the placement
			// TEST 1 : One all big lightmap for this object
			//sint32 CurrentMaterialId = AllFaces[0]->MaterialId;
			//sint32 BeginningFace = 0;
			vector<uint8> TheLightMap;
			sint32 nLightMapSizeX = 1, nLightMapSizeY = 1;
			TheLightMap.resize(1);
			nNbFace = pMB->Faces.size();
			// Calculation of the texture ratio
			double SpaceDist = 0.0, TextureDist = 0.0;
			for( i = 0; i < nNbFace; ++i )
			{
				CVector v = AllVertices[AllFaces[i]->Corner[1].Vertex] - AllVertices[AllFaces[i]->Corner[0].Vertex];
				SpaceDist += v.norm();
				v = AllVertices[AllFaces[i]->Corner[2].Vertex] - AllVertices[AllFaces[i]->Corner[1].Vertex];
				SpaceDist += v.norm();
				v = AllVertices[AllFaces[i]->Corner[0].Vertex] - AllVertices[AllFaces[i]->Corner[2].Vertex];
				SpaceDist += v.norm();
				TextureDist += getUVDist( AllFaces[i]->Corner[1].Uvws[1], AllFaces[i]->Corner[0].Uvws[1] );
				TextureDist += getUVDist( AllFaces[i]->Corner[2].Uvws[1], AllFaces[i]->Corner[1].Uvws[1] );
				TextureDist += getUVDist( AllFaces[i]->Corner[0].Uvws[1], AllFaces[i]->Corner[2].Uvws[1] );
			}
			double LMTextRatio = SpaceDist / TextureDist;
			LMTextRatio = LMTextRatio / RATIOLIGHTMAP;

			for( i = 0; i < nNbFace; ++i )
			{
				// Piece creation
				vector<uint8> Piece;
				sint32 nPieceSizeX, nPieceSizeY;				
				// Try to map this face with quantized vector and express uv in lumel
				// MapFace( AllFaces[i], AllVertices, RATIOLIGHTMAP );
				for( j = 0; j < 3; ++j ) // Express the UVs in lumel with a ratio
				{	// For each corner
					AllFaces[i]->Corner[j].Uvws[1].U *= LMTextRatio;
					AllFaces[i]->Corner[j].Uvws[1].V *= LMTextRatio;
				}
				// Create the related piece
				CreatePiece( Piece, nPieceSizeX, nPieceSizeY, nNewPosX, nNewPosY,
							 AllFaces[i]->Corner[0].Uvws[1].U,	// lumx1
							 AllFaces[i]->Corner[0].Uvws[1].V,	// lumy1
							 AllFaces[i]->Corner[1].Uvws[1].U,	// lumx2
							 AllFaces[i]->Corner[1].Uvws[1].V,	// lumy2
							 AllFaces[i]->Corner[2].Uvws[1].U,	// lumx3
							 AllFaces[i]->Corner[2].Uvws[1].V,	// lumy3
							 (i%254)+1 ); // color
				// Set the UV of the face in same basis as the piece (0->nPieceSize)
				for( j = 0; j < 3; ++j )
				{	// For each corner
					AllFaces[i]->Corner[j].Uvws[1].U -= nNewPosX;
					AllFaces[i]->Corner[j].Uvws[1].V -= nNewPosY;
				}

				while( true )
				{
					if( !PutPieceInLightMap( Piece, nPieceSizeX, nPieceSizeY, 
											 TheLightMap, nLightMapSizeX, nLightMapSizeY,
											 nNewPosX, nNewPosY ) )
					{
						if( ( nLightMapSizeX < MAXLIGHTMAPSIZE ) || ( nLightMapSizeY < MAXLIGHTMAPSIZE ) )
						{
							if( nLightMapSizeX < nLightMapSizeY )
								ResizeBitmap( TheLightMap, nLightMapSizeX, nLightMapSizeY, nLightMapSizeX*2, nLightMapSizeY );
							else
								ResizeBitmap( TheLightMap, nLightMapSizeX, nLightMapSizeY, nLightMapSizeX, nLightMapSizeY*2 );
						}
						else
						{
							// ERROR: we reached the maximum texture size
							nlstop;
						}
					}
					else
					{
						// We found a position for this piece, update texture coord in lumel format
						for( j = 0; j < 3; ++j )
						{
							AllFaces[i]->Corner[j].Uvws[1].U += nNewPosX;
							AllFaces[i]->Corner[j].Uvws[1].V += nNewPosY;
						}
						break;
					}
				}
			}
			// All faces have been put in the lightmap so update All texture coord with the size of the lightmap
			// Convert all faces from lumels to lumels/sizeofLightMap to get uv from 0.0 to 1.0
			for( i = 0; i < nNbFace; ++i )
			for( j = 0; j < 3; ++j )
			{
				AllFaces[i]->Corner[j].Uvws[1].U /= nLightMapSizeX;
				AllFaces[i]->Corner[j].Uvws[1].V /= nLightMapSizeY;
			}
			// Write the new lightmap and update the buildmesh materials
			{
				CTextureFile *pTexture = new CTextureFile;
				string sTemp = AllMeshBuilds[nNode].second->GetName();
				sTemp += ".tga";
				pTexture->setFileName( sTemp );
				sTemp = theExportSceneStruct.sExportLighting;
				if(sTemp[sTemp.size()-1] != '\\' ) sTemp += "\\";
				sTemp += AllMeshBuilds[nNode].second->GetName();
				sTemp += ".tga";
				pTexture->resize( nLightMapSizeX, nLightMapSizeY );
				CObjectVector<uint8> &rText = pTexture->getPixels();
				for( j = 0; j < nLightMapSizeY; ++j )
				for( i = 0; i < nLightMapSizeX; ++i )
					if( TheLightMap[i+j*nLightMapSizeX] != 0 )
					{
						rText[(i+j*nLightMapSizeX)*4+0] = 63+((TheLightMap[i+j*nLightMapSizeX]*15)%190);
						rText[(i+j*nLightMapSizeX)*4+1] = 63+((TheLightMap[i+j*nLightMapSizeX]*27+63)%190);
						rText[(i+j*nLightMapSizeX)*4+2] = 63+((TheLightMap[i+j*nLightMapSizeX]*13+127)%190);
						rText[(i+j*nLightMapSizeX)*4+3] = 255;
					}
					else
					{
						rText[(i+j*nLightMapSizeX)*4+0] = 0;
						rText[(i+j*nLightMapSizeX)*4+1] = 0;
						rText[(i+j*nLightMapSizeX)*4+2] = 0;
						rText[(i+j*nLightMapSizeX)*4+3] = 0;
					}
				// Write texture to disk
				//{
				//	COFile f;
				//	f.open( sTemp );
				//	pTexture->writeTGA( f, 32 );
				//}
				for( i = 0; i < AllMeshBuilds[nNode].first->Materials.size(); ++i )
				{
					AllMeshBuilds[nNode].first->Materials[i].setTexture( 1, pTexture );
					//AllMeshBuilds[nNode].first->Materials[i].setLighting( false );
				}
			}
		} // End for all nodes
		// All nodes are mapped correctly with a lightmap and lightmap is created on disk
		// --------
		// RAYTRACE the scene
		// --------
		// 0 - Get Lights from 3DSMAX
		vector<SLightBuild> vLights;
		getLightBuilds( vLights, tvTime, *theCNelExport.ip );
// Old version (with lighting for the Nel Core Engine)
//		vector<CLight> vLights;
//		CExportNel::getLights( vLights, tvTime, *theCNelExport.ip );

		// 1 - Light with the normal and the lights
		for( nNode=0; nNode < nNbMesh; ++nNode )
		{
			CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
			CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
			vector<CVector> AllVertices; // All vertices in world space

			AllVertices.resize(pMB->Vertices.size());
			for( i = 0; i < pMB->Vertices.size(); ++i )
				AllVertices[i] = MBMatrix * pMB->Vertices[i];

			// Select all lights that interact with this mesh
			
			sint32 nNbFace = pMB->Faces.size();
			MBMatrix.invert();
			MBMatrix.transpose();
			for( i = 0; i < nNbFace; ++i )
			{
				vector<uint8> Piece;
				sint32 nPieceSizeX, nPieceSizeY, nPiecePosX, nPiecePosY;
				// Get the lightmap from the material of the face
				ITexture *pText = pMB->Materials[pMB->Faces[i].MaterialId].getTexture( 1 );
				CTextureFile *pLightMap = dynamic_cast<CTextureFile*>(pText);
				if( pLightMap == NULL )
					continue;
				double	u1 = pMB->Faces[i].Corner[0].Uvws[1].U * pLightMap->getWidth(),
						v1 = pMB->Faces[i].Corner[0].Uvws[1].V * pLightMap->getHeight(),
						u2 = pMB->Faces[i].Corner[1].Uvws[1].U * pLightMap->getWidth(), 
						v2 = pMB->Faces[i].Corner[1].Uvws[1].V * pLightMap->getHeight(),
						u3 = pMB->Faces[i].Corner[2].Uvws[1].U * pLightMap->getWidth(), 
						v3 = pMB->Faces[i].Corner[2].Uvws[1].V * pLightMap->getHeight();
				CVector p1 = AllVertices[pMB->Faces[i].Corner[0].Vertex],
						p2 = AllVertices[pMB->Faces[i].Corner[1].Vertex],
						p3 = AllVertices[pMB->Faces[i].Corner[2].Vertex];
				CVector n1 = MBMatrix.mulVector( pMB->Faces[i].Corner[0].Normal ),
						n2 = MBMatrix.mulVector( pMB->Faces[i].Corner[1].Normal ),
						n3 = MBMatrix.mulVector( pMB->Faces[i].Corner[2].Normal );
				n1.normalize(); n2.normalize(); n3.normalize();
				// Denominator Gradient along u and v
				double GradDen = 1.0 / ( (u3-u1)*(v2-v1) - (u2-u1)*(v3-v1) );

				// Get the mask
				CreatePiece( Piece, nPieceSizeX, nPieceSizeY, nPiecePosX, nPiecePosY,
							 u1, v1, u2, v2, u3, v3, 1 );
				// Parse the mask
				for( k = 0; k < nPieceSizeY; ++k )
				for( j = 0; j < nPieceSizeX; ++j )
				if( Piece[j+k*nPieceSizeX] != 0 ) // Must this point be lighted ?
				{	// Yes
					CRGBAF theColor;
					// Convert the (u,v) to world space coord (use gradient of x along u and v) (same for y,z...)
					double theU = nPiecePosX + j + 0.5;
					double theV = nPiecePosY + k + 0.5;
					CVector p, n; // p is the point in the world space

					double GradU = ( (p3.x-p1.x)*(v2-v1)-(p2.x-p1.x)*(v3-v1) ) * GradDen;
					double GradV = ( (p2.x-p1.x)*(u3-u1)-(p3.x-p1.x)*(u2-u1) ) * GradDen;
					p.x = GradU*(theU-u1) + GradV*(theV-v1) + p1.x;

					GradU = ( (p3.y-p1.y)*(v2-v1)-(p2.y-p1.y)*(v3-v1) ) * GradDen;
					GradV = ( (p2.y-p1.y)*(u3-u1)-(p3.y-p1.y)*(u2-u1) ) * GradDen;
					p.y = GradU*(theU-u1) + GradV*(theV-v1) + p1.y;

					GradU = ( (p3.z-p1.z)*(v2-v1)-(p2.z-p1.z)*(v3-v1) ) * GradDen;
					GradV = ( (p2.z-p1.z)*(u3-u1)-(p3.z-p1.z)*(u2-u1) ) * GradDen;
					p.z = GradU*(theU-u1) + GradV*(theV-v1) + p1.z;

					// The same for the normal
					
					GradU = ( (n3.x-n1.x)*(v2-v1)-(n2.x-n1.x)*(v3-v1) ) * GradDen;
					GradV = ( (n2.x-n1.x)*(u3-u1)-(n3.x-n1.x)*(u2-u1) ) * GradDen;
					n.x = GradU*(theU-u1) + GradV*(theV-v1) + n1.x;

					GradU = ( (n3.y-n1.y)*(v2-v1)-(n2.y-n1.y)*(v3-v1) ) * GradDen;
					GradV = ( (n2.y-n1.y)*(u3-u1)-(n3.y-n1.y)*(u2-u1) ) * GradDen;
					n.y = GradU*(theU-u1) + GradV*(theV-v1) + n1.y;

					GradU = ( (n3.z-n1.z)*(v2-v1)-(n2.z-n1.z)*(v3-v1) ) * GradDen;
					GradV = ( (n2.z-n1.z)*(u3-u1)-(n3.z-n1.z)*(u2-u1) ) * GradDen;
					n.z = GradU*(theU-u1) + GradV*(theV-v1) + n1.z;
					
					// --- n = n1 + n2 + n3;
					n.normalize();

					
					theColor.R = theColor.G = theColor.B = theColor.A = 0.0;
					// Color calculation
					for( sint32 nLight = 0; nLight < vLights.size(); ++nLight )
					{
						SLightBuild rLight = vLights[nLight];
						CRGBAF lightAmbiCol = CRGBAF(0.0, 0.0, 0.0, 0.0);
						CRGBAF lightDiffCol = CRGBAF(0.0, 0.0, 0.0, 0.0);
						CRGBAF lightSpecCol = CRGBAF(0.0, 0.0, 0.0, 0.0);

						switch( rLight.Type )
						{
							case SLightBuild::LightAmbient:
								lightAmbiCol.R = rLight.Ambient.R / 255.0;
								lightAmbiCol.G = rLight.Ambient.G / 255.0;
								lightAmbiCol.B = rLight.Ambient.B / 255.0;
								lightAmbiCol.A = rLight.Ambient.A / 255.0;
							break;
							case SLightBuild::LightPoint:
							{
								CVector p_light = rLight.Position - p;
								double p_light_distance = p_light.norm();
								double light_intensity;
								if( p_light_distance < rLight.rRadiusMin )
									light_intensity = 1.0;
								else
								if( p_light_distance > rLight.rRadiusMax )
									light_intensity = 0.0;
								else
									light_intensity = 1.0 - (p_light_distance-rLight.rRadiusMin)/(rLight.rRadiusMax-rLight.rRadiusMin);
								p_light.normalize();

								// ??? light_intensity *= light_intensity * light_intensity;
								lightDiffCol.R = light_intensity * rLight.Diffuse.R * max(0.0f, n*p_light) / 255.0;
								lightDiffCol.G = light_intensity * rLight.Diffuse.G * max(0.0f, n*p_light) / 255.0;
								lightDiffCol.B = light_intensity * rLight.Diffuse.B * max(0.0f, n*p_light) / 255.0;
								lightDiffCol.A = light_intensity * rLight.Diffuse.A * max(0.0f, n*p_light) / 255.0;
							}
							break;
							case SLightBuild::LightDir:
							{
								CVector light = - rLight.Direction;
								light.normalize();
								lightDiffCol.R = rLight.Diffuse.R * max(0.0f, n*light) / 255.0;
								lightDiffCol.G = rLight.Diffuse.G * max(0.0f, n*light) / 255.0;
								lightDiffCol.B = rLight.Diffuse.B * max(0.0f, n*light) / 255.0;
								lightDiffCol.A = rLight.Diffuse.A * max(0.0f, n*light) / 255.0;
							}
							break;
							case SLightBuild::LightSpot:
								// TODO
							break;
							default:
							break;
						}
						theColor.R += lightAmbiCol.R + lightDiffCol.R;// * matDiffCol.R;
						if( theColor.R > 1.0 ) theColor.R = 1.0;
						theColor.G += lightAmbiCol.G + lightDiffCol.G;// * matDiffCol.G;
						if( theColor.G > 1.0 ) theColor.G = 1.0;
						theColor.B += lightAmbiCol.B + lightDiffCol.B;// * matDiffCol.B;
						if( theColor.B > 1.0 ) theColor.B = 1.0;
						theColor.A += lightAmbiCol.A + lightDiffCol.A;// * matDiffCol.A;
						if( theColor.A > 1.0 ) theColor.A = 1.0;
					}					
					// Affect the color

		theColor.R = (1+n.x)/2;
		theColor.G = (1+n.y)/2;
		theColor.B = (1+n.z)/2;
		theColor.A = 1.;

					CObjectVector<uint8> &pixel = pLightMap->getPixels();
					pixel[(j+nPiecePosX + (k+nPiecePosY)*pLightMap->getWidth())*4+0] = theColor.R*255;
					pixel[(j+nPiecePosX + (k+nPiecePosY)*pLightMap->getWidth())*4+1] = theColor.G*255;
					pixel[(j+nPiecePosX + (k+nPiecePosY)*pLightMap->getWidth())*4+2] = theColor.B*255;
					pixel[(j+nPiecePosX + (k+nPiecePosY)*pLightMap->getWidth())*4+3] = theColor.A*255;
				}				
			}
			// Write the new LightMap
			{
				COFile f;
				ITexture *pText = pMB->Materials[0].getTexture( 1 );
				CTextureFile *pLightMap = dynamic_cast<CTextureFile*>(pText);
				string sTemp = theExportSceneStruct.sExportLighting;
				if(sTemp[sTemp.size()-1] != '\\' ) sTemp += "\\";
				sTemp += pLightMap->getFileName();
				f.open( sTemp );
				pLightMap->writeTGA( f, 32 );
			}

		}

		// End of the lighting process for this node we have to export the data
		for( nNode=0; nNode < nNbMesh; ++nNode )
		{
			// First order face by Material and by texture surface
			CMesh::CMeshBuild *pMB = AllMeshBuilds[nNode].first;
			CMesh* mesh = new CMesh;
			pMB->VertexFlags |= CVertexBuffer::TexCoord1Flag;
			// Build the mesh with the build interface

			// TEMP YOYO (DU) DE SA RACE
			pMB->VertexFlags |= CVertexBuffer::PrimaryColorFlag;
			CMatrix MBMatrix = getObjectToWorldMatrix( pMB );
			for( i = 0; i < pMB->Faces.size(); ++i )			
			{
				CVector n = MBMatrix.mulVector(pMB->Faces[i].Corner[0].Normal);
				pMB->Faces[i].Corner[0].Color.R = 255.0 * (n.x+1.0)/2.0;
				pMB->Faces[i].Corner[0].Color.G = 255.0 * (n.y+1.0)/2.0;
				pMB->Faces[i].Corner[0].Color.B = 255.0 * (n.z+1.0)/2.0;
				pMB->Faces[i].Corner[0].Color.A = 255.0;
				n = MBMatrix.mulVector(pMB->Faces[i].Corner[1].Normal);
				pMB->Faces[i].Corner[1].Color.R = 255.0 * (n.x+1.0)/2.0;
				pMB->Faces[i].Corner[1].Color.G = 255.0 * (n.y+1.0)/2.0;
				pMB->Faces[i].Corner[1].Color.B = 255.0 * (n.z+1.0)/2.0;
				pMB->Faces[i].Corner[1].Color.A = 255.0;
				n = MBMatrix.mulVector(pMB->Faces[i].Corner[2].Normal);
				pMB->Faces[i].Corner[2].Color.R = 255.0 * (n.x+1.0)/2.0;
				pMB->Faces[i].Corner[2].Color.G = 255.0 * (n.y+1.0)/2.0;
				pMB->Faces[i].Corner[2].Color.B = 255.0 * (n.z+1.0)/2.0;
				pMB->Faces[i].Corner[2].Color.A = 255.0;
			}
			// TEMP YOYO (DU) DE SA RACE end

			for( i = 0; i < pMB->Materials.size(); ++i )
			{
				pMB->Materials[i].setLighting( false );
				pMB->Materials[i].setColor( CRGBA(255,255,255,255) );
			}
			// *** TMP : force material to be animatable
			// TODO: check if material are animated
			//for (uint i=0; i<buildMesh.Materials.size(); i++)
			//{
			//	mesh->setAnimatedMaterial (i, materialNames[i]);
			//}

			mesh->build( *pMB );


			COFile file;
			char sSavePath[512];
			strcpy( sSavePath, theExportSceneStruct.sExportShapes.c_str() );
			if(theExportSceneStruct.sExportShapes[theExportSceneStruct.sExportShapes.size()-1] != '\\' )
				strcat( sSavePath, "\\" );
			strcat( sSavePath, AllMeshBuilds[nNode].second->GetName() );
			strcat( sSavePath, ".shape" );
			if (file.open( sSavePath ))
			{
				try
				{
					// Create a streamable shape
					CShapeStream shapeStream( mesh );
					// Serial the shape
					shapeStream.serial (file);
				}
				catch (...)
				{
				}
			}

			// Delete the pointer
			delete mesh;
		}

		// ? -> Ending deletion des objets temporaires
		for( nNode = 0; nNode < nNbMesh; ++nNode )
			delete AllMeshBuilds[nNode].first;
	} // End if lighting process asked

	return true;
}

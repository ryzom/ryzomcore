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

#include "std3d.h"
#include "nel/3d/zone_symmetrisation.h"
#include "nel/3d/zone.h"
#include "nel/3d/tile_bank.h"

using namespace std;
using namespace NLMISC;

namespace NL3D
{

// ***************************************************************************

CZoneSymmetrisation::CZoneSymmetrisation()
{
}

// ***************************************************************************

/*
Bit field
	1-0 : state layer 0
	3-2 : state layer 1
	5-4 : state layer 2
	7-6 : border state
	9-8 : oriented border state
	10  : corner flag
*/

// ***************************************************************************

CZoneSymmetrisation::TState CZoneSymmetrisation::getTileState (uint patch, uint tile, uint layer) const
{
	nlassert (layer<5);
	return (TState)((_TilesLayerStates[patch][tile]>>(layer*2))&0x3);
}

// ***************************************************************************

CZoneSymmetrisation::TState CZoneSymmetrisation::getTileBorderState (uint patch, uint tile) const
{
	return getTileState (patch, tile, 3);
}

// ***************************************************************************

CZoneSymmetrisation::TState CZoneSymmetrisation::getOrientedTileBorderState (uint patch, uint tile) const
{
	return getTileState (patch, tile, 4);
}

// ***************************************************************************

bool CZoneSymmetrisation::getOrientedTileCorner (uint patch, uint tile)
{
	return ( ( _TilesLayerStates[patch][tile] >> 10 ) & 1 ) != 0;
}

// ***************************************************************************

void CZoneSymmetrisation::setTileState (uint patch, uint tile, uint layer, TState state)
{
	uint16 &ref = _TilesLayerStates[patch][tile];
	ref &= ~(3<<(layer*2));
	ref |= ((uint16)state)<<(layer*2);
}

// ***************************************************************************

void CZoneSymmetrisation::setTileBorderState (uint patch, uint tile, TState state)
{
	setTileState (patch, tile, 3, state);
}

// ***************************************************************************

void CZoneSymmetrisation::setOrientedTileBorderState (uint patch, uint tile, TState state)
{
	setTileState (patch, tile, 4, state);
}

// ***************************************************************************

void CZoneSymmetrisation::setOrientedTileCorner (uint patch, uint tile, bool corner)
{
	uint16 &ref = _TilesLayerStates[patch][tile];
	ref &= ~(1<<10);
	ref |= ((uint16)corner)<<(10);
}

// ***************************************************************************

/*

REMARKS:
- I call "isotile" the set of connected tiles in the zone using the same tileset and a continus rotation value.

This method determines the state of each tile in the zone. This state will be used to transform
the tile during symmetrisation of the zone.

The state can be Regular, Goofy or Nothing.

If the state is Nothing, this tile is in an "isotile" that is not connected to a zone border. No matter the method
you use to tranform it, it will not influence the neighbor zones.

If the state is Regular, the rotation of this tile after symmetrisation will by given by this formula : tilerot = (4-tilerot)&3
If the state is Goofy, the rotation of this tile after symmetrisation will by given by this formula : tilerot = (4-tilerot+2)&3

- Getting the state of the tile:
	- A) We flag all the zone patches as Nothing.
	- B) We need to select patches having an open edge on the zone border. To do so, we use the snapCell and weldThreshold
	parameters.
	- C) Then, for each patches on the zone border, we need to know if they are Goofy or Regular.

		Y

		/\
		|
		|  0     3
		|   *****
		|   *   *   This patch is regular
		|   *   *
		|   *****
		|  1     2
		|
		|  2     1
		|   *****
		|   *   *   This patch is regular
		|   *   *
		|   *****
		|  3     0
		|
		|  3     2
		|   *****
		|   *   *   This patch is goofy
		|   *   *
		|   *****
		|  0     1
		|
		|  1     4
		|   *****
		|   *   *   This patch is goofy
		|   *   *
		|   *****
		|  2     3
		-----------------------------------------> X

	- D) We flag each tiles as Nothing
	- E) We flag each tile on an opened edge using this formula:
		- If the patch is not Nothing do
			- tileIsGoofy = (patchIsGoofy) XOR ((tileRotation&1) != 0)
	- F) Then, we propagate the tile A flag across the tiles in the zone following this rules:
		- If A is not Nothing do
			- For each neighbor B of A do
				- If B is different from A do
					- If A is Regular (res Goofy), and B is Nothing, B will be Regular (res Goofy)
					- If A is Regular (res Goofy), and B is Goofy (res Regular), -> Error
					- Propagate B
*/

bool CZoneSymmetrisation::build (const std::vector<CPatchInfo> &patchInfo, float snapCell, float weldThreshold, const CTileBank &bank, CError &errorDesc, const NLMISC::CMatrix &toOriginalSpace)
{
	// * Clear errors
	errorDesc.Errors.clear ();

	// * Build the patches state

	// A) Resize arrays
	_TilesLayerStates.resize (patchInfo.size ());

	// D) Resize the tile array
	uint i;
	for (i=0; i<patchInfo.size (); i++)
	{
		// Ref on the patch
		const CPatchInfo &patch = patchInfo[i];
		_TilesLayerStates[i].resize (0);
		_TilesLayerStates[i].resize (patch.OrderS * patch.OrderT, 0);
	}

	// B), C) and E) We need to select patches having an open edge on the zone border. To do so, we use the snapCell and weldThreshold parameters
	for (i=0; i<patchInfo.size (); i++)
	{
		// Ref on the patch
		const CPatchInfo &patch = patchInfo[i];

		// Does this patch is over a border ?
		TState patchState;
		if (!setTileState (patch, i, snapCell, weldThreshold, patchState, toOriginalSpace, bank))
		{
			// Push an error
			errorDesc.Errors.push_back ("Patch nb "+toString (i)+" is invalid");
		}

		// Set the oriented patch state
		if (!setOrientedTileState (patch, i, snapCell, weldThreshold, patchState, toOriginalSpace, bank))
		{
			// Push an error
			errorDesc.Errors.push_back ("Patch nb "+toString (i)+" is invalid");
		}
	}

	// F) We flag each tile on an opened edge using this formula
	//	- If the patch is not Nothing do
	//		- tileIsGoofy = (patchIsGoofy) XOR ((tileRotation&1) != 0)
	for (i=0; i<patchInfo.size (); i++)
	{
		// Ref on the patch
		const CPatchInfo &patch = patchInfo[i];

		// For each tile
		uint s,t;
		for (t=0; t<patch.OrderT; t++)
		{
			for (s=0; s<patch.OrderS; s++)
			{
				if (!propagateTileState (i, s, t, patchInfo, bank, false))
				{
					// Push an error
					errorDesc.Errors.push_back ("Error during propagation. Topology invalid.");
					return false;
				}
			}
		}
	}

	// G) Force all remaining Nothing tiles to Regular
	for (i=0; i<patchInfo.size (); i++)
	{
		// Ref on the patch
		const CPatchInfo &patch = patchInfo[i];

		// For each tile
		uint s,t;
		for (t=0; t<patch.OrderT; t++)
		{
			for (s=0; s<patch.OrderS; s++)
			{
				if (!propagateTileState (i, s, t, patchInfo, bank, true))
				{
					// Push an error
					errorDesc.Errors.push_back ("Error during propagation. Topology invalid.");
					return false;
				}
			}
		}
	}

	// Returns true if no error
	return true; // errorDesc.Errors.size () == 0;
}

// ***************************************************************************

bool CZoneSymmetrisation::snapOnGrid (float& value, float resolution, float snap)
{
	// Calc the floor
	float _floor = (float) ( resolution * floor (value / resolution) );
	nlassert (_floor<=value);

	// Calc the remainder
	float remainder = value - _floor;
	//nlassert ( (remainder>=0) && (remainder<resolution) );

	// Check the snape
	if ( remainder <= snap )
	{
		// Flag it
		value = _floor;

		// Floor is good
		return true;
	}
	else if ( (resolution - remainder) <= snap )
	{
		// Flag it
		value = _floor + resolution;

		// Floor + resolution is good
		return true;
	}
	return false;
}

// ***************************************************************************

bool CZoneSymmetrisation::setTileState (const NL3D::CPatchInfo &patch, uint patchId, float snapCell, float weldThreshold, TState &state, const NLMISC::CMatrix &toOriginalSpace, const CTileBank &bank)
{
	// Edges state
	TState edgesState[4] = { Nothing, Nothing, Nothing, Nothing };

	// Vertices position
	sint32 vertPosU[4];
	sint32 vertPosV[4];

	// For each vertices
	uint i;
	for (i=0; i<4; i++)
	{
		// Snap the vertex
		CVector original = toOriginalSpace * patch.Patch.Vertices[i];
		float valueU = original.x;
		float valueV = original.y;

		// Snap on U
		if (snapOnGrid (valueU, snapCell, weldThreshold))
			vertPosU[i] = (sint32)((valueU+0.5f) / snapCell);
		else
			vertPosU[i] = 0x80000000;

		// Snap on V
		if (snapOnGrid (valueV, snapCell, weldThreshold))
			vertPosV[i] = (sint32)((valueV+0.5f) / snapCell);
		else
			vertPosV[i] = 0x80000000;
	}

	// Patch flags
	bool regular = false;
	bool goofy = false;
	bool EdgeSnaped[4] = { false, false, false, false };

	// For each edges
	for (i=0; i<4; i++)
	{
		// Vertex snapped and align on a common axis ?
		if ( ((uint32) vertPosU[i] != 0x80000000) || ((uint32) vertPosV[i] != 0x80000000) )
		{
			// Snapped on U or V ?
			bool snapU = (vertPosU[i] == vertPosU[(i+1)&3]) && ((uint32) vertPosU[i] != 0x80000000);
			bool snapV = (vertPosV[i] == vertPosV[(i+1)&3]) && ((uint32) vertPosV[i] != 0x80000000);

			// If snapped on one, continue
			if (snapU || snapV)
			{
				// If snap on the both, error
				if (snapU && snapV)
					return false;

				// Is this edge Regular or Goofy ?
				if (snapU)
					edgesState[i] = (i&1)?Goofy:Regular;
				else // (snapV)
					edgesState[i] = (i&1)?Regular:Goofy;

				// Flag the patch
				if (edgesState[i] == Regular)
					regular = true;
				else
					goofy = true;

				// Edge snaped
				EdgeSnaped[i] = true;
			}
		}
	}

	// Goofy and regular ? Error
	if (goofy && regular)
		return false;

	// Nothing ?
	if ((!goofy) && (!regular))
		state = Nothing;
	else
	{
		// Not nothing ?
		state = regular?Regular:Goofy;

		// * Set the tiles

		// For each edges
		for (i=0; i<4; i++)
		{
			// Edge snapped ?
			if (EdgeSnaped[i])
			{
				// For each tiles
				uint tileCount = ((i&1)!=0)?patch.OrderS:patch.OrderT;
				sint currentTile;
				sint delta;
				switch (i)
				{
				case 0:
					currentTile = 0;
					delta = patch.OrderS;
					break;
				case 1:
					currentTile = patch.OrderS*(patch.OrderT-1);
					delta = 1;
					break;
				case 2:
					currentTile = patch.OrderS-1;
					delta = patch.OrderS;
					break;
				case 3:
					currentTile = 0;
					delta = 1;
					break;
				default:
					currentTile = 0;
					delta = 1;
					break;
				}
				uint j;
				for (j=0; j<tileCount; j++)
				{
					// Set the border state
					setTileBorderState (patchId, currentTile, state);

					// For each layer
					uint layer;
					for (layer=0; layer<3; layer++)
					{
						// Get the tiles set used here
						uint tile = patch.Tiles[currentTile].Tile[layer];
						if (tile != NL_TILE_ELM_LAYER_EMPTY)
						{
							int tileSet;
							int number;
							CTileBank::TTileType type;
							bank.getTileXRef (tile, tileSet, number, type);

							if ((tileSet < 0) || (tileSet >= bank.getTileSetCount()))
							{
								nlwarning("CZoneSymmetrisation::setTileState : tile %d has an unknown tileSet (%d)", tile, tileSet);
								return false;
							}

							// Set it only if not oriented
							if (!bank.getTileSet (tileSet)->getOriented ())
							{
								// Set the tile state
								setTileState (patchId, currentTile, layer, state);
							}
						}
					}

					// Next tile
					currentTile += delta;
				}
			}
		}
	}

	return true;
}

// ***************************************************************************

bool CZoneSymmetrisation::setOrientedTileState (const NL3D::CPatchInfo &patch, uint patchId, float snapCell, float weldThreshold, TState &state, const NLMISC::CMatrix &toOriginalSpace, const CTileBank &bank)
{
	// Edges state
	TState edgesState[4] = { Nothing, Nothing, Nothing, Nothing };

	// Vertices position
	sint32 vertPosU[4];
	sint32 vertPosV[4];

	// For each vertices
	uint i;
	for (i=0; i<4; i++)
	{
		// Snap the vertex
		CVector original = toOriginalSpace * patch.Patch.Vertices[i];
		float valueU = original.x;
		float valueV = original.y;

		// Snap on U
		if (snapOnGrid (valueU, snapCell, weldThreshold))
			vertPosU[i] = (sint32)((valueU+0.5f) / snapCell);
		else
			vertPosU[i] = 0x80000000;

		// Snap on V
		if (snapOnGrid (valueV, snapCell, weldThreshold))
			vertPosV[i] = (sint32)((valueV+0.5f) / snapCell);
		else
			vertPosV[i] = 0x80000000;
	}

	// Patch flags
	bool regular = false;
	bool goofy = false;
	bool EdgeSnaped[4] = { false, false, false, false };

	// For each edges
	for (i=0; i<4; i++)
	{
		// Vertex snapped and align on a common axis ?
		if ( ((uint32) vertPosU[i] != 0x80000000) || ((uint32) vertPosV[i] != 0x80000000) )
		{
			// Snapped on U or V ?
			bool snapU = (vertPosU[i] == vertPosU[(i+1)&3]) && ((uint32) vertPosU[i] != 0x80000000);
			bool snapV = (vertPosV[i] == vertPosV[(i+1)&3]) && ((uint32) vertPosV[i] != 0x80000000);

			// If snapped on one, continue
			if (snapU || snapV)
			{
				// If snap on the both, error
				if (snapU && snapV)
					return false;

				// Is this edge Regular or Goofy ?
				edgesState[i] = (i&1)?Goofy:Regular;

				// Flag the patch
				if (edgesState[i] == Regular)
					regular = true;
				else
					goofy = true;

				// Edge snaped
				EdgeSnaped[i] = true;
			}
		}
	}

	// * Set the tiles

	// For each edges
	for (i=0; i<4; i++)
	{
		// Edge snapped ?
		if (EdgeSnaped[i])
		{
			// For each tiles
			uint tileCount = ((i&1)!=0)?patch.OrderS:patch.OrderT;
			sint currentTile;
			sint delta;
			switch (i)
			{
			case 0:
				currentTile = 0;
				delta = patch.OrderS;
				break;
			case 1:
				currentTile = patch.OrderS*(patch.OrderT-1);
				delta = 1;
				break;
			case 2:
				currentTile = patch.OrderS-1;
				delta = patch.OrderS;
				break;
			case 3:
				currentTile = 0;
				delta = 1;
				break;
			default:
				currentTile = 0;
				delta = 1;
				break;
			}
			uint j;
			for (j=0; j<tileCount; j++)
			{
				// Set the border state
				setOrientedTileBorderState (patchId, currentTile, edgesState[i]);

				// For each layer
				uint layer;
				for (layer=0; layer<3; layer++)
				{
					// Get the tiles set used here
					uint tile = patch.Tiles[currentTile].Tile[layer];
					if (tile != NL_TILE_ELM_LAYER_EMPTY)
					{
						int tileSet;
						int number;
						CTileBank::TTileType type;

						bank.getTileXRef (tile, tileSet, number, type);
						if ((tileSet < 0) || (tileSet >= bank.getTileSetCount()))
						{
							nlwarning("CZoneSymmetrisation::setOrientedTileState : tile %d has an unknown tileSet (%d)", tile, tileSet);
							return false;
						}

						// Set it only if oriented
						if (bank.getTileSet (tileSet)->getOriented ())
						{
							setTileState (patchId, currentTile, layer, edgesState[i]);
						}
					}
				}

				// Next tile
				currentTile += delta;
			}
		}
	}

	// For each corners
	for (i=0; i<4; i++)
	{
		// Corner snapped ?
		uint next = (i+1)&3;
		if (EdgeSnaped[i] && EdgeSnaped[next])
		{
			// Flag tile as corner
			switch (i)
			{
			case 0:
				setOrientedTileCorner (patchId, patch.OrderS*(patch.OrderT-1), true);
				break;
			case 1:
				setOrientedTileCorner (patchId, patch.OrderS*patch.OrderT-1, true);
				break;
			case 2:
				setOrientedTileCorner (patchId, patch.OrderS-1, true);
				break;
			case 3:
				setOrientedTileCorner (patchId, 0, true);
				break;
			}
		}
	}

	return true;
}

// ***************************************************************************

CVector2f st2uv (sint s, sint t, const CPatchInfo &patch)
{
	return CVector2f ((((float)s)+0.5f)/(float)patch.OrderS, (((float)t)+0.5f)/(float)patch.OrderT);
}

// ***************************************************************************

void uv2st (const CVector2f &in, sint &s, sint &t, const CPatchInfo &patch)
{
	s = (sint)(in.x*(float)patch.OrderS);
	t = (sint)(in.y*(float)patch.OrderT);
}

// ***************************************************************************

class CFillStackNode
{
public:
	CFillStackNode (uint16 patch, uint16 s, uint16 t, uint8 rotate, CZoneSymmetrisation::TState	state) { Patch = patch; S = s; T = t; Edge = 0; Rotate = rotate; State = state; };
	uint16	S;
	uint16	T;
	uint16	Patch;
	uint8	Edge;
	uint8	Rotate;
	CZoneSymmetrisation::TState	State;
};

// ***************************************************************************

bool CZoneSymmetrisation::propagateTileState (uint patch, uint s, uint t, const std::vector<CPatchInfo> &patchInfo, const CTileBank &bank, bool forceRegular)
{
	// For each layer
	uint layer;
	for (layer=0; layer<3; layer++)
	{
		// Get the patch ptr
		const CPatchInfo *currentPatchPtr = &(patchInfo[patch]);

		// Get the tile index
		uint tile = s+t*currentPatchPtr->OrderS;

		// Get the tiles set used here
		uint tileIndex = currentPatchPtr->Tiles[tile].Tile[layer];
		if (tileIndex != NL_TILE_ELM_LAYER_EMPTY)
		{
			// Valid tile number ?
			if (tileIndex >= (uint)bank.getTileCount ())
			{
				nlwarning ("CZoneSymmetrisation::propagateTileState: Invalid tile index");
				return false;
			}

			// Get the tile set used by this layer
			int tileSetToPropagate;
			int number;
			CTileBank::TTileType type;
			bank.getTileXRef (tileIndex, tileSetToPropagate, number, type);

			if ((tileSetToPropagate < 0) || (tileSetToPropagate >= bank.getTileSetCount()))
			{
				nlwarning("CZoneSymmetrisation::propagateTileState: tile %d has an unknown tileSet (%d)", tileIndex, tileSetToPropagate);
			}
			else
			{
				// Oriented ?
				bool oriented = bank.getTileSet (tileSetToPropagate)->getOriented ();

				// If oriented, must not be a corner
				if (!(oriented && getOrientedTileCorner (patch, tile)))
				{
					// Current node
					CFillStackNode currentNode (patch, s, t, currentPatchPtr->Tiles[tile].getTileOrient(layer), getTileState (patch, tile, layer));

					// Propagate non-Nothing tiles
					if ( (!forceRegular && (currentNode.State != Nothing)) || (forceRegular && (currentNode.State == Nothing)) )
					{
						// Force to Regular ?
						if (forceRegular)
						{
							setTileState (patch, tile, layer, Regular);
							currentNode.State = Regular;
						}

						// Fill stack
						vector<CFillStackNode>	stack;
						stack.push_back (currentNode);

						// While people in the stack
						while (!stack.empty ())
						{
							// Pop last element
							currentNode = stack.back ();
							stack.pop_back ();

							do
							{
								// Set current patch pointer
								currentPatchPtr = &(patchInfo[currentNode.Patch]);

								// Get neighbor
								CFillStackNode neighborNode (currentNode.Patch, currentNode.S, currentNode.T, currentNode.Rotate, currentNode.State);
								switch (currentNode.Edge)
								{
								case 0:
									neighborNode.S--;
									break;
								case 1:
									neighborNode.T++;
									break;
								case 2:
									neighborNode.S++;
									break;
								case 3:
									neighborNode.T--;
									break;
								}

								// Is still in patch ?
								if ( (neighborNode.S>=patchInfo[currentNode.Patch].OrderS) || (neighborNode.T>=patchInfo[currentNode.Patch].OrderT) )
								{
									// No, found new patch
									uint position;
									switch (currentNode.Edge)
									{
									case 0:
										position = neighborNode.T;
										break;
									case 1:
										position = neighborNode.S;
										break;
									case 2:
										position = patchInfo[currentNode.Patch].OrderT - neighborNode.T - 1;
										break;
									case 3:
										position = patchInfo[currentNode.Patch].OrderS - neighborNode.S - 1;
										break;
									default:
										position = 0;
										break;
									}

									// Get next patch
									uint patchOut;
									sint sOut;
									sint tOut;
									if (patchInfo[currentNode.Patch].getNeighborTile (currentNode.Patch, currentNode.Edge, position,
										patchOut, sOut, tOut, patchInfo))
									{
										// Should be another patch
										nlassert (patchOut != currentNode.Patch);

										// Get patch id
										neighborNode.Patch = patchOut;

										// Coordinate must be IN the patch
										nlassert (sOut >= 0);
										nlassert (tOut >= 0);
										nlassert (sOut < patchInfo[neighborNode.Patch].OrderS);
										nlassert (tOut < patchInfo[neighborNode.Patch].OrderT);

										// Copy it
										neighborNode.S = sOut;
										neighborNode.T = tOut;

										// Find neighbor
										const CPatchInfo::CBindInfo &neighborBindInfo = patchInfo[currentNode.Patch].BindEdges[currentNode.Edge];
										uint edgePatch;
										for (edgePatch=0; edgePatch<(uint)neighborBindInfo.NPatchs; edgePatch++)
										{
											if (neighborBindInfo.Next[edgePatch] == neighborNode.Patch)
												break;
										}

										// Must find one patch
										nlassert (edgePatch<(uint)neighborBindInfo.NPatchs);

										// Rotation
										neighborNode.Rotate = (currentNode.Rotate + 2 + neighborBindInfo.Edge[edgePatch] - currentNode.Edge) & 3;

										// Toggle the state ?
										if ((neighborNode.Rotate ^ currentNode.Rotate) & 1)
										{
											// Yes
											neighborNode.State = (neighborNode.State == Regular) ? Goofy : Regular;
										}
									}
									else
									{
										// No propagation, continue
										currentNode.Edge++;
										continue;
									}
								}

								// Neighbor patch
								const CPatchInfo *neighborPatchPtr = &(patchInfo[neighborNode.Patch]);

								// Get the tile index
								uint neighborTile = neighborNode.S+neighborNode.T*neighborPatchPtr->OrderS;

								// Look for the same tile set in the new tile
								uint neighborLayer;
								for (neighborLayer=0; neighborLayer<3; neighborLayer++)
								{
									// Get the tile index
									uint neighborTileIndex = neighborPatchPtr->Tiles[neighborTile].Tile[neighborLayer];

									if (neighborTileIndex != NL_TILE_ELM_LAYER_EMPTY)
									{
										// Valid tile number ?
										if (neighborTileIndex >= (uint)bank.getTileCount ())
										{
											nlwarning ("CZoneSymmetrisation::propagateTileState: Invalid tile index");
											return false;
										}

										// Get tileset
										int neighborTileSet;
										int neighborNumber;
										CTileBank::TTileType neighborType;
										bank.getTileXRef (neighborTileIndex, neighborTileSet, neighborNumber, neighborType);

										// Same tileset ? Stop!
										if (	(neighborTileSet == tileSetToPropagate) &&
												(neighborNode.Rotate == neighborPatchPtr->Tiles[neighborTile].getTileOrient(neighborLayer)) )
											break;
									}
								}

								// Found ?
								if (neighborLayer<3)
								{
									// If oriented, must not be a corner
									if (!(oriented && getOrientedTileCorner (neighborNode.Patch, neighborTile)))
									{
										// Propagate in the new node ?
										TState neighborState = getTileState (neighborNode.Patch, neighborTile, neighborLayer);
										if (neighborState == Nothing)
										{
											// Set the state
											setTileState (neighborNode.Patch, neighborTile, neighborLayer, neighborNode.State);

											// Stack current node if some neighbor left to visit
											if (currentNode.Edge < 3)
											{
												currentNode.Edge++;
												stack.push_back (currentNode);
											}

											// Continue with the new node
											currentNode = neighborNode;
										}
										else if (neighborState != neighborNode.State)
										{
											// Error, same tile but not same state
											// nlwarning ("CZoneSymmetrisation::propagateTileState: error, find same iso surfaces with different state.");

											// No propagation, continue
											currentNode.Edge++;
										}
										else
										{
											// No propagation, continue
											currentNode.Edge++;
										}
									}
									else
									{
										// No propagation, continue
										currentNode.Edge++;
									}
								}
								else
									// No propagation, continue
									currentNode.Edge++;
							}
							while (currentNode.Edge<4);
						}
					}
				}
			}
		}
	}

	return true;
}

// ***************************************************************************

} // NL3D

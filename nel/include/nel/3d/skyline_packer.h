// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
// Copyright (C) 2026  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_SKYLINE_PACKER_H
#define NL_SKYLINE_PACKER_H

#include "nel/misc/types_nl.h"

#include <vector>

namespace NL3D {

/**
 * Places rectangles in one area, each as low as it goes.
 *
 * The packer keeps the outline of what it has placed, seen from above: a row of steps, each with
 * the height the area is filled to over its width. A rectangle goes on the steps where its top
 * comes lowest, and of these on the leftmost. So a short rectangle that follows a tall one leaves
 * a step, and a later rectangle that fits on the step goes there instead of above the tall one.
 *
 * A rectangle that is wider than a step lies over the lower steps beside it, and the room under it
 * is not in the outline any more. The packer keeps that room as gaps, and a rectangle that fits in
 * a gap goes there before it goes on the outline.
 */
class CSkylinePacker
{
public:
	CSkylinePacker();

	/// Empties the area and gives it its size
	void reset(uint width, uint height);

	/** Places a rectangle.
	 * \return false when the rectangle finds no room or has no area; nothing is placed then */
	bool place(uint width, uint height, uint &x, uint &y);

	uint getWidth() const { return m_Width; }
	uint getHeight() const { return m_Height; }

private:
	// One step of the outline: the area is filled up to Y from X over Width
	struct CStep
	{
		uint X, Y, Width;
	};

	// Free room under the outline
	struct CGap
	{
		uint X, Y, Width, Height;
	};

	// The height a rectangle of this width stands at with its left side on the step, if it has the width for it
	bool fitOnStep(uint step, uint width, uint &y) const;
	// Places a rectangle in the gap that it fills best, if it fits in one
	bool placeInGap(uint width, uint height, uint &x, uint &y);
	void addGap(uint x, uint y, uint width, uint height);

	std::vector<CStep> m_Steps;
	std::vector<CGap> m_Gaps;
	uint m_Width;
	uint m_Height;
};

/// A rectangle of CRectBinPacker
struct CPackRect
{
	/// The size asked for; afterwards the size given, which is smaller when there was no room for the size asked for
	uint Width, Height;
	/// Where the rectangle is: the bin, and the corner in it
	uint Bin, X, Y;
	/// False when the rectangle has no room at its smallest size either: Bin, X and Y are 0 then
	bool Placed;

	CPackRect()
	    : Width(0)
	    , Height(0)
	    , Bin(0)
	    , X(0)
	    , Y(0)
	    , Placed(false)
	{
	}
};

/**
 * Places rectangles in bins of one size, with CSkylinePacker.
 *
 * The rectangles are placed the tallest first, which leaves the least gaps between rectangles of
 * many heights. Where that has no room for all of them they are placed the widest first: a wide
 * rectangle that follows a taller, narrow one stands on that one, and the room beside the tall one
 * is a gap that takes nothing wider than itself.
 *
 * The rectangles come in order of priority. Each gets its place in the first bin that has room, and
 * a new bin is opened when none has, up to \c maxBins. Once the bins are full the rectangles give
 * way by priority: a rectangle gets the largest size at which it and the ones before it have room,
 * these at the sizes they got, where a smaller size is half of both sides, not below
 * \c minSize (a side that is shorter than \c minSize stays as it is). A rectangle larger than a bin is
 * cut to the bin first.
 *
 * A packer keeps its working memory from call to call, and so does the list it is given: a packer
 * that places a list in every frame allocates nothing once its lists have grown, which is when
 * the largest list has been placed both with room for all and without.
 */
class CRectBinPacker
{
public:
	/**
	 * \param rects the rectangles, by priority, see CPackRect
	 * \param maxBins the number of bins there may be, or negative for any number
	 * \return the number of bins in use
	 */
	uint pack(std::vector<CPackRect> &rects, uint binWidth, uint binHeight, sint maxBins, uint minSize);

private:
	// A rectangle of the list at the size it is tried at
	struct CSizedRect
	{
		uint Index;
		uint Width, Height;
	};

	// The orders the rectangles are placed in: the tallest first, of these the widest, of these by priority,
	static bool tallestFirst(const CSizedRect &a, const CSizedRect &b);
	// and the widest first, of these the tallest, of these by priority
	static bool widestFirst(const CSizedRect &a, const CSizedRect &b);

	// True when the rectangles are more than the bins hold: they have no room then, in whatever order
	static bool exceedRoom(const std::vector<CSizedRect> &sized, uint binWidth, uint binHeight, sint maxBins);

	// Places the rectangles of m_Order in that order, each in the first bin that has room for it.
	// Returns false when one has no room; what is in places means nothing then.
	bool packInOrder(uint binWidth, uint binHeight, sint maxBins, std::vector<CPackRect> &places, uint &numBins);
	// Places the rectangles at the sizes given: the tallest first, and where that has no room, the widest first.
	// Returns false when neither has room for all; what is in places means nothing then.
	bool packSized(const std::vector<CSizedRect> &sized, uint binWidth, uint binHeight, sint maxBins,
	    std::vector<CPackRect> &places, uint &numBins);

	std::vector<CSkylinePacker> m_Bins;
	std::vector<CSizedRect> m_Sized; // all rectangles, at the sizes asked for
	std::vector<CSizedRect> m_Kept; // the ones that have room, at the sizes they have it at
	std::vector<CSizedRect> m_Order; // the ones of an attempt, in its order
	std::vector<CPackRect> m_Attempt;
	std::vector<CPackRect> m_Placed;
};

/// CRectBinPacker::pack with a packer of its own, for a list that is placed once
uint packRectsByPriority(std::vector<CPackRect> &rects, uint binWidth, uint binHeight, sint maxBins, uint minSize);

} // NL3D

#endif // NL_SKYLINE_PACKER_H

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

#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include <nel/3d/skyline_packer.h>

namespace {

NL3D::CPackRect rect(uint width, uint height)
{
	NL3D::CPackRect r;
	r.Width = width;
	r.Height = height;
	return r;
}

bool overlap(const NL3D::CPackRect &a, const NL3D::CPackRect &b)
{
	return a.Bin == b.Bin
	    && a.X < b.X + b.Width && b.X < a.X + a.Width
	    && a.Y < b.Y + b.Height && b.Y < a.Y + a.Height;
}

// The rectangles that are placed lie inside their bins and beside each other
void expectSound(const std::vector<NL3D::CPackRect> &rects, uint binWidth, uint binHeight, uint numBins)
{
	for (size_t i = 0; i < rects.size(); ++i)
	{
		if (!rects[i].Placed)
			continue;
		EXPECT_LT(rects[i].Bin, numBins) << "rect " << i;
		EXPECT_LE(rects[i].X + rects[i].Width, binWidth) << "rect " << i;
		EXPECT_LE(rects[i].Y + rects[i].Height, binHeight) << "rect " << i;
		for (size_t j = i + 1; j < rects.size(); ++j)
		{
			if (rects[j].Placed)
			{
				EXPECT_FALSE(overlap(rects[i], rects[j])) << "rects " << i << " and " << j;
			}
		}
	}
}

// A sequence of numbers that is the same on every run
class CSequence
{
public:
	CSequence(uint32 seed)
	    : m_State(seed)
	{
	}
	uint next(uint count)
	{
		m_State = m_State * 1664525u + 1013904223u;
		return (uint)((m_State >> 8) % count);
	}

private:
	uint32 m_State;
};

} // namespace

TEST(CUT3DSkylinePacker, emptyAreaTakesNothing)
{
	NL3D::CSkylinePacker packer;
	uint x, y;
	EXPECT_FALSE(packer.place(1, 1, x, y));
	packer.reset(0, 64);
	EXPECT_FALSE(packer.place(1, 1, x, y));
}

TEST(CUT3DSkylinePacker, refusesWhatDoesNotFit)
{
	NL3D::CSkylinePacker packer;
	packer.reset(64, 32);
	uint x, y;
	EXPECT_FALSE(packer.place(0, 8, x, y));
	EXPECT_FALSE(packer.place(8, 0, x, y));
	EXPECT_FALSE(packer.place(65, 8, x, y));
	EXPECT_FALSE(packer.place(8, 33, x, y));
	// a refusal leaves the area as it was
	ASSERT_TRUE(packer.place(64, 32, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(0u, y);
	EXPECT_FALSE(packer.place(1, 1, x, y));
}

TEST(CUT3DSkylinePacker, fillsARowFromTheLeft)
{
	NL3D::CSkylinePacker packer;
	packer.reset(96, 64);
	uint x, y;
	for (uint i = 0; i < 3; ++i)
	{
		ASSERT_TRUE(packer.place(32, 32, x, y));
		EXPECT_EQ(i * 32, x);
		EXPECT_EQ(0u, y);
	}
	ASSERT_TRUE(packer.place(32, 32, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(32u, y);
}

// A row that begins with a tall rectangle: what follows the row goes on the short ones, beside the tall one
TEST(CUT3DSkylinePacker, usesTheStepBesideATallRectangle)
{
	NL3D::CSkylinePacker packer;
	packer.reset(128, 128);
	uint x, y;
	ASSERT_TRUE(packer.place(32, 128, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(0u, y);
	for (uint i = 0; i < 3; ++i)
	{
		ASSERT_TRUE(packer.place(32, 32, x, y));
		EXPECT_EQ(32 + i * 32, x);
		EXPECT_EQ(0u, y);
	}
	// the row is full: the next ones stand on the short ones
	for (uint row = 1; row < 4; ++row)
	{
		for (uint i = 0; i < 3; ++i)
		{
			ASSERT_TRUE(packer.place(32, 32, x, y));
			EXPECT_EQ(32 + i * 32, x);
			EXPECT_EQ(row * 32, y);
		}
	}
	EXPECT_FALSE(packer.place(32, 32, x, y));
}

// A tall narrow rectangle between short ones: the short ones that follow do not start above it
TEST(CUT3DSkylinePacker, tallRectangleInARowOfShortOnes)
{
	NL3D::CSkylinePacker packer;
	packer.reset(160, 128);
	uint x, y;
	ASSERT_TRUE(packer.place(64, 32, x, y));
	ASSERT_TRUE(packer.place(32, 128, x, y));
	EXPECT_EQ(64u, x);
	EXPECT_EQ(0u, y);
	ASSERT_TRUE(packer.place(64, 32, x, y));
	EXPECT_EQ(96u, x);
	EXPECT_EQ(0u, y);
	// both sides of the tall one fill up
	ASSERT_TRUE(packer.place(64, 32, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(32u, y);
	ASSERT_TRUE(packer.place(64, 32, x, y));
	EXPECT_EQ(96u, x);
	EXPECT_EQ(32u, y);
}

// A rectangle that lies over a lower step stands on the higher one
TEST(CUT3DSkylinePacker, standsOnTheHighestStepUnderIt)
{
	NL3D::CSkylinePacker packer;
	packer.reset(96, 96);
	uint x, y;
	ASSERT_TRUE(packer.place(32, 48, x, y));
	ASSERT_TRUE(packer.place(32, 16, x, y));
	EXPECT_EQ(32u, x);
	// wider than the low steps to the right of the first: it lies over the second and the floor
	ASSERT_TRUE(packer.place(64, 32, x, y));
	EXPECT_EQ(32u, x);
	EXPECT_EQ(16u, y);
	// the outline is one step now
	ASSERT_TRUE(packer.place(96, 48, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(48u, y);
	// the room under the third, over the floor, is all that is left
	EXPECT_FALSE(packer.place(33, 16, x, y));
	EXPECT_FALSE(packer.place(32, 17, x, y));
	ASSERT_TRUE(packer.place(32, 16, x, y));
	EXPECT_EQ(64u, x);
	EXPECT_EQ(0u, y);
	EXPECT_FALSE(packer.place(1, 1, x, y));
}

// A rectangle as wide as the area on a narrow tall one: the room beside the tall one, under the
// wide one, is still to be had
TEST(CUT3DSkylinePacker, keepsTheRoomUnderAWideRectangle)
{
	NL3D::CSkylinePacker packer;
	packer.reset(512, 512);
	uint x, y;
	ASSERT_TRUE(packer.place(256, 256, x, y));
	ASSERT_TRUE(packer.place(512, 128, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(256u, y);
	// the room is 256 x 256 at 256, 0: a rectangle goes in its corner, the next ones beside and over it
	ASSERT_TRUE(packer.place(128, 128, x, y));
	EXPECT_EQ(256u, x);
	EXPECT_EQ(0u, y);
	ASSERT_TRUE(packer.place(128, 128, x, y));
	EXPECT_EQ(384u, x);
	EXPECT_EQ(0u, y);
	ASSERT_TRUE(packer.place(256, 128, x, y));
	EXPECT_EQ(256u, x);
	EXPECT_EQ(128u, y);
	// what does not fit under the wide one goes on it
	ASSERT_TRUE(packer.place(512, 128, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(384u, y);
	EXPECT_FALSE(packer.place(1, 1, x, y));
}

TEST(CUT3DSkylinePacker, resetEmptiesTheArea)
{
	NL3D::CSkylinePacker packer;
	packer.reset(32, 32);
	uint x, y;
	ASSERT_TRUE(packer.place(32, 32, x, y));
	EXPECT_FALSE(packer.place(8, 8, x, y));
	packer.reset(16, 48);
	EXPECT_EQ(16u, packer.getWidth());
	EXPECT_EQ(48u, packer.getHeight());
	ASSERT_TRUE(packer.place(16, 48, x, y));
	EXPECT_EQ(0u, x);
	EXPECT_EQ(0u, y);
}

TEST(CUT3DSkylinePacker, placedRectanglesNeverOverlap)
{
	CSequence sequence(20261125);
	for (uint round = 0; round < 200; ++round)
	{
		const uint width = 32 + sequence.next(480), height = 32 + sequence.next(480);
		NL3D::CSkylinePacker packer;
		packer.reset(width, height);
		std::vector<NL3D::CPackRect> placed;
		uint area = 0;
		for (uint i = 0; i < 64; ++i)
		{
			const uint rectWidth = 1 + sequence.next(width / 2);
			const uint rectHeight = 1 + sequence.next(height / 2);
			NL3D::CPackRect r = rect(rectWidth, rectHeight);
			if (packer.place(r.Width, r.Height, r.X, r.Y))
			{
				r.Placed = true;
				placed.push_back(r);
				area += r.Width * r.Height;
			}
		}
		EXPECT_LE(area, width * height);
		expectSound(placed, width, height, 1);
	}
}

// ---------------------------------------------------------------------------

TEST(CUT3DSkylinePacker, byPriorityKeepsTheSizesThatFit)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(64, 32));
	rects.push_back(rect(32, 128));
	rects.push_back(rect(64, 32));
	rects.push_back(rect(128, 64));
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 256, 128, 1, 32));
	ASSERT_EQ(4u, rects.size());
	const uint widths[4] = { 64, 32, 64, 128 };
	const uint heights[4] = { 32, 128, 32, 64 };
	for (uint i = 0; i < 4; ++i)
	{
		EXPECT_TRUE(rects[i].Placed);
		EXPECT_EQ(widths[i], rects[i].Width);
		EXPECT_EQ(heights[i], rects[i].Height);
	}
	expectSound(rects, 256, 128, 1);
	// the tallest stands first, whatever its priority
	EXPECT_EQ(0u, rects[1].X);
	EXPECT_EQ(0u, rects[1].Y);
}

// Pool reflections of mixed heights that fill one texture only when no gap is left: 2 of 64 x 128,
// 4 of 64 x 64, 8 of 32 x 32, in the order that leaves the most gaps to a packer that fills rows
TEST(CUT3DSkylinePacker, byPriorityLeavesNoGapBetweenMixedHeights)
{
	std::vector<NL3D::CPackRect> rects;
	for (uint i = 0; i < 2; ++i)
	{
		rects.push_back(rect(32, 32));
		rects.push_back(rect(64, 64));
		rects.push_back(rect(32, 32));
		rects.push_back(rect(64, 128));
		rects.push_back(rect(32, 32));
		rects.push_back(rect(64, 64));
		rects.push_back(rect(32, 32));
	}
	// 2 * 8192 + 4 * 4096 + 8 * 1024 = 40960 = 320 * 128
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 320, 128, 1, 32));
	uint area = 0;
	for (size_t i = 0; i < rects.size(); ++i)
	{
		EXPECT_TRUE(rects[i].Placed) << "rect " << i;
		area += rects[i].Width * rects[i].Height;
	}
	EXPECT_EQ(320u * 128u, area);
	expectSound(rects, 320, 128, 1);
}

// Two rectangles as wide as the bin and a taller narrow one: the wide ones stand on the narrow one,
// and the last goes in the room beside the narrow one, under them. The bin is full only that way.
TEST(CUT3DSkylinePacker, byPriorityUsesTheRoomUnderWideRectangles)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(512, 128));
	rects.push_back(rect(512, 128));
	rects.push_back(rect(256, 256));
	rects.push_back(rect(256, 128));
	const std::vector<NL3D::CPackRect> asked = rects;
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 512, 512, 1, 32));
	expectSound(rects, 512, 512, 1);
	for (size_t i = 0; i < rects.size(); ++i)
	{
		EXPECT_TRUE(rects[i].Placed) << "rect " << i;
		EXPECT_EQ(asked[i].Width, rects[i].Width) << "rect " << i;
		EXPECT_EQ(asked[i].Height, rects[i].Height) << "rect " << i;
	}
	EXPECT_EQ(256u, rects[3].X);
	EXPECT_EQ(0u, rects[3].Y);
}

// The tallest first: the widest stands on the tallest, and the room beside the tallest is too
// narrow for the second. The widest first has room for all three.
TEST(CUT3DSkylinePacker, byPriorityPlacesTheWidestFirstWhereTheTallestFirstHasNoRoom)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(112, 48));
	rects.push_back(rect(96, 32));
	rects.push_back(rect(32, 64));
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 128, 128, 1, 32));
	expectSound(rects, 128, 128, 1);
	EXPECT_TRUE(rects[0].Placed);
	EXPECT_EQ(112u, rects[0].Width);
	EXPECT_EQ(48u, rects[0].Height);
	EXPECT_EQ(0u, rects[0].X);
	EXPECT_EQ(0u, rects[0].Y);
	EXPECT_TRUE(rects[1].Placed);
	EXPECT_EQ(96u, rects[1].Width);
	EXPECT_EQ(32u, rects[1].Height);
	EXPECT_EQ(0u, rects[1].X);
	EXPECT_EQ(48u, rects[1].Y);
	EXPECT_TRUE(rects[2].Placed);
	EXPECT_EQ(32u, rects[2].Width);
	EXPECT_EQ(64u, rects[2].Height);
	EXPECT_EQ(96u, rects[2].X);
	EXPECT_EQ(48u, rects[2].Y);
}

TEST(CUT3DSkylinePacker, byPriorityOpensBinsWhileItMay)
{
	std::vector<NL3D::CPackRect> rects(5, rect(64, 64));
	EXPECT_EQ(5u, NL3D::packRectsByPriority(rects, 64, 64, -1, 32));
	for (uint i = 0; i < 5; ++i)
	{
		EXPECT_TRUE(rects[i].Placed);
		EXPECT_EQ(i, rects[i].Bin);
		EXPECT_EQ(64u, rects[i].Width);
	}

	// two bins: the first two have their sizes, the later ones share what the second leaves
	rects.clear();
	rects.push_back(rect(64, 64));
	rects.push_back(rect(64, 32));
	rects.push_back(rect(64, 64));
	rects.push_back(rect(64, 64));
	EXPECT_EQ(2u, NL3D::packRectsByPriority(rects, 64, 64, 2, 32));
	expectSound(rects, 64, 64, 2);
	EXPECT_TRUE(rects[0].Placed);
	EXPECT_EQ(64u, rects[0].Width);
	EXPECT_EQ(64u, rects[0].Height);
	EXPECT_EQ(0u, rects[0].Bin);
	EXPECT_TRUE(rects[1].Placed);
	EXPECT_EQ(64u, rects[1].Width);
	EXPECT_EQ(32u, rects[1].Height);
	EXPECT_EQ(1u, rects[1].Bin);
	for (uint i = 2; i < 4; ++i)
	{
		EXPECT_TRUE(rects[i].Placed);
		EXPECT_EQ(32u, rects[i].Width);
		EXPECT_EQ(32u, rects[i].Height);
		EXPECT_EQ(1u, rects[i].Bin);
	}
}

// With the bins full the earlier rectangles keep their sizes and the later ones give way
TEST(CUT3DSkylinePacker, byPriorityShrinksTheLaterOnes)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(128, 64));
	rects.push_back(rect(128, 128)); // taller than the first, and after it
	rects.push_back(rect(128, 128));
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 256, 128, 1, 32));
	expectSound(rects, 256, 128, 1);
	EXPECT_TRUE(rects[0].Placed);
	EXPECT_EQ(128u, rects[0].Width);
	EXPECT_EQ(64u, rects[0].Height);
	EXPECT_TRUE(rects[1].Placed);
	EXPECT_EQ(128u, rects[1].Width);
	EXPECT_EQ(128u, rects[1].Height);
	EXPECT_TRUE(rects[2].Placed);
	EXPECT_EQ(64u, rects[2].Width);
	EXPECT_EQ(64u, rects[2].Height);
}

TEST(CUT3DSkylinePacker, byPriorityLeavesOutWhatHasNoRoom)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(256, 128));
	rects.push_back(rect(128, 64));
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 256, 128, 1, 32));
	EXPECT_TRUE(rects[0].Placed);
	EXPECT_EQ(256u, rects[0].Width);
	EXPECT_EQ(128u, rects[0].Height);
	EXPECT_FALSE(rects[1].Placed);
	EXPECT_EQ(32u, rects[1].Width);
	EXPECT_EQ(32u, rects[1].Height);
	EXPECT_EQ(0u, rects[1].Bin);
	EXPECT_EQ(0u, rects[1].X);
	EXPECT_EQ(0u, rects[1].Y);
}

// A side below the smallest size is not made longer, and a rectangle larger than a bin is cut to it
TEST(CUT3DSkylinePacker, byPriorityKeepsShortSidesAndCutsToTheBin)
{
	std::vector<NL3D::CPackRect> rects;
	rects.push_back(rect(240, 128));
	rects.push_back(rect(16, 256));
	rects.push_back(rect(16, 128));
	EXPECT_EQ(1u, NL3D::packRectsByPriority(rects, 256, 128, 1, 32));
	expectSound(rects, 256, 128, 1);
	EXPECT_TRUE(rects[1].Placed);
	EXPECT_EQ(16u, rects[1].Width);
	EXPECT_EQ(128u, rects[1].Height);
	// no column is left for the third: it gives way in height only, and finds no room
	EXPECT_FALSE(rects[2].Placed);
	EXPECT_EQ(16u, rects[2].Width);
	EXPECT_EQ(32u, rects[2].Height);
}

TEST(CUT3DSkylinePacker, byPriorityWithoutBins)
{
	std::vector<NL3D::CPackRect> rects(2, rect(8, 8));
	EXPECT_EQ(0u, NL3D::packRectsByPriority(rects, 64, 64, 0, 32));
	EXPECT_FALSE(rects[0].Placed);
	EXPECT_FALSE(rects[1].Placed);

	rects.clear();
	EXPECT_EQ(0u, NL3D::packRectsByPriority(rects, 64, 64, -1, 32));
}

TEST(CUT3DSkylinePacker, byPriorityNeverOverlaps)
{
	CSequence sequence(8128);
	const uint sides[5] = { 16, 32, 64, 128, 256 };
	for (uint round = 0; round < 300; ++round)
	{
		const uint binWidth = sides[2 + sequence.next(3)] * 2, binHeight = sides[2 + sequence.next(3)];
		const sint maxBins = (sint)sequence.next(4) - 1; // -1 is any number
		std::vector<NL3D::CPackRect> rects;
		const uint count = 1 + sequence.next(24);
		for (uint i = 0; i < count; ++i)
		{
			const uint rectWidth = sides[sequence.next(5)];
			const uint rectHeight = sides[sequence.next(5)];
			rects.push_back(rect(rectWidth, rectHeight));
		}
		const std::vector<NL3D::CPackRect> asked = rects;
		const uint numBins = NL3D::packRectsByPriority(rects, binWidth, binHeight, maxBins, 32);
		ASSERT_EQ(asked.size(), rects.size());
		if (maxBins >= 0)
		{
			EXPECT_LE(numBins, (uint)maxBins);
		}
		expectSound(rects, binWidth, binHeight, numBins);
		for (size_t i = 0; i < rects.size(); ++i)
		{
			EXPECT_LE(rects[i].Width, asked[i].Width);
			EXPECT_LE(rects[i].Height, asked[i].Height);
			EXPECT_GT(rects[i].Width, 0u);
			EXPECT_GT(rects[i].Height, 0u);
			if (!rects[i].Placed)
			{
				// left out at the smallest size, with no place
				EXPECT_EQ(std::min(asked[i].Width, 32u), rects[i].Width);
				EXPECT_EQ(std::min(asked[i].Height, 32u), rects[i].Height);
				EXPECT_EQ(0u, rects[i].Bin);
				EXPECT_EQ(0u, rects[i].X);
				EXPECT_EQ(0u, rects[i].Y);
			}
			if (maxBins < 0)
			{
				// with any number of bins nothing gives way
				EXPECT_TRUE(rects[i].Placed);
				EXPECT_EQ(std::min(asked[i].Width, binWidth), rects[i].Width);
				EXPECT_EQ(std::min(asked[i].Height, binHeight), rects[i].Height);
			}
		}
	}
}

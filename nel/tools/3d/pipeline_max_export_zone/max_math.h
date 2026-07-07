/**
 * \file max_math.h
 * \brief Headless replication of the 3ds Max float matrix/quat math the reference ig exporter
 * runs through: Matrix3 (row-vector, 4x3), the PRS transform composition, and the Graphics
 * Gems IV polar/affine matrix decomposition (Shoemake) that CExportNel::decompMatrix wraps.
 * All arithmetic in float, mimicking the SDK operation order — see pipeline_max_design.md for
 * the bit-exactness contract (T3-epsilon class where x87 intermediates differ).
 * \author Jan Boon (Kaetemi)
 * \author Claude Fable 5
 */

/*
 * Copyright (C) 2026  by authors
 *
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef PIPELINE_MAX_EXPORT_ZONE_MAX_MATH_H
#define PIPELINE_MAX_EXPORT_ZONE_MAX_MATH_H

#include <nel/misc/types_nl.h>

namespace MAXMATH {

// Max-convention quaternion value (the bits stored in the .max controller chunks).
struct QuatM
{
	float x, y, z, w;
};

// Max Point3
struct Point3M
{
	float x, y, z;
};

// Max Matrix3: 4 rows of 3 (rows 0..2 = basis vectors, row 3 = translation), row-vector
// convention (v' = v * M).
struct Matrix3M
{
	float m[4][3];

	static Matrix3M identity();
};

Matrix3M operator*(const Matrix3M &a, const Matrix3M &b);
Matrix3M inverseM3(const Matrix3M &m);
Matrix3M quatToMatrix3(const QuatM &q); // Max Quat::MakeMatrix (rotation rows, zero translation)

// The Max ScaleValue (per-axis scale s + axis-system quat q).
struct ScaleValueM
{
	Point3M s;
	QuatM q;
};

// Compose the PRS local matrix exactly like the Max PRS controller does on an identity parent:
// M = S * R * T for row vectors (scale applied first).
Matrix3M composePRS(const Point3M &pos, const QuatM &rot, const ScaleValueM &scale);

// Max Point3 * Matrix3 (row-vector point transform: v' = v * M, translation applied).
Point3M transformPoint(const Point3M &v, const Matrix3M &m);

// ---------------------------------------------------------------------------------------------
// Graphics Gems IV affine matrix decomposition (Ken Shoemake, "Polar Matrix Decomposition").
// Operates on the Max row-vector Matrix3M with the same input adaptation the SDK version uses.

struct AffinePartsM
{
	Point3M t;   // translation
	QuatM q;     // essential rotation
	QuatM u;     // stretch rotation
	Point3M k;   // stretch factors
	float f;     // sign of determinant (+1/-1)
};

void decompAffine(const Matrix3M &a, AffinePartsM &parts);

} /* namespace MAXMATH */

#endif /* PIPELINE_MAX_EXPORT_ZONE_MAX_MATH_H */

/* end of file */

/**
 * \file hex_blob.cpp
 * \brief See hex_blob.h.
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

#include <nel/misc/types_nl.h>
#include "hex_blob.h"

#include <cstdio>

namespace NLGLTF {

std::string bytesToHex(const uint8 *data, size_t len)
{
	std::string hex;
	hex.reserve(len * 2);
	char buf[4];
	for (size_t k = 0; k < len; ++k)
	{
		snprintf(buf, sizeof(buf), "%02x", data[k]);
		hex += buf;
	}
	return hex;
}

std::string bytesToHex(const std::vector<uint8> &bytes)
{
	return bytesToHex(bytes.empty() ? NULL : &bytes[0], bytes.size());
}

bool hexToBytes(const std::string &hex, std::vector<uint8> &out)
{
	out.clear();
	if (hex.size() % 2)
		return false;
	out.resize(hex.size() / 2);
	for (size_t k = 0; k < out.size(); ++k)
	{
		unsigned x = 0;
		for (int h = 0; h < 2; ++h)
		{
			char c = hex[k * 2 + h];
			x <<= 4;
			if (c >= '0' && c <= '9') x |= (unsigned)(c - '0');
			else if (c >= 'a' && c <= 'f') x |= (unsigned)(c - 'a' + 10);
			else if (c >= 'A' && c <= 'F') x |= (unsigned)(c - 'A' + 10);
			else return false;
		}
		out[k] = (uint8)x;
	}
	return true;
}

} /* namespace NLGLTF */

/* end of file */

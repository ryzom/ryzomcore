/**
 * \file storage_ole.h
 * \brief CStorageOleIn / CStorageOleOut
 * \date 2026-07-07
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 * Access to the OLE2 / MS-CFB (Compound File Binary Format) container that wraps a .max file.
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

#ifndef PIPELINE_STORAGE_OLE_H
#define PIPELINE_STORAGE_OLE_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>

// NeL includes

// Project includes

namespace PIPELINE {
namespace MAX {

/**
 * \brief CStorageOleIn
 * \date 2026-07-07
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 *
 * Read side of the OLE2 compound-document container. A .max file is an MS-CFB file with a flat set
 * of root-level streams (Scene, DllDirectory, ClassDirectory3, ClassData, Config, VideoPostQueue,
 * the two \\05*Information blobs) plus a root-storage CLSID. This exposes those streams by name as
 * raw byte vectors — the CStorageContainer layer (via CStorageStream) does everything above the
 * container.
 *
 * There are two interchangeable backends selected at compile time by NL_PIPELINE_NATIVE_OLE (see
 * pipeline_max_design.md): the default self-contained native CFB parser (cross-platform,
 * deterministic, no third-party dependency), and a libgsf-backed one kept for cross-validation.
 * Both return byte-for-byte identical stream contents; only the physical sector layout of a
 * rewritten file differs (which is irrelevant — the roundtrip unit is the stream, not the file).
 */
class CStorageOleIn
{
public:
	CStorageOleIn();
	~CStorageOleIn();

	/// Open and parse the compound file. Returns false if the file cannot be read or is not a
	/// valid OLE2/CFB container. Safe to call once per instance.
	bool open(const std::string &path);
	/// True after a successful open().
	bool isOpen() const;
	/// Release resources (also done by the destructor).
	void close();

	/// Copy the 16-byte root-storage CLSID (the .max "class id") into \a classId. Returns false
	/// if no CLSID is present (all-zero CLSIDs are reported as present, matching the source).
	bool getClassId(uint8 classId[16]) const;

	/// True if a root-level stream with this exact name exists.
	bool hasStream(const std::string &name) const;
	/// Copy a root-level stream's full bytes into \a out. Returns false if the stream is absent.
	bool readStream(const std::string &name, std::vector<uint8> &out) const;
	/// Root-level stream names, in the container's directory order.
	const std::vector<std::string> &streamNames() const;

private:
	CStorageOleIn(const CStorageOleIn &);
	CStorageOleIn &operator=(const CStorageOleIn &);

	void *m_Impl;

}; /* class CStorageOleIn */

/**
 * \brief CStorageOleOut
 * \date 2026-07-07
 * \author Jan Boon (Kaetemi)
 * \author Claude Opus 4.8 (1M context)
 *
 * Write side: accumulate root-level streams (in add order) plus the root CLSID, then assemble the
 * whole MS-CFB container and write it to disk. Deterministic: the same set of streams and CLSID
 * always produces the same file bytes.
 */
class CStorageOleOut
{
public:
	CStorageOleOut();
	~CStorageOleOut();

	/// Set the 16-byte root-storage CLSID written to the file. Defaults to all-zero.
	void setClassId(const uint8 classId[16]);

	/// Add a root-level stream. Names must be unique; call order is preserved for the mini/big
	/// stream data layout (the directory itself is emitted in CFB tree order regardless).
	void addStream(const std::string &name, const std::vector<uint8> &data);
	/// Move-in variant: \a data is swapped empty to avoid a copy.
	void addStreamSwap(const std::string &name, std::vector<uint8> &data);

	/// Assemble and write the container to \a path. Returns false on any I/O error.
	bool write(const std::string &path);

private:
	CStorageOleOut(const CStorageOleOut &);
	CStorageOleOut &operator=(const CStorageOleOut &);

	void *m_Impl;

}; /* class CStorageOleOut */

} /* namespace MAX */
} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_STORAGE_OLE_H */

/* end of file */

/**
 * \file max_thumbnail.h
 * \brief OLE SummaryInformation thumbnail R/W for zone_painter browsers + save (ui M5)
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 *
 * Public include contract: NLMISC only (path/file/bitmap types via strings). No
 * patch_eval, SCENELIB, NLGUI. The .cpp opens OLE via pipeline_max CStorageOleIn/Out
 * read-only for extract; write rebuilds only the SummaryInformation stream.
 *
 * Cache layout: CPath::getApplicationDirectory("zone_painter") + "thumbcache/"
 * keyed by source path hash + mtime.
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

#ifndef ZONE_PAINTER_MAX_THUMBNAIL_H
#define ZONE_PAINTER_MAX_THUMBNAIL_H

#include <nel/misc/types_nl.h>

#include <string>
#include <vector>

namespace NLMISC {
class CBitmap;
}

namespace ZPTHUMB {

/// OLE SummaryInformation stream name (leading \\05 / 0x05).
extern const char *kSummaryInformationStream;

/** Absolute thumbcache directory (created on demand). */
std::string thumbCacheDir();

/**
 * Cache path for a .max source: thumbcache/<pathhash>_<mtime>.tga
 * Empty if maxPath is empty.
 */
std::string cachedThumbPath(const std::string &maxPath);

/**
 * Extract PIDSI_THUMBNAIL (property 17, VT_CF) from a .max file into a CBitmap.
 * Handles absence gracefully (no stream / no property / non-DIB → false).
 * Decodes 24bpp bottom-up DIB at minimum (CF_DIB or DIB embedded in CF_METAFILEPICT).
 */
bool extractThumbnailBitmap(const std::string &maxPath, NLMISC::CBitmap &out);

/**
 * Extract raw CLIPDATA bytes of PIDSI_THUMBNAIL (after type/cbSize/ulClipFmt header
 * fields as stored — full property value starting at VT). Used by roundtrip tests.
 * Returns false if missing.
 */
bool extractThumbnailProperty(const std::string &maxPath, std::vector<uint8> &outPropValue);

/**
 * Read SummaryInformation stream bytes from a .max (empty if absent).
 */
bool readSummaryInformationStream(const std::string &maxPath, std::vector<uint8> &out);

/**
 * Ensure a cached .tga exists for maxPath (extract on miss / stale mtime).
 * On success sets outTgaPath to the absolute cache path and returns true.
 * On failure (no thumb) outTgaPath is cleared and returns false.
 */
bool ensureCachedThumbnail(const std::string &maxPath, std::string &outTgaPath);

/**
 * Replace PIDSI_THUMBNAIL in a SummaryInformation stream with a 24bpp DIB thumbnail
 * built from `bmp` (max dimension scaled to maxDim, default 128). Other properties
 * preserved. Returns false on parse failure.
 *
 * When `bmp` is NULL and keepExisting is true: re-encode the stream with the existing
 * thumbnail (roundtrip null test path).
 */
bool rebuildSummaryInformationWithThumbnail(const std::vector<uint8> &inStream,
                                            const NLMISC::CBitmap *bmp,
                                            std::vector<uint8> &outStream,
                                            uint maxDim = 128,
                                            bool keepExisting = false);

/**
 * Encode `bmp` as a 24bpp bottom-up DIB (BITMAPINFOHEADER + pixels, BGR, row-padded).
 * Scales so max(w,h) <= maxDim. Out is the raw DIB (no CF wrapper).
 */
bool encodeDib24(const NLMISC::CBitmap &bmp, std::vector<uint8> &outDib, uint maxDim = 128);

/**
 * Roundtrip test: parse SummaryInformation and re-serialize without changing the
 * thumbnail; returns true if out bytes == in bytes. Fills err on failure/mismatch.
 */
bool thumbRoundtripIdentical(const std::string &maxPath, std::string &err);

/**
 * Build a replacement SummaryInformation stream for maxPath with `bmp` as the
 * new PIDSI_THUMBNAIL (scaled to maxDim). Returns false if the input SI cannot
 * be parsed (or is missing and cannot be synthesized). On success, outStream is
 * the full SI bytes to write into the OLE container.
 *
 * When gateRoundtrip is true, requires that a keep-existing re-encode of the
 * input SI is byte-identical first (warn-and-skip safety for odd Max variants).
 */
bool buildSummaryInformationWithThumbnail(const std::string &maxPath,
                                          const NLMISC::CBitmap &bmp,
                                          std::vector<uint8> &outStream,
                                          uint maxDim = 128,
                                          bool gateRoundtrip = true,
                                          std::string *err = NULL);

/**
 * Build a CF_DIB (clipboard format 8) CLIPDATA property value (VT_CF) from a DIB.
 * Layout: type=71, cbSize, ulClipFmt=-1, cf=8, dib bytes.
 */
void wrapDibAsVtCfProperty(const std::vector<uint8> &dib, std::vector<uint8> &outPropValue);

// ---------------------------------------------------------------------------------------------
// Tileset palette previews (ui M8): 64x64 TGA cache under thumbcache/tileset/

/** Absolute tileset-preview cache directory (created on demand). */
std::string tilesetPreviewCacheDir();

/**
 * Cache path for a tileset preview: thumbcache/tileset/<bankhash>_s<idx>_<season>_<srcmtime>.tga
 * seasonKey is a short tag (e.g. "sp", "su", "auto"); empty becomes "auto".
 */
std::string cachedTilesetPreviewPath(const std::string &bankPath, int setIndex,
                                    const std::string &seasonKey, uint32 sourceMtime);

/**
 * Ensure a 64x64 TGA preview exists for one tileset cell.
 * Loads sourcePath (absolute, already CPath-resolved diffuse) via CBitmap, resamples to
 * sidePx (default 64), writes the cache path. On hit (cache present) reuses without reload.
 * Returns true and sets outTgaPath on success; false leaves outTgaPath empty.
 */
bool ensureTilesetPreview(const std::string &bankPath, int setIndex,
                          const std::string &seasonKey, const std::string &sourcePath,
                          std::string &outTgaPath, uint sidePx = 64);

// ---------------------------------------------------------------------------------------------
// Displacement palette previews (ui M9a/M10a): 64x64 TGA cache under thumbcache/displace/
// Point-upsampled from 32x32 noise maps: min..max stretch + sqrt gamma lift (M10a).

/** Absolute displace-preview cache directory (created on demand). */
std::string displacePreviewCacheDir();

/**
 * Cache path: thumbcache/displace/<bankhash>_m<mapIdx>_<srcmtime>_g1.tga
 * Keyed by bank + global displacement-map index + source mtime + preview-gen version
 * (_g1 = min..max + sqrt gamma; bump suffix when the tone-map changes).
 * Maps are season-invariant.
 */
std::string cachedDisplacePreviewPath(const std::string &bankPath, int mapIndex, uint32 sourceMtime);

/**
 * Ensure a 64x64 grayscale preview for one bank displacement map.
 * Loads sourcePath, stretches luminance min..max to 0..1, applies sqrt gamma lift,
 * point-upsamples to sidePx, writes TGA cache. Returns true and sets outTgaPath on success.
 */
bool ensureDisplacePreview(const std::string &bankPath, int mapIndex,
                           const std::string &sourcePath, std::string &outTgaPath,
                           uint sidePx = 64);

} // namespace ZPTHUMB

#endif // ZONE_PAINTER_MAX_THUMBNAIL_H

/* end of file */

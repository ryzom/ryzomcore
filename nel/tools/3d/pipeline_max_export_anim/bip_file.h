/**
 * \file bip_file.h
 * \brief Character Studio .bip (motion clip) loader for the headless anim exporter.
 *
 * Snowballs (and many era workflows) store biped takes as standalone .bip files rather than
 * keys inside the figure .max. The BIP stream reuses the same track id pairs as the Biped
 * (0x9155) system object (0x012c/d horizontal, …, 0x0149/a pony2 — pipeline_max_design §10c),
 * with a compact header and a slightly different time-record field order.
 *
 * Decoded 2026-07-12 against the Snowballs Media Database .bip takes; feeds SBipAnimKeys for
 * CBipedAnimEval override. See pipeline_max_design §10z-snowballs.
 * \author Jan Boon (Kaetemi)
 * \author Grok 4.5
 */

#ifndef PIPELINE_MAX_EXPORT_ANIM_BIP_FILE_H
#define PIPELINE_MAX_EXPORT_ANIM_BIP_FILE_H

#include "biped_anim.h"

#include <string>

namespace BIPANIM {

/// Load a Character Studio .bip motion file into keytracks. Returns false on I/O or structural
/// failure (err filled). On success every present track has matching Times/Recs; absent tracks
/// stay empty (figure-hold).
bool loadBipFile(const std::string &path, SBipAnimKeys &out, std::string &err);

} /* namespace BIPANIM */

#endif /* PIPELINE_MAX_EXPORT_ANIM_BIP_FILE_H */

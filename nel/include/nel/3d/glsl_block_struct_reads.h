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

#ifndef NL_GLSL_BLOCK_STRUCT_READS_H
#define NL_GLSL_BLOCK_STRUCT_READS_H

#include "nel/misc/types_nl.h"

#include <string>

namespace NL3D {

/**
 * Rewrites the reads of whole structs out of uniform blocks in the source of a shader (GLSL 3.30 or
 * GLSL ES 3.00), so that the shader only reads members of the blocks:
 *
 *     NlLightInfo li = nlLights[idx];
 *
 * becomes
 *
 *     NlLightInfo li = nlBlockRead_nlLights(int(idx));
 *
 * and behind the declaration of the block, on its last line, comes
 *
 *     NlLightInfo nlBlockRead_nlLights(int nlBlockIndex0) { return NlLightInfo(nlLights[nlBlockIndex0].dirOrPos, ...); }
 *
 * Why: a struct in a uniform block has the layout of the block, and a translator of the shader to
 * another shading language may give a struct value a layout of its own. ANGLE does for Direct3D:
 * for a read of a whole struct it converts the whole array the struct is an element of, as an
 * initialized global, which takes the Direct3D shader compiler about a minute for an array of 128
 * structs of 11 members. Reads of members are not converted. The shader says the same either way,
 * so a shader is written as reads best, and this function gives it the form that compiles fast.
 *
 * What stays as it is: the declarations of the blocks and of the structs, so the data that goes
 * into the blocks; the number of lines; everything that is no such read. The source is taken as
 * tokens between its brackets: the function knows the declarations of structs and of uniform blocks
 * and follows the fields and indices behind the name of a block member. It does not know the types
 * of expressions and does not need to: what it puts in place of a read is a value of the same type.
 *
 * The preprocessor lines are passed on as they are and are not followed: the function sees the
 * text of every branch of an \c \#if at once, and nothing of what a \c \#define stands for.
 *
 * What it leaves alone, and says so in the message. A read that stays costs what all of them cost:
 * ANGLE converts every struct of a block once one read of a whole struct is left.
 *  - Nothing is rewritten in a source where the name of a block member that holds structs, or the
 *    instance name of its block, is declared again as a variable, a parameter, a function or a type
 *    (the reads cannot be told from uses of that other name); where two uniform blocks declare the
 *    same member; where a read needs a struct whose fields depend on a preprocessor line, or that
 *    is declared twice; where a preprocessor line stands inside a read; that has a name beginning
 *    like the ones that are added; or that cannot be taken apart (brackets that do not match).
 *  - The rest is rewritten when an array of structs is read as a whole; when the body of a
 *    \c \#define names such a block member; when an instance name stands without a member behind it;
 *    when a uniform block is an array of blocks, has a preprocessor line between its braces, or has
 *    a member of a type that is no struct of the source and no type of GLSL.
 *
 * \param source the whole source of the shader
 * \param result the source with the reads rewritten; the same text as \c source when there is
 * nothing to rewrite, or when nothing is rewritten
 * \param message empty, or what was left alone and why, with the line where there is one
 * \return true when the message is empty
 */
bool rewriteGlslBlockStructReads(const std::string &source, std::string &result, std::string &message);

} // NL3D

#endif // NL_GLSL_BLOCK_STRUCT_READS_H

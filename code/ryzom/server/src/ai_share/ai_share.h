// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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



#ifndef AI_SHARE_H
#define AI_SHARE_H

namespace NLLIGO
{
	class CLigoConfig;
	class CPrimitives;
}

namespace AI_SHARE 
{
	extern bool LinkWithAiSpawnCommands;
	extern bool LinkWithAiActionCommands;
	extern bool LinkWithPrimitiveParser;
	extern NLLIGO::CLigoConfig *LigoConfig;

	inline void init(NLLIGO::CLigoConfig *config)
	{
		// this code forces the linker to link in files that would otherwise be lost
		// these files use constructors of static objects to instantiate objects
		LinkWithAiSpawnCommands=true;
		LinkWithAiActionCommands=true;
		LinkWithPrimitiveParser=true;
		LigoConfig=config;
	}

	// the parser for '.primitive' files
	extern void parsePrimFile(const std::string &filename);

	// the parser for 'AiActions' stream in pdr (R2 format)
	extern void parsePrimStream(NLMISC::IStream & stream, const std::string & streamName);

	// the parser for 'AiActions' primitives in pdr (R2 format)
	extern void parsePrimNoStream( NLLIGO::CPrimitives* primDoc, const std::string & streamName );

	//-------------------------------------------------------------------------------------------------
	// some handy utilities
	//-------------------------------------------------------------------------------------------------

	// dumb routine to simplify repetitive text parsing code
	inline bool isWhiteSpace(char c)
	{
		return (c==' ' || c=='\t');
	}

	// -- stringToKeywordAndTail() --
	// The following routine splits a text string into a keyword and a tail.
	// A ':' is used as separator between keyword and tail
	// All leading and trailing ' ' and '\t' round keyword and tail characters are stripped
	// If no keyword is found routine retuns false (keyword and tail retain previous content)
	inline bool stringToKeywordAndTail(const std::string &input,std::string &keyword, std::string &tail)
	{
		uint i=0, j, k;

		// skip white space
		while (i<input.size() && isWhiteSpace(input[i])) ++i;		// i points to start of keyword

		// look for the end of the keyword
		for (j=i;j<input.size() && input[j]!=':';) ++j;				// j points to ':' after keyword
		
		// prune any unnecessary white space before ':'
		for (k=j; k>i && isWhiteSpace(input[k-1]);)--k;				// k points to character after end of keyword

		// if no keyword found then give up
		if (k==i) return false;

		// copy out the keyword 
		keyword=input.substr(i,k-i);

		// find the end of the tail text
		for (k=(uint)input.size();k>j && isWhiteSpace(input[k-1]);) --k;	// k points to character after end of tail text

		// find start of tail text
		do { ++j; } while(j<k && isWhiteSpace(input[j]));			// j points to start of tail text

		// copy out the tail (or clear if no tail found in input)
		if (j<k)
			tail=input.substr(j,k-j);
		else
			tail.clear();

		return true;
	}

	// -- stringToWordAndTail() --
	// The following routine splits a text string into a keyword and a tail.
	// A white space is used as separator between keyword and tail
	// All leading and trailing ' ' and '\t' round keyword and tail characters are stripped
	// If no keyword is found routine retuns false (keyword and tail retain previous content)
	inline bool stringToWordAndTail(const std::string &input,std::string &word, std::string &tail)
	{
		uint i=0, j;

		// skip white space
		while (i<input.size() && isWhiteSpace(input[i])) ++i;		// i points to start of word

		// look for the end of the word
		for (j=i;j<input.size() && !isWhiteSpace(input[j]);) ++j;	// j points to next character after word
		
		// if no word found then give up
		if (j==i) return false;

		// copy out the word 
		word=input.substr(i,j-i);

		// find the end of the tail text
		for (i=(uint)input.size();i>j && isWhiteSpace(input[i-1]);) --i;	// i points to character after end of tail text

		// find start of tail text
		do { ++j; } while(j<i && isWhiteSpace(input[j]));			// j points to start of tail text

		// copy out the tail (or clear if no tail found in input)
		if (j<i)
			tail=input.substr(j,i-j);
		else
			tail.clear();

		return true;
	}


} // end of namespace

#endif

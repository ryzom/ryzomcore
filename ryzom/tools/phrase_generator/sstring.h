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


#ifndef PDSLIB_STRING_H
#define PDSLIB_STRING_H

#include <string>


class CSString: public std::string
{
public:
	CSString()
	{
	}

	CSString(const char *s)
	{
		*(std::string *)this=s;
	}

	CSString(const std::string &s)
	{
		*(std::string *)this=s;
	}

	CSString(char c)
	{
		*(std::string *)this=c;
	}

	CSString(int i,const char *fmt="%d")
	{
		char buf[1024];
		sprintf(buf,fmt,i);
		*this=buf;
	}

	CSString(unsigned u,const char *fmt="%u")
	{
		char buf[1024];
		sprintf(buf,fmt,u);
		*this=buf;
	}

	CSString(double d,const char *fmt="%f")
	{
		char buf[1024];
		sprintf(buf,fmt,d);
		*this=buf;
	}

	CSString(const char *s,const char *fmt)
	{
		char buf[1024];
		sprintf(buf,fmt,s);
		*this=buf;
	}

	CSString(const std::string &s,const char *fmt)
	{
		char buf[1024];
		sprintf(buf,fmt,s.c_str());
		*this=buf;
	}

	char operator*()
	{
		if (empty())
			return 0;
		return (*this)[0];
	}

	// return the n right hand most characters of a string
	CSString right(unsigned count) const
	{
		if (count>=size())
			return *this;
		return substr(size()-count);
	}

	// return the string minus the n right hand most characters of a string
	CSString rightCrop(unsigned count) const
	{
		if (count>=size())
			return CSString();
		return substr(0,size()-count);
	}

	// return the n left hand most characters of a string
	CSString left(unsigned count) const
	{
		return substr(0,count);
	}

	// return the string minus the n left hand most characters of a string
	CSString leftCrop(unsigned count) const
	{
		if (count>=size())
			return CSString();
		return substr(count);
	}

	// return sub string up to but not including first instance of given character
	CSString splitTo(char c,bool truncateThis=false)
	{
		unsigned i;
		CSString result;
		for (i=0;i<size() && (*this)[i]!=c;++i)
			result+=(*this)[i];

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size()-1)
				(*this)=substr(i+1);	// +1 to skip the separator character
			else
				clear();
		}

		return result;
	}

	// return sub string up to but not including first instance of given character
	CSString splitTo(const char *s,bool truncateThis=false)
	{
		unsigned i;
		CSString result;
		for (i=0;i<size();++i)
		{
			// perform a quick string compare
			int j;
			for (j=0;s[j]!=0 && s[j]==(&((*this)[i]))[j];++j)
			{
			}
			// if string compare matched then return result so far
			if (s[j]==0)
			{
				// remove the result string from the input string if so desired
				if (truncateThis)
				{
					if (i<size()-1)
						(*this)=substr(i+1);	// +1 to skip the separator character
					else
						clear();
				}

				return result;
			}
			result+=(*this)[i];
		}
		// we didn't find the separator string so we're returning a copy of the whole string
		if (truncateThis)
			clear();
		return result;
	}

	// return sub string from character following first instance of given character on
	CSString splitFrom(char c) const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end() && *it!=c;++it)
		{}
		if (it!=end())
		{
			++it;
			for (;it!=end();++it)
				result+=*it;
		}
		return result;
	}

	// return sub string from character following first instance of given character on
	CSString splitFrom(const char *s) const
	{
		unsigned int i;
		CSString result;
		for (i=0;i<size();++i)
		{
			// perform a quick string compare
			unsigned int j;
			for (j=0;i+j<size() && s[j]!=0 && s[j]==(*this)[i+j];++j)
			{
			}
			// if string compare matched then build and return a result
			if (s[j]==0)
			{
				result=substr(i+j);
				return result;
			}
		}
		return result;
	}

	// behave like a s strtok() routine, returning the sun string extracted from (and removed from) *this
	CSString strtok(const char *separators)
	{
		unsigned int i;
		CSString result;

		// skip leading junk
		for (i=0;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// copy out everything up to the next separator character
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (separators[j])
				break;
			result+=(*this)[i];
		}

		// skip trailing junk
		for (;i<size();++i)
		{
			// look for the next character in the 'separator' character list supplied
			unsigned j;
			for (j=0;separators[j] && (*this)[i]!=separators[j];++j)
			{}
			// if not found then we're at end of leading junk
			if (!separators[j])
				break;
		}

		// delete the treated bit from this string
		(*this)=substr(i);

		return result;
	}

	// return first word (blank separated)
	CSString firstWord(bool truncateThis=false)
	{
		CSString result;
		unsigned i=0;
		// skip white space
		for (i=0;i<size() && isWhiteSpace((*this)[i]);++i)
		{}

		if ( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') || 
			 ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
		{
			// copy out an alpha-numeric string
			for (;i<(*this).size() && 
				( ((*this)[i]>='A' && (*this)[i]<='Z') || ((*this)[i]>='a' && (*this)[i]<='z') || 
				  ((*this)[i]>='0' && (*this)[i]<='9') || (*this)[i]=='_')
				;++i)
				result+=(*this)[i];
		}
		else
		{
			// just take the first character of the input
			result=(*this)[i];
			++i;
		}

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size())
				(*this)=substr(i);
			else
				clear();
		}

		return result;
	}

	CSString firstWordConst() const
	{
		return const_cast<CSString *>(this)->firstWord();
	}

	// return sub string up to but not including first instance of given character
	CSString tailFromFirstWord() const
	{
		CSString hold=*this;
		hold.firstWord(true);
		return hold;
	}

	// count the number of words (or quote delimited sub-strings) in a string
	unsigned countWords() const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstWord().strip();
			++count;
		}
		return count;
	}

	// count the number of words (or quote delimited sub-strings) in a string
	CSString word(unsigned idx) const
	{
		CSString hold=strip();

		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWord().strip();

		return hold.firstWord();
	}

	// return first word or quote-encompassed sub-string
	CSString firstWordOrWords(bool truncateThis=false)
	{
		CSString hold=strip();
		if (hold[0]!='\"')
			return firstWord(truncateThis);

		// the string is quote enclosed
		CSString result;
		unsigned i=1; // skip leading quote
		// copy from character following opening quote to char preceding closing quote (or end of string)
		while (i<hold.size() && hold[i]!='\"')
		{
			result+=hold[i];
			++i;
		}

		// remove the result string from the input string if so desired
		if (truncateThis)
		{
			if (i<size()-1)
				(*this)=substr(i+1);	// +1 to skip the closing quote
			else
				clear();
		}

		return result;
	}

	CSString firstWordOrWordsConst() const
	{
		return const_cast<CSString *>(this)->firstWordOrWords();
	}

	// return sub string up to but not including first instance of given character
	CSString tailFromFirstWordOrWords() const
	{
		CSString hold=*this;
		hold.firstWordOrWords(true);
		return hold;
	}

	// count the number of words (or quote delimited sub-strings) in a string
	unsigned countWordOrWords() const
	{
		unsigned count=0;
		CSString hold=strip();
		while (!hold.empty())
		{
			hold=hold.tailFromFirstWordOrWords().strip();
			++count;
		}
		return count;
	}

	// count the number of words (or quote delimited sub-strings) in a string
	CSString wordOrWords(unsigned idx) const
	{
		CSString hold=strip();
		
		for (unsigned count=0;count<idx;++count)
			hold=hold.tailFromFirstWordOrWords().strip();

		return hold.firstWordOrWords();
	}

	// a handy utility routine for knowing if a character is a white space character or not
	static bool isWhiteSpace(char c) { return c==' ' || c=='\t' || c=='\n' || c=='\r' || c==26; }

	// return a copy of the string with leading and trainling spaces rmoved
	CSString strip() const
	{
		CSString result;
		int i,j;
		for (j=size()-1; j>=0 && isWhiteSpace((*this)[j]); --j) {}
		for (i=0;		 i<j  && isWhiteSpace((*this)[i]); ++i) {}
		for (; i<=j; ++i)
			result+=(*this)[i];
		return result;
	}

	// making an upper case copy of a string
	CSString toUpper() const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end();++it)
		{
			char c=(*it);
			if (c>='a' && c<='z')
				c^=('a'^'A');
			result+=c;
		}
		return result;
	}

	// making a lower case copy of a string
	CSString toLower() const
	{
		CSString result;
		std::string::const_iterator it;
		for (it=begin();it!=end();++it)
		{
			char c=(*it);
			if (c>='A' && c<='Z')
				c^=('a'^'A');
			result+=c;
		}
		return result;
	}

	// replacing all occurences of one string with another
	CSString replace(const char *toFind,const char *replacement) const
	{
		// just bypass the problems that can cause a crash...
		if (toFind==NULL || *toFind==0)
			return *this;

		unsigned i,j;
		CSString result;
		for (i=0;i<size();)
		{
			// string compare toFind against (*this)+i ...
			for (j=0;toFind[j];++j)
				if ((*this)[i+j]!=toFind[j])
					break;
			// if strings were identical then j reffers to ASCIIZ terminator at end of 'toFind'
			if (toFind[j]==0)
			{
				if (replacement!=NULL)
					result+=replacement;
				i+=j;
			}
			else
			{
				result+=(*this)[i];
				++i;
			}
		}
		return result;
	}

	// find index at which a sub-string starts - if sub-string not found then returns size()
	unsigned find(const char *toFind,unsigned startLocation=0) const
	{
		// just bypass the problems that can cause a crash...
		if (toFind==NULL || *toFind==0 || startLocation>size())
			return size();

		unsigned i,j;
		for (i=startLocation;i<size();++i)
		{
			// string compare toFind against (*this)+i ...
			for (j=0;toFind[j];++j)
				if ((*this)[i+j]!=toFind[j])
					break;
			// if strings were identical then we're done
			if (toFind[j]==0)
				return i;
		}
		return i;
	}

	// return true if this contains given sub string
	bool contains(const char *toFind) const
	{
		return find(toFind)!=size();
	}

	// a couple of handy atoi routines...
	template <class C> bool atoi(C& result) const
	{
		result=::atoi(c_str());
		return (result!=0 || *this=="0");
	}
	unsigned atoi() const
	{
		return ::atoi(c_str());
	}

	// a couple of handy atof routines...
	template <class C> bool atof(C& result) const
	{
		result=::atof(c_str());
		return (result!=0 || *this=="0");
	}
	double atof() const
	{
		return ::atof(c_str());
	}

	// case insensitive string compare
	bool operator==(const std::string &other) const
	{
		return stricmp(c_str(),other.c_str())==0;
	}

	// case insesnsitive string compare
	bool operator!=(const std::string &other) const
	{
		return !(*this==other);
	}
};

#endif

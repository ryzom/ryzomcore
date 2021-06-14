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

/*
  This file provides systems for verifying that STL allocators have not been damaged

  The firt implementation only tests the allocator for blocks of <= 8 bytes.
  This is because this is the only allocator causing code crashes at the time of writing.
  This allocator is reffered to as Allocator '0'

  The following macros are provided to facilitate code instrumentaion:
  STL_ALLOC_TEST - performs a punctual test at the indicated position in the code
  STL_ALLOC_CONTEXT - instantiates an object of type CStlAllocatorChecker that performs a test at construction time and another test at destruction time

  The NLMISC variable 'EnableStlAllocatorChecker' can be used to enable and dissable the tests provided by the above macros
*/

#ifndef RY_STL_ALLOCATOR_CHECKER_H
#define RY_STL_ALLOCATOR_CHECKER_H

extern bool EnableStlAllocatorChecker;

void testStlMemoryAllocator(const char* context);

class CStlAllocatorChecker
{
public:
	CStlAllocatorChecker(const char* startContextName,const char* endContextName);
	~CStlAllocatorChecker();
private:
	const char* _StartContextName;
	const char* _EndContextName;
};

inline CStlAllocatorChecker::CStlAllocatorChecker(const char* startContextName,const char* endContextName): _StartContextName(startContextName), _EndContextName(endContextName)
{
	if (EnableStlAllocatorChecker)
		testStlMemoryAllocator(_StartContextName);
}

inline CStlAllocatorChecker::~CStlAllocatorChecker()
{
	if (EnableStlAllocatorChecker)
		testStlMemoryAllocator(_EndContextName);
}

#define STL_ALLOC_CONTEXT _STL_ALLOC_CONTEXT1(__FILE__,__LINE__)
#define _STL_ALLOC_CONTEXT1(file,line) _STL_ALLOC_CONTEXT2(file,line)
#define _STL_ALLOC_CONTEXT2(file,line) _STL_ALLOC_CONTEXT3(file ":" #line)
#define _STL_ALLOC_CONTEXT3(fileLine) CStlAllocatorChecker __StlAllocatorChecker__(fileLine ": @construction",fileLine ": @destruction");

#define STL_ALLOC_TEST _STL_ALLOC_TEST1(__FILE__,__LINE__)
#define _STL_ALLOC_TEST1(file,line) _STL_ALLOC_TEST2(file,line)
#define _STL_ALLOC_TEST2(file,line) if (!EnableStlAllocatorChecker) {} else testStlMemoryAllocator(file ":" #line);

#endif

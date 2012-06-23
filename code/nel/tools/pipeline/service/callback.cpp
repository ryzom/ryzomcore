/**
 * \file callback.cpp
 * \brief CCallback
 * \date 2009-03-03 18:09GMT
 * \author Jan Boon (Kaetemi)
 * CCallback
 */

/* 
 * Copyright (C) 2009-2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "callback.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

typedef CCallback<bool, const int &> CTestCallback;
bool test_blah(const int &) { return false; }
class CTestMeeeee { public: CTestMeeeee() { } bool test_method(const int &) { return true; } };
void dummy_callback_cpp() { int whaha = 0; CTestCallback testCallback; testCallback.callback(whaha); testCallback = CTestCallback(test_blah); 
	CTestMeeeee blahsomething; testCallback = CTestCallback(&blahsomething, &CTestMeeeee::test_method); }

} /* namespace PIPELINE */

/* end of file */

// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include "stdmisc.h"

#include "nel/misc/eval_num_expr.h"
#include "nel/misc/debug.h"

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

using namespace std;

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::readDecimal (double &value)
{
	// Read first integer value
	readIntegerNumberDecimal (value);

	// Dot ?
	char currentChar = *_ExprPtr;
	if (currentChar == '.')
	{
		// Next char
		_ExprPtr++;
		currentChar = *_ExprPtr;
		if (currentChar < '0' || currentChar > '9')
			return NumberSyntaxError;

		// Read the decimal part
		const char *start = _ExprPtr;
		double fract;
		readIntegerNumberDecimal (fract);
		fract /= (double)pow (10.0,(sint)(_ExprPtr-start));
		value += fract;
	}

	return NoError;
}

// ***************************************************************************

void CEvalNumExpr::readIntegerNumberDecimal (double &value)
{
	// Registered values
	register double regValue = 0;

	// Read the first value
	char currentChar = *_ExprPtr;
	if ((currentChar >= '0') && (currentChar <= '9'))
	{
		regValue += (currentChar - '0');
		_ExprPtr++;
		currentChar = *_ExprPtr;

		// For each values
		while ((currentChar >= '0') && (currentChar <= '9'))
		{
			regValue *= 10;
			regValue += (currentChar - '0');

			// Next char
			_ExprPtr++;
			currentChar = *_ExprPtr;
		}
	}

	// Store value
	value = regValue;
}

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::getNextToken (TToken &token)
{
	// Get the current char
	uint8 currentChar = *_ExprPtr;

	// Skip space
	while ((currentChar!=0) && (currentChar<=0x20))
	{
		_ExprPtr++;
		currentChar = *_ExprPtr;
	}

	// Can be an operator ?
	if (currentChar <= 128)
	{
		// Get the operator
		_Op = _OperatorArray[currentChar];

		// Is an operator ?
		if (_Op != NotOperator)
		{
			// It is an operator
			token = Operator;

			// Is a 2 characters operator ?
			if (_Op != ExtOperator)
			{
				// Return next character
				_ExprPtr++;
				return NoError;
			}
			else
			{
				// Have a second character ?
				char secondChar = *(_ExprPtr+1);

				// What kind of 1st character
				switch (currentChar)
				{
				case '!':
					if (secondChar == '=')
					{
						_Op = NotEqual;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Not;
						_ExprPtr++;
						return NoError;
					}
				case '&':
					if (secondChar == '&')
					{
						_Op = LogicalAnd;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = And;
						_ExprPtr+=1;
						return NoError;
					}
				case '-':
					if (secondChar == '>')
					{
						_Op = SRightShift;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Minus;
						_ExprPtr++;
						return NoError;
					}
				case '<':
					if (secondChar == '<')
					{
						_Op = ULeftShift;
						_ExprPtr+=2;
						return NoError;
					}
					else if (secondChar == '=')
					{
						_Op = InferiorEqual;
						_ExprPtr+=2;
						return NoError;
					}
					else if (secondChar == '-')
					{
						_Op = SLeftShift;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Inferior;
						_ExprPtr+=1;
						return NoError;
					}
				case '=':
					if (secondChar == '=')
					{
						_Op = Equal;
						_ExprPtr+=2;
						return NoError;
					}
				case '>':
					if (secondChar == '>')
					{
						_Op = URightShift;
						_ExprPtr+=2;
						return NoError;
					}
					else if (secondChar == '=')
					{
						_Op = SuperiorEqual;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Superior;
						_ExprPtr+=1;
						return NoError;
					}
				case '^':
					if (secondChar == '^')
					{
						_Op = LogicalXor;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Xor;
						_ExprPtr+=1;
						return NoError;
					}
				case '|':
					if (secondChar == '|')
					{
						_Op = LogicalOr;
						_ExprPtr+=2;
						return NoError;
					}
					else
					{
						_Op = Or;
						_ExprPtr+=1;
						return NoError;
					}
				}

				// Can't found the operator
				return UnknownOperator;
			}
		}
		// Is End, '(', ')', '.' ?
		else if (currentChar == 0)
		{
			token = End;
			return NoError;
		}
		else if (currentChar == '(')
		{
			_ExprPtr++;
			token = Open;
			return NoError;
		}
		else if (currentChar == ')')
		{
			_ExprPtr++;
			token = Close;
			return NoError;
		}
		else if (currentChar == ',')
		{
			_ExprPtr++;
			token = Coma;
			return NoError;
		}
		// Is a number ?
		else if (((currentChar >= '0') && (currentChar <= '9')) || (currentChar == '.'))
		{
			// This is a number
			token = Number;

			// Have a second character ?
			char secondChar = *(_ExprPtr+1);

			// Is an hexadecimal value ?
			if ((currentChar == '0') && (secondChar == 'x'))
			{
				// Go to the number
				_ExprPtr +=2;
				currentChar = *_ExprPtr;

				// Registered values
				register double regValue = 0;
				if ((currentChar >= '0') && (currentChar <= '9'))
				{
					regValue += (currentChar - '0');
				}
				else if ((currentChar >= 'a') && (currentChar <= 'f'))
				{
					regValue += (currentChar - 'a' + 10);
				}
				else if ((currentChar >= 'A') && (currentChar <= 'F'))
				{
					regValue += (currentChar - 'A' + 10);
				}
				else
				{
					// Number syntax error
					return NumberSyntaxError;
				}
				_ExprPtr++;
				currentChar = *_ExprPtr;

				// For each values
				for(;;)
				{
					if ((currentChar >= '0') && (currentChar <= '9'))
					{
						regValue *= 16;
						regValue += (currentChar - '0');
					}
					else if ((currentChar >= 'a') && (currentChar <= 'f'))
					{
						regValue *= 16;
						regValue += (currentChar - 'a' + 10);
					}
					else if ((currentChar >= 'A') && (currentChar <= 'F'))
					{
						regValue *= 16;
						regValue += (currentChar - 'A' + 10);
					}
					else
					{
						// Stop
						break;
					}

					// Next char
					_ExprPtr++;
					currentChar = *_ExprPtr;
				}

				// Store value
				_Value = regValue;

				// Number ok
				return NoError;
			}
			// Is an octal value ?
			else if ((currentChar == '0') && (secondChar >= '0') && (secondChar <= '9'))
			{
				// Go to the number
				_ExprPtr ++;
				currentChar = *_ExprPtr;

				// Registered values
				register double regValue = 0;

				// Check octal number
				if (currentChar > '7')
					return NumberSyntaxError;

				// Read the first value
				regValue += (currentChar - '0');
				_ExprPtr++;
				currentChar = *_ExprPtr;

				// For each values
				while ((currentChar >= '0') && (currentChar <= '9'))
				{
					// Check octal number
					if (currentChar > '7')
						return NumberSyntaxError;

					regValue *= 8;
					regValue += (currentChar - '0');

					// Next char
					_ExprPtr++;
					currentChar = *_ExprPtr;
				}

				// Store value
				_Value = regValue;

				// Number ok
				return NoError;
			}
			// It is a decimal value
			else
			{
				// Read value
				TReturnState state = readDecimal (_Value);
				if (state == NoError)
				{
					// Exponent ?
					currentChar = *_ExprPtr;
					if ( (currentChar == 'e') || (currentChar == 'E') )
					{
						// Next char
						_ExprPtr++;

						// Minus ?
						bool negative = false;
						if (*_ExprPtr == '-')
						{
							negative = true;
							_ExprPtr++;
						}

						// Read value
						double exponent;
						state = readDecimal (exponent);
						if (state == NoError)
						{
							// Negative value ?
							if (negative)
								exponent = -exponent;

							// Raise 10 at the power of
							_Value *= pow (10.0, exponent);
						}
						else
						{
							return state;
						}
					}

					// Number ok
					return NoError;
				}
				else
				{
					return state;
				}
			}
		}
		// Is a string ?
		else if (currentChar == '"')
		{
			// Look for the end of the string
			_ExprPtr++;
			currentChar = *_ExprPtr;
			const char *start = _ExprPtr;
			while ( (currentChar != 0) && (currentChar != '"') )
			{
				_ExprPtr++;
				currentChar = *_ExprPtr;
			}

			// End reached ?
			if (currentChar == 0)
				return MustBeDoubleQuote;

			// This is a user string, copy the string
			uint size = (uint)(_ExprPtr - start);
			if (size >= (InternalStringLen-1))
			{
				_InternalStlString.resize (size);
				uint i;
				for (i=0; i<size; i++)
					_InternalStlString[i] = start[i];
				_InternalStringPtr = _InternalStlString.c_str ();
			}
			else
			{
				memcpy (_InternalString, start, size);
				_InternalString[size] = 0;
				_InternalStringPtr = _InternalString;
			}

			// Token
			_ExprPtr++;
			token = String;
			return NoError;
		}
	}

	// Read a string
	const char *start = _ExprPtr;
	while ( (currentChar >= 128) || _StringChar[currentChar] )
	{
		_ExprPtr++;
		currentChar = *_ExprPtr;
	}

	// Is pi ?
	if (((_ExprPtr - start) == 2) && (start[0] == 'p') && (start[1] == 'i'))
	{
		token = Number;
		_Value = 3.1415926535897932384626433832795;
		return NoError;
	}
	// Is e ?
	else if (((_ExprPtr - start) == 1) && (start[0] == 'e'))
	{
		token = Number;
		_Value = 2.7182818284590452353602874713527;
		return NoError;
	}

	// This is a user string, copy the string
	uint size = (uint)(_ExprPtr - start);
	if (size >= (InternalStringLen-1))
	{
		_InternalStlString.resize (size);
		uint i;
		for (i=0; i<size; i++)
			_InternalStlString[i] = start[i];
		_InternalStringPtr = _InternalStlString.c_str ();
	}
	else
	{
		memcpy (_InternalString, start, size);
		_InternalString[size] = 0;
		_InternalStringPtr = _InternalString;
	}

	// Search for a reserved word ?
	uint begin = 0;
	uint end = ReservedWordCount-1;
	sint result = strcmp (_InternalStringPtr, _ReservedWord[begin]);
	if ( result >= 0 )
	{
		// The first is the good ?
		if ( result == 0 )
		{
			end = begin;
		}

		result = strcmp (_InternalStringPtr, _ReservedWord[end]);
		if (result <= 0)
		{
			// The last is the good ?
			if ( result == 0 )
			{
				begin = end;
			}

			// While there is a middle..
			while ((end - begin) > 1)
			{
				uint middle = begin + (end - begin) / 2;
				result = strcmp (_InternalStringPtr, _ReservedWord[middle]);
				if (result == 0)
				{
					begin = middle;
					end = middle;
					break;
				}
				else if (result < 0)
				{
					end = middle;
				}
				else
				{
					begin = middle;
				}
			}
		}

		// Found ?
		if (end == begin)
		{
			// Return the token
			_ReservedWordFound = (TReservedWord)begin;
			token = _ReservedWordToken[begin];

			// Ok
			return NoError;
		}
	}

	// Token
	token = String;
	return NoError;
}

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::evalExpression (const char *expression, double &result,
														 int *errorIndex, uint32 userData)
{
	// Init the ptr
	_ExprPtr = expression;

	TToken nextToken;
	TReturnState error = evalExpression (result, nextToken, userData);
	if (error == NoError)
	{
		// The end ?
		if (nextToken == End)
			return NoError;
		else
		{
			if (errorIndex)
				*errorIndex = (int)(_ExprPtr - expression);
			return MustBeEnd;
		}
	}
	else
	{
		if (errorIndex)
			*errorIndex = (int)(_ExprPtr - expression);
		return error;
	}
}

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::evalExpression (double &finalResult, TToken &nextToken, uint32 userData)
{
	// Array of result

	uint exprCount = 0;
	double result[InternalOperator];
	vector<double> resultSup;

	uint opCount = 0;
	TOperator resultOp[InternalOperator];
	vector<TOperator> resultOpSup;

	// Read a token
	TReturnState error = getNextToken (nextToken);
	if (error != NoError)
		return error;
	for(;;)
	{
		// Unary opertor
		uint unaryOpCount = 0;
		TOperator resultUnaryOp[InternalOperator];
		vector<TOperator> resultUnaryOpSup;

		// init table
		for (uint i = 0; i < (uint)InternalOperator; ++i) resultUnaryOp[i] = NotOperator;

		// Current value
		double value;

		// Unary operator ?
		if ( (nextToken == Operator) && ( (_Op == Minus) || (_Op == Not) || (_Op == Tilde) ) )
		{
			// Push the unary operator
			if (unaryOpCount<InternalOperator)
				resultUnaryOp[unaryOpCount] = _Op;
			else
				resultUnaryOpSup.push_back (_Op);
			unaryOpCount++;

			// Read next token
			error = getNextToken (nextToken);
			if (error != NoError)
				return error;
		}

		// Parenthesis ?
		if (nextToken == Open)
		{
			// Eval sub expression
			error = evalExpression (value, nextToken, userData);
			if (error == NoError)
			{
				if (nextToken != Close)
					return MustBeClose;
			}
			else
				return error;

			// Get next token
			error = getNextToken (nextToken);
			if (error != NoError)
				return error;
		}
		// This is a function ?
		else if ( (nextToken == Function1) || (nextToken == Function2) )
		{
			TToken backupedToken = nextToken;

			// Get the function
			TReservedWord reservedWord = _ReservedWordFound;

			// Read a token
			error = getNextToken (nextToken);
			if (error == NoError)
			{
				// Open ?
				if (nextToken != Open)
				{
					return MustBeOpen;
				}

				// Eval an expression
				double arg0;
				error = evalExpression (arg0, nextToken, userData);
				if (error == NoError)
				{
					// 2 arg ?
					if (backupedToken == Function2)
					{
						if (nextToken == Coma)
						{
							// Second argument
							double arg1;
							error = evalExpression (arg1, nextToken, userData);
							if (error == NoError)
							{
								// Final with close ?
								if (nextToken == Close)
								{
									switch (reservedWord)
									{
									case Atan2:
										value = atan2 (arg0, arg1);
										break;
									case Max:
										value = (arg0>arg1) ? arg0 : arg1;
										break;
									case Min:
										value = (arg0<arg1) ? arg0 : arg1;
										break;
									case Pow:
										value = pow (arg0, arg1);
										break;
									case Rand:
										value = arg0 + (arg1-arg0) * (double)rand () / ((double)RAND_MAX+1.0);
										break;
									default:
										// Can't be here after getToken
										nlstop;
									}
								}
								else
									return MustBeClose;
							}
							else
								return error;
						}
						else
							return MustBeComa;
					}
					else
					{
						if (nextToken == Close)
						{
							// Eval the function
							switch (reservedWord)
							{
							case Abs:
								value = fabs (arg0);
								break;
							case Acos:
								value = acos (arg0);
								break;
							case Asin:
								value = asin (arg0);
								break;
							case Atan:
								value = atan (arg0);
								break;
							case Ceil:
								value = ceil (arg0);
								break;
							case Cosh:
								value = cosh (arg0);
								break;
							case Cos:
								value = cos (arg0);
								break;
							case Exponent:
								{
									int exponent;
									frexp( arg0, &exponent);
									value = (double)exponent;
								}
								break;
							case Exp:
								value = exp (arg0);
								break;
							case Floor:
								value = floor (arg0);
								break;
							case Int:
								value = (double)(int)(arg0);
								break;
							case Log10:
								value = log10 (arg0);
								break;
							case Log:
								value = log (arg0);
								break;
							case Mantissa:
								{
									int exponent;
									value = frexp( arg0, &exponent);
								}
								break;
							case Round:
								value = floor (arg0 + 0.5);
								break;
							case Sinh:
								value = sinh (arg0);
								break;
							case Sin:
								value = sin (arg0);
								break;
							case Sqrt:
								value = sqrt (arg0);
								break;
							case Sq:
								value = arg0 * arg0;
								break;
							case Tanh:
								value = tanh (arg0);
								break;
							case Tan:
								value = tan (arg0);
								break;
							default:
								// Can't be hear after getToken
								nlstop;
							}
						}
						else
							return MustBeClose;
					}
				}
				else
					return error;
			}
			else
				return error;

			// Get next token
			error = getNextToken (nextToken);
			if (error != NoError)
				return error;
		}
		else if (nextToken == Number)
		{
			// Save the internal value
			value = _Value;

			// Get next token
			error = getNextToken (nextToken);
			if (error != NoError)
				return error;
		}
		else if (nextToken == String)
		{
			// Copy the string
			char			internalString[InternalStringLen];
			std::string		internalStlString;
			const char		*internalStringPtr;
			if (strlen (_InternalStringPtr) >= InternalStringLen-1)
			{
				internalStlString = _InternalStringPtr;
				internalStringPtr = internalStlString.c_str ();
			}
			else
			{
				strcpy (internalString ,_InternalStringPtr);
				internalStringPtr = internalString;
			}

			// Read a token
			error = getNextToken (nextToken);
			if (error == NoError)
			{
				// Open ?
				if (nextToken == Open)
				{
					// Eval an expression
					double arg0;
					error = evalExpression (arg0, nextToken, userData);
					if (error == NoError)
					{
						if (nextToken == Coma)
						{
							// Second argument
							double arg1;
							error = evalExpression (arg1, nextToken, userData);
							if (error == NoError)
							{
								// Final with close ?
								if (nextToken == Close)
								{
									// Eval the function
									error = evalFunction (internalStringPtr, arg0, arg1, value);
									if (error != NoError)
										return error;

									// Get next token
									error = getNextToken (nextToken);
									if (error != NoError)
										return error;
								}
								else
									return MustBeClose;
							}
							else
								return error;
						}
						else
						{
							if (nextToken == Close)
							{
								// Eval the function
								error = evalFunction (internalStringPtr, arg0, value);
								if (error != NoError)
									return error;

								// Get next token
								error = getNextToken (nextToken);
								if (error != NoError)
									return error;
							}
							else
								return MustBeClose;
						}
					}
					else
						return error;
				}
				else
				{
					// This is a user value
					error = evalValue (internalStringPtr, value, userData);
					if (error != NoError)
						return error;
				}
			}
			else
				return error;
		}
		else
		{
			return MustBeExpression;
		}

		// Eval unary operator
		sint i;
		for (i=unaryOpCount-1; i>=0; i--)
		{
			switch ((i<InternalOperator)?resultUnaryOp[i]:resultUnaryOpSup[i-InternalOperator])
			{
			case Not:
				value = (double)(uint)((floor (value+0.5)==0.0));
				break;
			case Tilde:
				value = (double)(~((uint)floor (value+0.5)) & std::numeric_limits<uint>::max());
				break;
			case Minus:
				value = -value;
				break;
			default:
				// Can't be hear after getToken
				nlstop;
			}
		}

		// Push the value
		if (exprCount < InternalOperator)
			result[exprCount] = value;
		else
			resultSup.push_back (value);
		exprCount++;

		// Look for an operator
		// Operator ?
		if (nextToken == Operator)
		{
			// Yes, push it
			if (opCount < InternalOperator)
				resultOp[opCount] = _Op;
			else
				resultOpSup.push_back (_Op);
			opCount++;
		}
		else
		{
			// Exit the evaluate loop
			break;
		}

		// Next token
		error = getNextToken (nextToken);
	}

	// Reduce the expression
	uint index = 1;
	while (exprCount != 1)
	{
		// Reduct ?
		TOperator before = (((index-1)<InternalOperator)?resultOp[index-1]:resultOpSup[index-1-InternalOperator]);
		TOperator after = (index < opCount)?(((index)<InternalOperator)?resultOp[index]:resultOpSup[index-InternalOperator]):NotOperator;
		if ((index == opCount) || (_OperatorPrecedence[before] <= _OperatorPrecedence[after]))
		{
			// Eval the value
			double &v0 = ((index-1)<InternalOperator)?result[index-1]:resultSup[index-1-InternalOperator];
			double &v1 = ((index)<InternalOperator)?result[index]:resultSup[index-InternalOperator];

			// Choose the operator
			switch (before)
			{
			case Not:
			case Tilde:
				return NotUnaryOperator;
			case Mul:
				v0 *= v1;
				break;
			case Div:
				if (v1 == 0)
				{
					return DividByZero;
				}
				else
				{
					v0 /= v1;
				}
				break;
			case Remainder:
				v0 = fmod (v0, v1);
				break;
			case Plus:
				v0 += v1;
				break;
			case Minus:
				v0 -= v1;
				break;
			case ULeftShift:
				v0 = (double)(((uint)floor (v0 + 0.5))<<((uint)floor (v1 + 0.5)));
				break;
			case URightShift:
				v0 = (double)(((uint)floor (v0 + 0.5))>>((uint)floor (v1 + 0.5)));
				break;
			case SLeftShift:
				v0 = (double)(((sint)floor (v0 + 0.5))<<((sint)floor (v1 + 0.5)));
				break;
			case SRightShift:
				v0 = (double)(((sint)floor (v0 + 0.5))>>((sint)floor (v1 + 0.5)));
				break;
			case Inferior:
				v0 = (v0<v1)?1.0:0.0;
				break;
			case InferiorEqual:
				v0 = (v0<=v1)?1.0:0.0;
				break;
			case Superior:
				v0 = (v0>v1)?1.0:0.0;
				break;
			case SuperiorEqual:
				v0 = (v0>=v1)?1.0:0.0;
				break;
			case Equal:
				v0 = (v0==v1)?1.0:0.0;
				break;
			case NotEqual:
				v0 = (v0!=v1)?1.0:0.0;
				break;
			case And:
				v0 = (double)(((uint)floor (v0 + 0.5)) & ((uint)floor (v1 + 0.5)));
				break;
			case Or:
				v0 = (double)(((uint)floor (v0 + 0.5)) | ((uint)floor (v1 + 0.5)));
				break;
			case Xor:
				v0 = (double)(((uint)floor (v0 + 0.5)) ^ ((uint)floor (v1 + 0.5)));
				break;
			case LogicalAnd:
				v0 = (double)(uint)((floor (v0 + 0.5) != 0.0) && (floor (v1 + 0.5) != 0.0));
				break;
			case LogicalOr:
				v0 = (double)(uint)((floor (v0 + 0.5) != 0.0) || (floor (v1 + 0.5) != 0.0));
				break;
			case LogicalXor:
				{
					bool b0 = floor (v0 + 0.5) != 0.0;
					bool b1 = floor (v1 + 0.5) != 0.0;
					v0 = (double)(uint)((b0&&!b1) || ((!b0)&&b1));
				}
				break;
			default:
				nlstop;
			}

			// Decal others values
			uint i = index;
			for (; i<exprCount-1; i++)
			{
				// Copy
				if (i<InternalOperator)
					result[i] = (i+1<InternalOperator)?result[i+1]:resultSup[i+1-InternalOperator];
				else
					resultSup[i-InternalOperator] = (i+1<InternalOperator)?result[i+1]:resultSup[i+1-InternalOperator];
			}
			exprCount--;

			// Decal operators
			i = index-1;
			for (; i<opCount-1; i++)
			{
				// Copy
				if (i<InternalOperator)
					resultOp[i] = (i+1<InternalOperator)?resultOp[i+1]:resultOpSup[i+1-InternalOperator];
				else
					resultOpSup[i-InternalOperator] = (i+1<InternalOperator)?resultOp[i+1]:resultOpSup[i+1-InternalOperator];

			}
			opCount--;

			// Last one ?
			if (index > 1)
				index--;
		}
		else
			index++;
	}

	finalResult = result[0];
	return NoError;
}

// ***************************************************************************

bool CEvalNumExpr::internalCheck ()
{
	for (uint i=0; i<ReservedWordCount-1; i++)
		if (strcmp (_ReservedWord[i], _ReservedWord[i+1]) >= 0)
		{
			nlstop;
			return false;
		}
	return true;
}

// ***************************************************************************

// ASCII TABLE

/*
0 NUL	SOH		STX		ETX		EOT		ENQ		ACK		BEL		BS		TAB		LF		VT		FF		CR		SO		SI
1 DLE	DC1		DC2		DC3		DC4		NAK		SYN		ETB		CAN		EM		SUB		ESC		FS		GS		RS		US
2		!		"		#		$		%		&		'		(		)		*		+		,		-		.		/
3 0		1		2		3		4		5		6		7		8		9		:		;		<		=		>		?
4 @		A		B		C		D		E		F		G		H		I		J		K		L		M		N		O
5 P		Q		R		S		T		U		V		W		X		Y		Z		[		\		]		^		_
6 `		a		b		c		d		e		f		g		h		i		j		k		l		m		n		o
7 p		q		r		s		t		u		v		w		x		y		z		{		|		}		~
*/

// ***************************************************************************

const CEvalNumExpr::TOperator	CEvalNumExpr::_OperatorArray[128] =
{
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator,
	NotOperator, ExtOperator, NotOperator, NotOperator, NotOperator, Remainder,   ExtOperator, NotOperator, NotOperator, NotOperator, Mul,		   Plus,		NotOperator, ExtOperator, NotOperator, Div,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, ExtOperator, ExtOperator, ExtOperator, NotOperator,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, ExtOperator, NotOperator,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator,
	NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, NotOperator, ExtOperator, NotOperator, Tilde,	   NotOperator,
};

// ***************************************************************************

const bool CEvalNumExpr::_StringChar[128] =
{
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
	false, false, true,  true,  true,  false, false, true,  false, false, false, false, false, false, false, false,
	true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, false, false, true,
	true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
	true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false,  true,
	true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,
	true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  false, true,  false, true,
};

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::evalValue (const char *value, double &result, uint32 userData)
{
	return UnknownValue;
}

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::evalFunction (const char *funcName, double arg0, double &result)
{
	return UnknownFunction;
}

// ***************************************************************************

CEvalNumExpr::TReturnState CEvalNumExpr::evalFunction (const char *funcName, double arg0, double arg1, double &result)
{
	return UnknownFunction;
}

// ***************************************************************************

const char *CEvalNumExpr::_ReservedWord[ReservedWordCount] =
{
	"abs", // Abs
	"acos", // Acos
	"asin", // Asin
	"atan", // Atan
	"atan2", // Atan2
	"ceil", // Ceil
	"cos",	// Cos
	"cosh", // Cosh
	"exp", // Exp
	"exponent", // Exponent
	"floor", // Floor
	"int", // Int
	"log", // Log
	"log10", // Log10
	"mantissa", // Mantissa
	"max", // Max
	"min", // Min
	"pow", // Pow
	"rand", // Rand
	"round", // Round
	"sin", // Sin
	"sinh", // Sinh
	"sq", // Sq
	"sqrt", // Sqrt
	"tan", // Tan
	"tanh", // Tanh
};

// ***************************************************************************

const CEvalNumExpr::TToken CEvalNumExpr::_ReservedWordToken[ReservedWordCount] =
{
	Function1, // Abs
	Function1, // Acos
	Function1, // Asin
	Function1, // Atan
	Function2, // Atan2
	Function1, // Ceil
	Function1, // Cos
	Function1, // Cosh
	Function1, // Exp
	Function1, // Exponent
	Function1, // Floor
	Function1, // Int
	Function1, // Log
	Function1, // Log10
	Function1, // Mantissa
	Function2, // Max
	Function2, // Min
	Function2, // Pow
	Function2, // Rand
	Function1, // Round
	Function1, // Sin
	Function1, // Sinh
	Function1, // Sq
	Function1, // Sqrt
	Function1, // Tan
	Function1, // Tanh
};

// ***************************************************************************

const char *CEvalNumExpr::_ErrorString[ReturnValueCount]=
{
	"No error",
	"Unknown value",
	"Error during user defined value evaluation",
	"Unknown function",
	"Error during user defined function evaluation",
	"Syntax error in a number expression",
	"Unknown operator",
	"Should be a open parentesis",
	"Should be a close parentesis",
	"Should be a coma character",
	"Should be an expression",
	"Should not be an unary operator",
	"Should be the end of the expression",
	"Should be a double quote",
	"Divid by zero",
};

// ***************************************************************************

const char* CEvalNumExpr::getErrorString (TReturnState state) const
{
	return _ErrorString[state];
}

// ***************************************************************************

const int CEvalNumExpr::_OperatorPrecedence[]=
{
	0,	// Not
	0,	// Tilde
	1,	// Mul
	1,	// Div
	1,	// Remainder
	2,	// Plus
	2,	// Minus
	3,	// ULeftShift
	3,	// URightShift
	3,	// SLeftShift
	3,	// SRightShift
	4,	// Inferior
	4,	// InferiorEqual
	4,	// Superior
	4,	// SuperiorEqual
	5,	// Equal
	5,	// NotEqual
	6,	// And
	7,	// Or
	8,	// Xor
	9,	// LogicalAnd
	10,	// LogicalOr
	11,	// LogicalXor
	-1,	// OperatorCount
	20,	// NotOperator
};

// ***************************************************************************

}

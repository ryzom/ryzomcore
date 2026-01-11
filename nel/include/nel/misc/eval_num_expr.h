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

#ifndef NL_EVAL_NUM_EXPR_H
#define NL_EVAL_NUM_EXPR_H

#include "types_nl.h"

namespace NLMISC
{

/**
  * This class performs numerical expression parsing.
  */
class CEvalNumExpr
{
public:
	virtual ~CEvalNumExpr() {}
	/// Eval return error.
	enum TReturnState
	{
		NoError,			// No error
		UnknownValue,		// Unknown value has been parsed
		ValueError,			// Error during user defined value evaluation
		UnknownFunction,	// Unknown function has been parsed
		FunctionError,		// Error during user defined function evaluation
		NumberSyntaxError,	// Syntax error in a number expression
		UnknownOperator,	// Unknown operator
		MustBeOpen,			// Should be a open parenthesis
		MustBeClose,		// Should be a close parenthesis
		MustBeComa,			// Should be a coma character
		MustBeExpression,	// Should be an expression
		NotUnaryOperator,	// Should not be an unary operator
		MustBeEnd,			// Should be the end
		MustBeDoubleQuote,	// Should be a double quote
		DividByZero,		// Divide by zero
		ReturnValueCount
	};

	/**
	  * Evaluate a numerical expression.
	  *
	  * Doesn't allocate heap memory for common complexity expression.
	  *
	  * \param expression is an expression string. See the expression grammar.
	  * \param result is filled with the result if the function returns "NoError".
	  * \param errorIndex is a pointer on an integer value filled with the index
	  * of the parsing error in the input string if function doesn't return "NoError".
	  * This value can be NULL.
	  * \param userData is a user data used by user eval function
	  *	\return NoError if the expression has been parsed. Result is filled with the evaluated value.
	  *
	  * This expression must follow the following grammar with the following evaluation priority:
	  *
	  * expression:	'-' expression
	  *				| '!' expression		// Returns true if a equal false, else false (logical not)
	  *				| '~' expression		// Returns ~ round(a) (bitwise not)
	  *				| '(' expression ')'
	  *				| expression operator expression
	  *				| function1 '(' expression ')'
	  *				| function2 '(' expression ',' expression ')'
	  *				| number
	  *				| constant
	  *				| string	// User defined value, evaluated by the evalValue() callback
	  *
	  * operator:	'*'		// Calculates (a * b)
	  *				| '/'		// Calculates (a / b)
	  *				| '%'		// Calculates the remainder of (a / b)
	  *				| '+'		// Calculates (a + b)
	  *				| '-'		// Calculates (a - b)
	  *				| '<<'		// Returns round(a) left 32 bits unsigned shift by round(b)
	  *				| '>>'		// Returns round(a) right 32 bits unsigned shift by round(b)
	  *				| '<-'		// Returns round(a) left 32 bits signed shift by round(b)
	  *				| '->'		// Returns round(a) right 32 bits signed shift by round(b)
	  *				| '<'		// Returns true if a is strictly smaller than b
	  *				| '<='		// Returns true if a is smaller or equal than b
	  *				| '>'		// Returns true if a is strictly bigger than b
	  *				| '>='		// Returns true if a is bigger or equal than b
	  *				| '=='		// Returns true if a equal b, else returns false (warning, performs a floating point comparison)
	  *				| '!='		// Returns false if a equal b, else returns true (warning, performs a floating point comparison)
	  *				| '&'		// Returns round(a) & round(b) over 32 bits
	  *				| '|'		// Returns round(a) | round(b) over 32 bits
	  *				| '^'		// Returns round(a) ^ round(b) over 32 bits
	  *				| '&&'		// Returns true if a equal true and b equal true else returns false
	  *				| '||'		// Returns true if a equal true or b equal true else returns false
	  *				| '^^'		// Returns true if a equal true and b equal false, or, a equal false and b equal 1.0, else returns false
	  *
	  * function1:	abs			// Calculates the absolute value
	  *				| acos		// Calculates the arccosine
	  *				| asin		// Calculates the arcsine
	  *				| atan		// Calculates the arctangent
	  *				| ceil		// Calculates the ceiling of a value ( ceil(-1.1) = -1, ceil(1.1) = 2 )
	  *				| cos		// Calculates the cosine
	  *				| cosh		// Calculates the hyperbolic cosine
	  *				| exp		// Calculates the exponential
	  *				| exponent	// Calculates the exponent of a floating point value
	  *				| floor		// Calculates the floor of a value ( floor(-1.1) = -2, floor(1.1) = 1 )
	  *				| int		// Calculates the C style integer value ( int(-1.6) = -1, int(1.6) = 1 )
	  *				| log		// Calculates logarithms
	  *				| log10		// Calculates base-10 logarithms
	  *				| mantissa	// Calculates the mantissa of a floating point value
	  *				| round		// Calculates the nearest integer value ( round(-1.6) = -2, round(1.1) = 1 )
	  *				| sin		// Calculate sines
	  *				| sinh		// Calculate hyperbolic sines
	  *				| sq		// Calculates the square
	  *				| sqrt		// Calculates the square root
	  *				| tan		// Calculates the tangent
	  *				| tanh		// Calculates the hyperbolic tangent
	  *				| string	// User defined one arg function, evaluated by the evalfunction() callback
	  *
	  * function2:	max			// Returns the larger of two values
	  *				| min		// Returns the smaller of two values
	  *				| atan2		// Calculates the arctangent of arg0/arg1
	  *				| pow		// Calculates a raised at the power of b
	  *				| rand		// Calculates a pseudo random value (arg0 <= randomValue < arg1)
	  *				| string	// User defined two args function, evaluated by the evalfunction() callback
	  *
	  * number:		[0-9]+								// Unsigned decimal integer
	  *				| "0x"[0-9a-fA-F]+					// Unsigned hexadecimal integer
	  *				| "0"[0-7]+							// Unsigned octal integer
	  *				| [0-9]*.[0-9]+						// Unsigned floating point value
	  *				| [0-9]*.[0-9]+[eE]-?[0-9]*.[0-9]+	// Unsigned floating point value with signed exponent
	  *
	  * constant:	e			// 2.7182818284590452353602874713527
	  *				| pi		// 3.1415926535897932384626433832795
	  *
	  * string:		[^ 0-9\t\n/\*-+=<>&|\^!%~\(\)\.,\"][^ \t\n/\*-+=<>&|\^!%~\(\)\.,\"]*	// Labels ($foo, #foo01, _001)
	  *				| \"[]+\"		// All kind of labels between double quotes "123456" "foo.foo[12]"
	  *
	  * Operator precedence:
	  *
	  *  0 - unary operator (-, ~, !)
	  *  1 - *, /, %
	  *	 2 - +, -,
	  *  3 - <<, >>, <-, ->
	  *  4 - <, <=, >, >=
	  *	 5 - ==, !=
	  *  6 - &
	  *	 7 - |
	  *	 8 - ^
	  *	 9 - &&
	  *	10 - ||
	  *	11 - ^^
	  *
	  */
	TReturnState evalExpression (const char *expression, double &result, int *errorIndex, uint32 userData = 0);

	/// Get error string
	const char* getErrorString (TReturnState state) const;

protected:

	/// Overridable functions

	/**
	  * Eval a user defined value. Default implementation returns UnknownValue.
	  * The user can parse the value and fill the result double and return NoError, UnknownValue or
	  * ValueError.
	  *
	  * \param value is the value to parse.
	  * \param result is the result to fill if the value has been successfully parsed.
	  * \param userData is a user data used by user eval function.
	  * \return UnknownValue if the value is not known, ValueError is the value evaluation failed or NoError
	  * if it has been parsed.
	  */
	virtual TReturnState evalValue (const char *value, double &result, uint32 userData);

	/**
	  * Eval a user defined function. Default implementation returns UnknownFunction.
	  * The user can parse the function name and fill the result double and return NoError, UnknownFunction
	  * or FunctionError.
	  *
	  * To convert double argu in boolean argu, use (round (value) != 0.0) ? true : false
	  *
	  * \param funcName is the name of the function to evaluate.
	  * \param arg0 is the first parameter passed to the function.
	  * \param arg1 is the second parameter passed to the function.
	  * \param result is the result to fill if the value has been successfully parsed.
	  * \return UnknownFunction if the function doesn't exist, FunctionError if the function evaluation
	  * failed, NoError if it has been parsed.
	  */
	virtual TReturnState evalFunction (const char *funcName, double arg0, double &result);
	virtual TReturnState evalFunction (const char *funcName, double arg0, double arg1, double &result);

private:

	/// Implementation

	/// Expression tokens
	enum TToken
	{
		Number,		// This is a number
		Function1,	// This is a function with one argu
		Function2,	// This is a function with two argu
		String,		// This is a user string
		Operator,	// This is an operator
		Open,		// (
		Close,		// )
		Coma,		// ,
		End,		// End of string
	};

	// Operators
	enum TOperator
	{
		Not = 0,		// !
		Tilde,			// ~
		Mul,			// *
		Div,			// /
		Remainder,		// %
		Plus,			// +
		Minus,			// -
		ULeftShift,		// <<
		URightShift,	// >>
		SLeftShift,		// <-
		SRightShift,	// ->
		Inferior,		// <
		InferiorEqual,	// <=
		Superior,		// >
		SuperiorEqual,	// >=
		Equal,			// ==
		NotEqual,		// !=
		And,			// &
		Or,				// |
		Xor,			// ^
		LogicalAnd,		// &&
		LogicalOr,		// ||
		LogicalXor,		// ^^
		OperatorCount,	//
		NotOperator, // This is not an operator
		ExtOperator, // This is a 2 characters operator
	};

	// Functions
	enum TReservedWord
	{
		Abs = 0,
		Acos,
		Asin,
		Atan,
		Atan2,
		Ceil,
		Cos,
		Cosh,
		Exp,
		Exponent,
		Floor,
		Int,
		Log,
		Log10,
		Mantissa,
		Max,
		Min,
		Pow,
		Rand,
		Round,
		Sin,
		Sinh,
		Sq,
		Sqrt,
		Tan,
		Tanh,
		ReservedWordCount,
	};

	// Some constant
	enum
	{
		InternalStringLen = 32,
		InternalOperator = 4,
	};

	/// Members
	TReturnState	_State;		// Current state
	const char		*_ExprPtr;	// Current pointer on the expression

	/// Read a decimal double
	TReturnState readDecimal (double &value);

	/// Read an integer
	void readIntegerNumberDecimal (double &value);

	/// Internal functions

	/// Get the next token
	TReturnState	getNextToken (TToken &token);

	/// Evaluate an expression
	TReturnState	evalExpression (double &result, TToken &nextToken, uint32 userData);

	/// Reserved word
	TReservedWord	_ReservedWordFound;

	/// Current string
	char			_InternalString[InternalStringLen];
	std::string		_InternalStlString;
	const char		*_InternalStringPtr;

	/// Current value
	double			_Value;

	/// Current operator
	TOperator		_Op;

	/// Static values

	/// Char to operator array
	static const TToken		_ReservedWordToken[ReservedWordCount];
	static const char		*_ReservedWord[ReservedWordCount];
	static const TOperator	_OperatorArray[128];
	static const bool		_StringChar[128];
	static const char		*_ErrorString[ReturnValueCount];
	static const int		_OperatorPrecedence[];

public:

	bool internalCheck ();
};

}

#endif // NL_EVAL_NUM_EXPR_H

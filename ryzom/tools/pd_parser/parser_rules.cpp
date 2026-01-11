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



#include "parser_rules.h"

/*
 * Main Bloc
 */
PARSE_START_NO_DESCRIPTION(Main, CParseNode)

	while (!tokenizer.end())
	{
		PARSE_MARK
		PARSE_ALTERNATIVE(Db)
		PARSE_FAIL
	}

PARSE_END


/*
 * Db Bloc
 */
PARSE_START(Db, CDbNode)

	PARSE_KEYWORD(Db)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)
	PARSE_MARK

	PARSE_OPT_KEYWORD_IN(String, MainFile);

	PARSE_START_BLOC

		PARSE_MARK

		PARSE_ALTERNATIVE(File)
		PARSE_ALTERNATIVE(UsePch)
		PARSE_FAIL

		PARSE_MARK

	PARSE_END_BLOC

	PARSE_MARK

PARSE_END


/*
 * File Bloc
 */
PARSE_START(File, CFileNode)

	PARSE_OPT_BEFORE(File)
		PARSE_OPT_IN(Separated, SeparatedFlag);
	PARSE_END_OPT_BEFORE()

	PARSE_KEYWORD(File)
	PARSE_MARK
	PARSE_STRING(Name)
	PARSE_MARK

	main->IncludeAs = main->Name;

	PARSE_OPT_KEYWORD_IN(String, IncludeAs);

	PARSE_START_BLOC

		PARSE_MARK

		PARSE_ALTERNATIVE(Include)
		PARSE_ALTERNATIVE(Type)
		PARSE_ALTERNATIVE(Class)
		PARSE_ALTERNATIVE(Enum)
		PARSE_ALTERNATIVE(Dimension)
		//PARSE_ALTERNATIVE(CppCode)
		PARSE_ALTERNATIVE(LogMsg)
		PARSE_ALTERNATIVE(LogContext)
		PARSE_FAIL

		PARSE_MARK

	PARSE_END_BLOC

	PARSE_MARK

PARSE_END


/*
 * Include Bloc
 */
PARSE_START(Include, CIncludeNode)

	PARSE_KEYWORD(Include)
	PARSE_MARK
	PARSE_STRING(Name)
	PARSE_MARK

PARSE_END

/*
 * Include Bloc
 */
PARSE_START(UsePch, CUsePchNode)

	PARSE_KEYWORD(UsePch)
	PARSE_MARK
	PARSE_STRING(Name)
	PARSE_MARK

PARSE_END


/*
 * Cpp Code Bloc
 */
PARSE_START(CppCode, CCppCodeNode)

	PARSE_OPT_KEYWORD_IN(Identifier, Name)
	PARSE_KEYWORD_IN(CppCode, RawCode)
	PARSE_MARK

PARSE_END


/*
 * Type Bloc
 */
PARSE_START(Type, CTypeNode)

	PARSE_OPT_BEFORE(Type)
		PARSE_OPT_IN(Extern, ExternFlag);
	PARSE_END_OPT_BEFORE()

	PARSE_KEYWORD(Type)
	PARSE_MARK
	if (main->ExternFlag)
	{
		PARSE_SCOPE_IDF(Name)
	}
	else
	{
		PARSE_IDENTIFIER(Name)
	}

	main->DisplayName = main->Name;

	PARSE_MARK
	if (tokenizer.current() == TokenOpenParenthesis)
	{
		tokenizer.next();
		PARSE_MARK
		PARSE_IDENTIFIER(CppType)
		PARSE_MARK
		PARSE_KEYWORD(Comma)
		PARSE_MARK
		PARSE_IDENTIFIER(StorageType)
		PARSE_MARK
		PARSE_KEYWORD(CloseParenthesis)
		PARSE_MARK

		if (tokenizer.current() == TokenOpenBrace)
		{
			tokenizer.next();

			PARSE_MARK
			PARSE_IDENTIFIER(Temp)
			if (main->Temp == main->CppType)
			{
				PARSE_MARK
				PARSE_KEYWORD(OpenParenthesis)
				PARSE_MARK
				PARSE_IDENTIFIER(Temp)
				if (main->Temp != main->StorageType)
					PARSE_FAIL
				PARSE_MARK
				PARSE_KEYWORD(CloseParenthesis)
				PARSE_MARK
				PARSE_NODE(CppCode, ToCppType)
				PARSE_MARK
			}
			else if (main->Temp == main->StorageType)
			{
				PARSE_MARK
				PARSE_KEYWORD(OpenParenthesis)
				PARSE_MARK
				PARSE_IDENTIFIER(Temp)
				if (main->Temp != main->CppType)
					PARSE_FAIL
				PARSE_MARK
				PARSE_KEYWORD(CloseParenthesis)
				PARSE_MARK
				PARSE_NODE(CppCode, ToStorageType)
				PARSE_MARK
			}
			else
				PARSE_FAIL

			if (tokenizer.current() != TokenCloseBrace)
			{
				PARSE_IDENTIFIER(Temp)
				PARSE_MARK
				if (main->Temp == main->CppType && main->ToCppType == NULL)
				{
					PARSE_MARK
					PARSE_KEYWORD(OpenParenthesis)
					PARSE_MARK
					PARSE_IDENTIFIER(Temp)
					if (main->Temp != main->StorageType)
						PARSE_FAIL
					PARSE_MARK
					PARSE_KEYWORD(CloseParenthesis)
					PARSE_MARK
					PARSE_NODE(CppCode, ToCppType)
					PARSE_MARK
				}
				else if (main->Temp == main->StorageType && main->ToStorageType == NULL)
				{
					PARSE_MARK
					PARSE_KEYWORD(OpenParenthesis)
					PARSE_MARK
					PARSE_IDENTIFIER(Temp)
					if (main->Temp != main->CppType)
						PARSE_FAIL
					PARSE_MARK
					PARSE_KEYWORD(CloseParenthesis)
					PARSE_MARK
					PARSE_NODE(CppCode, ToStorageType)
					PARSE_MARK
				}
				else
					PARSE_FAIL
			}

			PARSE_KEYWORD(CloseBrace)
		}
		else
		{
			PARSE_KEYWORD(SemiColon)
		}
	}
	else
	{
		PARSE_IDENTIFIER(CppType)
		main->StorageType = main->CppType;
		PARSE_MARK
		PARSE_KEYWORD(SemiColon)
	}

	PARSE_MARK

PARSE_END


/*
 * Class Bloc
 */
PARSE_START(Class, CClassNode)

	PARSE_OPT_BEFORE(Class)
		PARSE_OPT_IN(Mapped, MappedFlag);
		PARSE_OPT_IN(Derived, DerivatedFlag);
	PARSE_END_OPT_BEFORE()

	PARSE_KEYWORD(Class)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)
	PARSE_MARK

	PARSE_OPT(Colon)
		PARSE_MARK
		PARSE_IDENTIFIER(Inherited)
		PARSE_MARK
	PARSE_END_OPT

	PARSE_OPT(Key)
		PARSE_MARK
		PARSE_KEYWORD(OpenParenthesis)
		PARSE_MARK
		PARSE_IDENTIFIER(ClassKey)
		PARSE_MARK
		PARSE_KEYWORD(CloseParenthesis)
		PARSE_MARK
	PARSE_END_OPT

	PARSE_OPT(Implements)
		PARSE_MARK
		PARSE_IDENTIFIER(Implements)
		PARSE_MARK
	PARSE_END_OPT

	PARSE_OPT(Reserve)
		PARSE_MARK
		PARSE_KEYWORD(OpenParenthesis)
		PARSE_MARK
		PARSE_IDENTIFIER(Reserve)
		PARSE_MARK
		PARSE_KEYWORD(CloseParenthesis)
		PARSE_MARK
	PARSE_END_OPT

	PARSE_START_BLOC

		PARSE_MARK

		PARSE_ALTERNATIVE(CppCode)
		PARSE_ALTERNATIVE(Declaration)
		PARSE_FAIL

		PARSE_MARK

	PARSE_END_BLOC

	PARSE_MARK

PARSE_END


/*
 * Declaration Bloc
 */
PARSE_START(Declaration, CDeclarationNode)

	PARSE_OPT_BEFORE_2(Identifier, ScopedIdentifier)
		PARSE_OPT_IN(WriteTrigger, WriteTriggerFlag);
		PARSE_OPT_IN(InitFill, InitFillFlag);
		PARSE_OPT_IN(Parent, ParentFlag);
		PARSE_OPT_IN(Hidden, HiddenFlag);
		PARSE_OPT_IN(Mirrored, MirroredFlag);
	PARSE_END_OPT_BEFORE()

	if (main->ParentFlag)
	{
		PARSE_IDENTIFIER(ParentClass);
		PARSE_MARK
		PARSE_KEYWORD(Colon)
		PARSE_MARK
		PARSE_IDENTIFIER(ParentField);
		PARSE_MARK
		PARSE_IDENTIFIER(Name);
		PARSE_MARK
	}
	else
	{
		PARSE_SCOPE_IDF(Type);
		PARSE_MARK

		PARSE_OPT(Colon)
			PARSE_MARK
			PARSE_IDENTIFIER(ForwardRefAttribute);
			PARSE_MARK
		PARSE_END_OPT

		PARSE_OPT(OpenBracket)
			PARSE_MARK
			PARSE_IDENTIFIER(ArrayIndex);
			PARSE_MARK
			main->ArrayFlag = true;
			PARSE_KEYWORD(CloseBracket)
			PARSE_MARK

			PARSE_IDENTIFIER(Name);
			PARSE_MARK

		PARSE_NEXT_OPT(LessThan)
			main->SetFlag = true;
			PARSE_MARK
			PARSE_KEYWORD(GreaterThan)
			PARSE_MARK

			PARSE_IDENTIFIER(Name);
			PARSE_MARK

		PARSE_LAST_OPT

			PARSE_IDENTIFIER(Name);
			PARSE_MARK

			PARSE_OPT(Equal)
				//PARSE_VALUE(DefaultValue)
				PARSE_KEYWORD_IN(CppCode, DefaultValue)
			PARSE_END_OPT

		PARSE_END_OPT
	}

	if (tokenizer.current() == TokenOpenBrace)
	{
		PARSE_START_BLOC
			PARSE_MARK

			CDeclarationNode::CUserCode	UCode;

			PARSE_KEYWORD_IN_TEMP(Identifier, UCode.Event)

			PARSE_OPT_LIST(OpenParenthesis, CloseParenthesis)

				std::string	UCodeAtom;
				PARSE_KEYWORD_IN_TEMP(Identifier, UCodeAtom)
				if (!UCode.CodeSpecializer.empty())
					UCode.CodeSpecializer += ".";
				UCode.CodeSpecializer += UCodeAtom;

			PARSE_END_OPT_LIST(CloseParenthesis, Dot)

			PARSE_KEYWORD_IN_TEMP(CppCode, UCode.UserCode)

			main->UserCodes.push_back(UCode);

			PARSE_MARK
		PARSE_END_BLOC
	}
	else
	{
		PARSE_KEYWORD(SemiColon);
	}

	PARSE_MARK

PARSE_END


/*
 * Enum Bloc
 */
PARSE_START(Enum, CEnumNode)

	PARSE_KEYWORD(Enum)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)

	PARSE_START_LIST(OpenBrace, CloseBrace)

		PARSE_MARK

		PARSE_ALTERNATIVE(EnumRange)
		PARSE_ALTERNATIVE(EnumSimpleValue)
		PARSE_FAIL

		PARSE_MARK

	PARSE_END_LIST(CloseBrace, Comma)

	PARSE_OPT_KEYWORD_IN(Identifier, EndRange)

	PARSE_MARK

PARSE_END

//
PARSE_START(EnumSimpleValue, CEnumSimpleValueNode)

	PARSE_LIST_KEYWORDS(Identifier, Names, Or)

PARSE_END

//
PARSE_START(EnumRange, CEnumRangeNode)

	PARSE_IDENTIFIER(Name)

	PARSE_START_LIST(OpenBrace, CloseBrace)

		PARSE_MARK

		PARSE_ALTERNATIVE(EnumRange)
		PARSE_ALTERNATIVE(EnumSimpleValue)
		PARSE_FAIL

		PARSE_MARK

	PARSE_END_LIST(CloseBrace, Comma)
	PARSE_MARK

	PARSE_OPT_KEYWORD_IN(Identifier, EndRange)

	PARSE_MARK

PARSE_END


/*
 * Dimension Bloc
 */
PARSE_START(Dimension, CDimensionNode)

	PARSE_KEYWORD(Dimension)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)
	PARSE_MARK
	PARSE_INT(Dimension)
	PARSE_MARK
	PARSE_KEYWORD(SemiColon);
	PARSE_MARK

PARSE_END


/*
 * LogMsg Bloc
 */
PARSE_START(LogMsg, CLogMsgNode)

	PARSE_KEYWORD(LogMsg)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)
	PARSE_MARK

	PARSE_KEYWORD(OpenParenthesis)
	PARSE_MARK
	PARSE_LIST_2_KEYWORDS(Identifier, Identifier, Params, Comma)
	PARSE_KEYWORD(CloseParenthesis)
	PARSE_MARK

	PARSE_KEYWORD(OpenBrace)
	PARSE_MARK
	PARSE_LIST_KEYWORDS(String, Logs, Comma)
	PARSE_MARK

	PARSE_KEYWORD(CloseBrace)
	PARSE_MARK

PARSE_END

PARSE_START(LogContext, CLogMsgNode)

	PARSE_KEYWORD(LogContext)
	PARSE_MARK
	PARSE_IDENTIFIER(Name)
	PARSE_MARK

	main->Context = true;

	PARSE_KEYWORD(OpenParenthesis)
	PARSE_MARK
	PARSE_LIST_2_KEYWORDS(Identifier, Identifier, Params, Comma)
	PARSE_KEYWORD(CloseParenthesis)
	PARSE_MARK

	PARSE_KEYWORD(OpenBrace)
	PARSE_MARK
	PARSE_LIST_KEYWORDS(String, Logs, Comma)
	PARSE_MARK

	PARSE_KEYWORD(CloseBrace)
	PARSE_MARK

PARSE_END


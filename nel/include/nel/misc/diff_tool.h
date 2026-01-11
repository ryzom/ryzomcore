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

#ifndef DIFF_TOOL_H
#define DIFF_TOOL_H

#include "i18n.h"

namespace STRING_MANAGER
{
	const ucstring		nl("\n");


	struct TStringInfo
	{
		std::string			Identifier;
		ucstring			Text;
		ucstring			Text2;
		mutable ucstring	Comments;
		uint64				HashValue;
	};

	struct TStringDiffContext
	{
		typedef std::vector<TStringInfo>::iterator	iterator;
		const std::vector<TStringInfo>	&Addition;
		std::vector<TStringInfo>		&Reference;
		std::vector<TStringInfo>		&Diff;

		TStringDiffContext(const std::vector<TStringInfo> &addition, std::vector<TStringInfo> &reference, std::vector<TStringInfo> &diff)
			: Addition(addition),
			Reference(reference),
			Diff(diff)
		{
		}
	};

	struct TClause
	{
		std::string	Identifier;
		ucstring	Conditions;
		ucstring	Text;
		ucstring	Comments;
		uint64		HashValue;
	};

	struct TPhrase
	{
		std::string				Identifier;
		ucstring				Parameters;
		mutable ucstring		Comments;
		std::vector<TClause>	Clauses;
		uint64					HashValue;
	};

	struct TPhraseDiffContext
	{
		typedef	std::vector<TPhrase>::iterator iterator;
		const std::vector<TPhrase>	&Addition;
		std::vector<TPhrase>			&Reference;
		std::vector<TPhrase>			&Diff;

		TPhraseDiffContext(const std::vector<TPhrase> &addition, std::vector<TPhrase> &reference, std::vector<TPhrase> &diff)
			: Addition(addition),
			Reference(reference),
			Diff(diff)
		{
		}
	};

	struct TWorksheet
	{
		typedef std::vector<ucstring>	TRow;
		typedef std::vector<TRow>		TData;
		TData	Data;
		uint	ColCount;

		TWorksheet()
			: ColCount(0)
		{
		}

		std::vector<TRow>::iterator		begin()
		{
			return Data.begin();
		}

		std::vector<TRow>::iterator		end()
		{
			return Data.end();
		}

		std::vector<TRow>::const_iterator		begin() const
		{
			return Data.begin();
		}

		std::vector<TRow>::const_iterator		end() const
		{
			return Data.end();
		}

		void push_back(const TRow &row)
		{
			Data.push_back(row);
		}

		std::vector<TRow>::iterator insert(std::vector<TRow>::iterator pos, const TRow &value)
		{
			return Data.insert(pos, value);
		}

		std::vector<TRow>::iterator erase(std::vector<TRow>::iterator it)
		{
			return Data.erase(it);
		}

		TRow &back()
		{
			return Data.back();
		}

		TRow &operator [] (uint index)
		{
			return Data[index];
		}

		const TRow &operator [] (uint index) const
		{
			return Data[index];
		}

		uint size() const
		{
			return (uint)Data.size();
		}

		void insertColumn(uint colIndex)
		{
			nlassert(colIndex <= ColCount);

			for (uint i=0; i<Data.size(); ++i)
			{
				// insert a default value.
				Data[i].insert(Data[i].begin()+colIndex, ucstring());
			}
			ColCount++;
		}

		void copyColumn(uint srcColIndex, uint dstColIndex)
		{
			nlassert(srcColIndex < ColCount);
			nlassert(dstColIndex < ColCount);

			for (uint i=0; i<Data.size(); ++i)
			{
				Data[i][dstColIndex] = Data[i][srcColIndex];
			}
		}

		void eraseColumn(uint colIndex)
		{
			nlassertex(colIndex < ColCount, ("TWorksheet::eraseColumn : bad column index: colIndex(%u) is not less than ColCount(%u)", colIndex, ColCount));

			for (uint i=0; i<Data.size(); ++i)
			{
				// insert a default value.
				Data[i].erase(Data[i].begin()+colIndex);
			}
			ColCount--;
		}

		void moveColumn(uint oldColIndex, uint newColIndex)
		{
			nlassert(oldColIndex < ColCount);
			nlassert(newColIndex < ColCount);

			if (oldColIndex == newColIndex)
				return;

			if (newColIndex > oldColIndex)
			{
				// the dst is after the src, no problem with index
				insertColumn(newColIndex);
				copyColumn(oldColIndex, newColIndex);
				eraseColumn(oldColIndex);
			}
			else
			{
				// the dst is before the src, need to take the column insertion into account
				insertColumn(newColIndex);
				copyColumn(oldColIndex+1, newColIndex);
				eraseColumn(oldColIndex+1);
			}
		}

		void setColCount(uint count)
		{
			if (count != ColCount)
			{
				for (uint i=0; i<Data.size(); ++i)
					Data[i].resize(count);
			}
			ColCount = count;
		}

		bool findId(uint& colIndex)
		{
			if (Data.empty())
				return false;

			for (TWorksheet::TRow::iterator it=Data[0].begin(); it!=Data[0].end(); ++it)
			{
				std::string columnTitle = (*it).toString();
				if ( ! columnTitle.empty() )
				{
					// Return the first column for which the title does not begin with '*'
					if ( columnTitle[0] != '*' )
					{
						colIndex = (uint)(it - Data[0].begin());
						return true;
					}
				}
			}
			return false;
		}

		bool findCol(const ucstring &colName, uint &colIndex)
		{
			if (Data.empty())
				return false;
			TWorksheet::TRow::iterator it = std::find(Data[0].begin(), Data[0].end(), colName);
			if (it == Data[0].end())
				return false;

			colIndex = (uint)(it - Data[0].begin());
			return true;
		}

		void insertRow(uint rowIndex, const TRow &row)
		{
			nlassertex(rowIndex <= Data.size(), ("TWorksheet::insertRow: bad row index: rowIndex(%u) is out of range (max=%u)", rowIndex, Data.size()-1));
			nlassertex(row.size() == ColCount, ("TWorksheet::insertRow: bad column count : inserted row size(%u) is invalid (must be %u) at rowIndex(%u)", row.size(), ColCount, rowIndex));

			Data.insert(Data.begin()+rowIndex, row);
		}

		// resize the rows
		void resize(uint numRows)
		{
			uint	oldSize= (uint)Data.size();
			Data.resize(numRows);
			// alloc good Column count for new lines
			for(uint i= oldSize;i<Data.size();i++)
				Data[i].resize(ColCount);
		}

		bool findRow(uint colIndex, const ucstring &colValue, uint &rowIndex)
		{
			nlassertex(colIndex < ColCount, ("TWorksheet::findRow: bad column index: colIndex(%u) is not less than ColCount(%u)", colIndex, ColCount));

			TData::iterator first(Data.begin()), last(Data.end());

			for (; first != last; ++first)
			{
				if (first->operator[](colIndex) == colValue)
				{
					rowIndex = (uint)(first - Data.begin());
					return true;
				}

			}
			return false;
		}

		void setData(uint rowIndex, uint colIndex, const ucstring &value)
		{
			nlassertex(rowIndex < Data.size(), ("TWorksheet::setData: bad row index: rowIndex(%u) is out of range (max=%u)", rowIndex, Data.size()));
			nlassertex(colIndex < ColCount, ("TWorksheet::setData: bad column index: colIndex(%u) is not less than ColCount(%u) ar rowIndex(%u)", colIndex, ColCount, rowIndex));

			Data[rowIndex][colIndex] = value;
		}

		const ucstring &getData(uint rowIndex, uint colIndex) const
		{
			nlassertex(rowIndex < Data.size(), ("TWorksheet::getData: bad row index: rowIndex(%u) is out of range (max=%u)", rowIndex, Data.size()));
			nlassertex(colIndex < ColCount, ("TWorksheet::getData: bad column index: colIndex(%u) is not less than ColCount(%u) at rowIndex(%u)", colIndex, ColCount, rowIndex));

			return Data[rowIndex][colIndex];
		}

		void setData(uint rowIndex, const ucstring &colName, const ucstring &value)
		{
			nlassertex(rowIndex > 0, ("TWorksheet::setData: rowIndex(%u) must be greater then 0 !", rowIndex));
			nlassertex(rowIndex < Data.size(), ("TWorksheet::setData: rowIndex(%u) is out of range (max=%u)", rowIndex, Data.size()));
			TWorksheet::TRow::iterator it = std::find(Data[0].begin(), Data[0].end(), ucstring(colName));
			nlassertex(it != Data[0].end(), ("TWorksheet::setData: invalid colName: can't find the column named '%s' at row %u", colName.toString().c_str(), rowIndex));

			Data[rowIndex][it - Data[0].begin()] = value;
		}
		const ucstring &getData(uint rowIndex, const ucstring &colName) const
		{
			nlassertex(rowIndex > 0, ("TWorksheet::getData: bad row index: rowIndex(%u) must be greater then 0 !", rowIndex));
			nlassertex(rowIndex < Data.size(), ("TWorksheet::getData: bad row index: rowIndex(%u) is out of range (max=%u)", rowIndex, Data.size()));
			TWorksheet::TRow::const_iterator it = std::find(Data[0].begin(), Data[0].end(), ucstring(colName));
			nlassertex(it != Data[0].end(), ("TWorksheet::getData: invalid colName: can't find the column named '%s' at row %u", colName.toString().c_str(), rowIndex));

			return Data[rowIndex][it - Data[0].begin()];
		}
	};


	struct TGetWorksheetIdentifier
	{
		std::string operator()(const TWorksheet &container, uint index) const
		{
			return container.getData(index, 1).toString();
		}
	};

	struct TGetWorksheetHashValue
	{
		uint64 operator()(const TWorksheet &container, uint index) const
		{
			return NLMISC::CI18N::stringToHash(container.getData(index, ucstring("*HASH_VALUE")).toString());
		}
	};

	struct TTestWorksheetItem : public std::unary_function<TWorksheet::TRow, bool>
	{
		ucstring	Identifier;
		TTestWorksheetItem(const std::string &identifier)
			: Identifier(identifier)
		{}
		bool operator () (const TWorksheet::TRow &row) const
		{
			return row[1] == Identifier;
		}
	};


	struct TWordsDiffContext
	{
		typedef	TWorksheet::TData::iterator iterator;
		const TWorksheet	&Addition;
		TWorksheet			&Reference;
		TWorksheet			&Diff;

		TWordsDiffContext(const TWorksheet &addition, TWorksheet &reference, TWorksheet &diff)
			: Addition(addition),
			Reference(reference),
			Diff(diff)
		{
		}
	};

	template<class ItemType>
	struct TGetIdentifier
	{
		std::string operator()(const std::vector<ItemType> &container, uint index) const
		{
			return container[index].Identifier;
		}
	};

	template<class ItemType>
	struct TGetHashValue
	{
		uint64 operator()(const std::vector<ItemType> &container, uint index) const
		{
			return container[index].HashValue;
		}
	};

	template<class ItemType>
	struct TTestItem : public std::unary_function<ItemType, bool>
	{
		std::string	Identifier;
		TTestItem(const std::string &identifier)
			: Identifier(identifier)
		{}
		bool operator () (const ItemType &item) const
		{
			return item.Identifier == Identifier;
		}
	};

	/**
	*	ItemType must have a property named Identifier that uniquely
	*	identify each item.
	*	ItemType must have a property named HashValue that is used
	*	to determine the change between context.Addition and context.Reference vector.
	*/
	template <class ItemType, class Context, class GetIdentifier = TGetIdentifier<ItemType>, class GetHashValue = TGetHashValue<ItemType>, class TestItem = TTestItem<ItemType> >
	class CMakeDiff
	{
	public:
		struct IDiffCallback
		{
			virtual void onEquivalent(uint addIndex, uint refIndex, Context &context) = 0;
			virtual void onAdd(uint addIndex, uint refIndex, Context &context) = 0;
			virtual void onRemove(uint addIndex, uint refIndex, Context &context) = 0;
			virtual void onChanged(uint addIndex, uint refIndex, Context &context) = 0;
			virtual void onSwap(uint newIndex, uint refIndex, Context &context) = 0;

		};

		void makeDiff(IDiffCallback *callback, Context &context, bool skipFirstRecord = false)
		{
#ifdef NL_DEBUG
			// compile time checking
//			Context::iterator testIt;
#endif
			GetIdentifier	getIdentifier;
			GetHashValue	getHashValue;
			// compare the context.Reference an context.Addition file, remove any equivalent strings.
			uint addCount, refCount;
			if (skipFirstRecord)
			{
				addCount = 1;
				refCount = 1;
			}
			else
			{
				addCount = 0;
				refCount=0;
			}

			while (addCount < context.Addition.size() || refCount < context.Reference.size())
			{
				bool equal = true;
				if (addCount != context.Addition.size() && refCount != context.Reference.size())
				{
					equal = getHashValue(context.Addition, addCount) == getHashValue(context.Reference, refCount);
				}

//				vector<ItemType>::iterator it;

				if (addCount == context.Addition.size()
					||
						(
							!equal
						&&	find_if(context.Addition.begin(), context.Addition.end(), TestItem(getIdentifier(context.Reference, refCount))) == context.Addition.end()
						)
					)
				{
					// this can only be removal
					callback->onRemove(addCount, refCount, context);
					context.Reference.erase(context.Reference.begin()+refCount);
//					++refCount;
				}
				else if (refCount == context.Reference.size()
					||
						(
							!equal
						&&	find_if(context.Reference.begin(), context.Reference.end(), TestItem(getIdentifier(context.Addition, addCount))) == context.Reference.end()
						)
					)
				{
					// this can only be context.Addition
					callback->onAdd(addCount, refCount, context);
					context.Reference.insert(context.Reference.begin()+refCount, context.Addition[addCount]);
					++refCount;
					++addCount;
				}
				else if (getIdentifier(context.Addition, addCount) != getIdentifier(context.Reference, refCount))
				{
					// swap two element.
//					Context::iterator it = find_if(context.Reference.begin(), context.Reference.end(), TestItem(getIdentifier(context.Addition, addCount)));
//					if (it == context.Reference.end())

					if (find_if(
							context.Reference.begin(),
							context.Reference.end(),
							TestItem(getIdentifier(context.Addition, addCount)))
							== context.Reference.end())
					{
						// context.Addition
						callback->onAdd(addCount, refCount, context);
						context.Reference.insert(context.Reference.begin()+refCount, context.Addition[addCount]);
						++refCount;
						++addCount;
					}
					else
					{
//						nlassert(it != context.Reference.begin()+refCount);
						uint index = (uint)(find_if(context.Reference.begin(), context.Reference.end(), TestItem(getIdentifier(context.Addition, addCount))) - context.Reference.begin());

//						callback->onSwap(it - context.Reference.begin(), refCount, context);
						callback->onSwap(index, refCount, context);
//						std::swap(*it, context.Reference[refCount]);
						std::swap(context.Reference[index], context.Reference[refCount]);
					}
				}
				else if (getHashValue(context.Addition, addCount) != getHashValue(context.Reference, refCount))
				{
					// changed element
					callback->onChanged(addCount, refCount, context);
					++refCount;
					++addCount;
				}
				else
				{
					// same entry
					callback->onEquivalent(addCount, refCount, context);
					addCount++;
					refCount++;
				}
			}
		}
	};

	typedef CMakeDiff<TStringInfo, TStringDiffContext>		TStringDiff;
	typedef CMakeDiff<TPhrase, TPhraseDiffContext>			TPhraseDiff;
	typedef CMakeDiff<TWorksheet::TRow, TWordsDiffContext, TGetWorksheetIdentifier, TGetWorksheetHashValue, TTestWorksheetItem>	TWorkSheetDiff;


	uint64		makePhraseHash(const TPhrase &phrase);
	bool		parseHashFromComment(const ucstring &comments, uint64 &hashValue);

	bool		loadStringFile(const std::string filename, std::vector<TStringInfo> &stringInfos, bool forceRehash, ucchar openMark = '[', ucchar closeMark = ']', bool specialCase = false);
	ucstring	prepareStringFile(const std::vector<TStringInfo> &strings, bool removeDiffComments, bool noDiffInfo = false);

	bool		readPhraseFile(const std::string &filename, std::vector<TPhrase> &phrases, bool forceRehash);
	bool		readPhraseFileFromString(ucstring const& doc, const std::string &filename, std::vector<TPhrase> &phrases, bool forceRehash);
	ucstring	tabLines(uint nbTab, const ucstring &str);
	ucstring	preparePhraseFile(const std::vector<TPhrase> &phrases, bool removeDiffComments);

	bool		loadExcelSheet(const std::string filename, TWorksheet &worksheet, bool checkUnique = true);
	bool		readExcelSheet(const ucstring &text, TWorksheet &worksheet, bool checkUnique = true);
	void		makeHashCode(TWorksheet &sheet, bool forceRehash);
	ucstring	prepareExcelSheet(const TWorksheet &worksheet);

}	// namespace STRING_MANAGER

#endif // DIFF_TOOL_H

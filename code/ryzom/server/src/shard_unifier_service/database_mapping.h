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

/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#ifndef DATABASE_MAPPING
#define DATABASE_MAPPING
#include "nel/misc/types_nl.h"
#include <memory>
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"

#include "nel/misc/string_common.h"
#include "server_share/mysql_wrapper.h"

#include "ring_session_manager.h"
	
#include "game_share/ring_session_manager_itf.h"
	
#include "game_share/character_sync_itf.h"
	
namespace RSMGR
{
	
	class CKnownUser;

	class CKnownUserPtr;
	class CSessionParticipant;

	class CSessionParticipantPtr;
	class CCharacter;

	class CCharacterPtr;
	class CRingUser;

	class CRingUserPtr;
	class CSession;

	class CSessionPtr;
	class CShard;

	class CShardPtr;
	class CGuild;

	class CGuildPtr;
	class CGuildInvite;

	class CGuildInvitePtr;
	class CPlayerRating;

	class CPlayerRatingPtr;
	class CJournalEntry;

	class CJournalEntryPtr;
	class CFolder;

	class CFolderPtr;
	class CFolderAccess;

	class CFolderAccessPtr;
	class CScenario;

	class CScenarioPtr;
	class CSessionLog;

	class CSessionLogPtr;
	class CGmStatus;

	class CGmStatusPtr;



	class CKnownUserPtr
	{
		friend class CKnownUser;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CKnownUserPtr	*_NextPtr;
		CKnownUserPtr	*_PrevPtr;

		CKnownUser	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CKnownUserPtr()
			: _FileName(NULL),
			_LineNum(0),
			_NextPtr(NULL),
			_PrevPtr(NULL),
			_Ptr(NULL)
		{
		}

		CKnownUserPtr(const CKnownUserPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CKnownUserPtr(const CKnownUserPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CKnownUserPtr(CKnownUser *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CKnownUserPtr &assign(const CKnownUserPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CKnownUserPtr()
		{
			unlinkPtr();
		}

		CKnownUserPtr &assign(CKnownUser *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CKnownUserPtr &operator =(const CKnownUserPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CKnownUser *operator ->()
		{
			return _Ptr;
		}
		const CKnownUser *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CKnownUserPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CKnownUserPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CKnownUser *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CKnownUser *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CKnownUserPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CKnownUserPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CSessionParticipantPtr
	{
		friend class CSessionParticipant;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CSessionParticipantPtr	*_NextPtr;
		CSessionParticipantPtr	*_PrevPtr;

		CSessionParticipant	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CSessionParticipantPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CSessionParticipantPtr(const CSessionParticipantPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionParticipantPtr(const CSessionParticipantPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionParticipantPtr(CSessionParticipant *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CSessionParticipantPtr &assign(const CSessionParticipantPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CSessionParticipantPtr()
		{
			unlinkPtr();
		}

		CSessionParticipantPtr &assign(CSessionParticipant *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CSessionParticipantPtr &operator =(const CSessionParticipantPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CSessionParticipant *operator ->()
		{
			return _Ptr;
		}
		const CSessionParticipant *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CSessionParticipantPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CSessionParticipantPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CSessionParticipant *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CSessionParticipant *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CSessionParticipantPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CSessionParticipantPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CCharacterPtr
	{
		friend class CCharacter;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CCharacterPtr	*_NextPtr;
		CCharacterPtr	*_PrevPtr;

		CCharacter	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CCharacterPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CCharacterPtr(const CCharacterPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CCharacterPtr(const CCharacterPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CCharacterPtr(CCharacter *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CCharacterPtr &assign(const CCharacterPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CCharacterPtr()
		{
			unlinkPtr();
		}

		CCharacterPtr &assign(CCharacter *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CCharacterPtr &operator =(const CCharacterPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CCharacter *operator ->()
		{
			return _Ptr;
		}
		const CCharacter *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CCharacterPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CCharacterPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CCharacter *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CCharacter *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CCharacterPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CCharacterPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CRingUserPtr
	{
		friend class CRingUser;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CRingUserPtr	*_NextPtr;
		CRingUserPtr	*_PrevPtr;

		CRingUser	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CRingUserPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CRingUserPtr(const CRingUserPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CRingUserPtr(const CRingUserPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CRingUserPtr(CRingUser *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CRingUserPtr &assign(const CRingUserPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CRingUserPtr()
		{
			unlinkPtr();
		}

		CRingUserPtr &assign(CRingUser *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CRingUserPtr &operator =(const CRingUserPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CRingUser *operator ->()
		{
			return _Ptr;
		}
		const CRingUser *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CRingUserPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CRingUserPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CRingUser *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CRingUser *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CRingUserPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CRingUserPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CSessionPtr
	{
		friend class CSession;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CSessionPtr	*_NextPtr;
		CSessionPtr	*_PrevPtr;

		CSession	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CSessionPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CSessionPtr(const CSessionPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionPtr(const CSessionPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionPtr(CSession *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CSessionPtr &assign(const CSessionPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CSessionPtr()
		{
			unlinkPtr();
		}

		CSessionPtr &assign(CSession *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CSessionPtr &operator =(const CSessionPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CSession *operator ->()
		{
			return _Ptr;
		}
		const CSession *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CSessionPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CSessionPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CSession *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CSession *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CSessionPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CSessionPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CShardPtr
	{
		friend class CShard;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CShardPtr	*_NextPtr;
		CShardPtr	*_PrevPtr;

		CShard	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CShardPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CShardPtr(const CShardPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CShardPtr(const CShardPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CShardPtr(CShard *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CShardPtr &assign(const CShardPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CShardPtr()
		{
			unlinkPtr();
		}

		CShardPtr &assign(CShard *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CShardPtr &operator =(const CShardPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CShard *operator ->()
		{
			return _Ptr;
		}
		const CShard *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CShardPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CShardPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CShard *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CShard *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CShardPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CShardPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CGuildPtr
	{
		friend class CGuild;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CGuildPtr	*_NextPtr;
		CGuildPtr	*_PrevPtr;

		CGuild	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CGuildPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CGuildPtr(const CGuildPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGuildPtr(const CGuildPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGuildPtr(CGuild *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CGuildPtr &assign(const CGuildPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CGuildPtr()
		{
			unlinkPtr();
		}

		CGuildPtr &assign(CGuild *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CGuildPtr &operator =(const CGuildPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CGuild *operator ->()
		{
			return _Ptr;
		}
		const CGuild *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CGuildPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CGuildPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CGuild *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CGuild *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CGuildPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CGuildPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CGuildInvitePtr
	{
		friend class CGuildInvite;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CGuildInvitePtr	*_NextPtr;
		CGuildInvitePtr	*_PrevPtr;

		CGuildInvite	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CGuildInvitePtr()
			: _FileName(NULL),
			_LineNum(0),
			_NextPtr(NULL),
			_PrevPtr(NULL),
			_Ptr(NULL)
		{
		}

		CGuildInvitePtr(const CGuildInvitePtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGuildInvitePtr(const CGuildInvitePtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGuildInvitePtr(CGuildInvite *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CGuildInvitePtr &assign(const CGuildInvitePtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CGuildInvitePtr()
		{
			unlinkPtr();
		}

		CGuildInvitePtr &assign(CGuildInvite *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CGuildInvitePtr &operator =(const CGuildInvitePtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CGuildInvite *operator ->()
		{
			return _Ptr;
		}
		const CGuildInvite *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CGuildInvitePtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CGuildInvitePtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CGuildInvite *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CGuildInvite *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CGuildInvitePtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CGuildInvitePtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CPlayerRatingPtr
	{
		friend class CPlayerRating;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CPlayerRatingPtr	*_NextPtr;
		CPlayerRatingPtr	*_PrevPtr;

		CPlayerRating	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CPlayerRatingPtr()
			: _FileName(NULL),
			_LineNum(0),
			_NextPtr(NULL),
			_PrevPtr(NULL),
			_Ptr(NULL)
		{
		}

		CPlayerRatingPtr(const CPlayerRatingPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CPlayerRatingPtr(const CPlayerRatingPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CPlayerRatingPtr(CPlayerRating *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CPlayerRatingPtr &assign(const CPlayerRatingPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CPlayerRatingPtr()
		{
			unlinkPtr();
		}

		CPlayerRatingPtr &assign(CPlayerRating *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CPlayerRatingPtr &operator =(const CPlayerRatingPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CPlayerRating *operator ->()
		{
			return _Ptr;
		}
		const CPlayerRating *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CPlayerRatingPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CPlayerRatingPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CPlayerRating *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CPlayerRating *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CPlayerRatingPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CPlayerRatingPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CJournalEntryPtr
	{
		friend class CJournalEntry;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CJournalEntryPtr	*_NextPtr;
		CJournalEntryPtr	*_PrevPtr;

		CJournalEntry	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CJournalEntryPtr()
			: _FileName(NULL),
			_LineNum(0),
			_NextPtr(NULL),
			_PrevPtr(NULL),
			_Ptr(NULL)
		{
		}

		CJournalEntryPtr(const CJournalEntryPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CJournalEntryPtr(const CJournalEntryPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CJournalEntryPtr(CJournalEntry *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CJournalEntryPtr &assign(const CJournalEntryPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CJournalEntryPtr()
		{
			unlinkPtr();
		}

		CJournalEntryPtr &assign(CJournalEntry *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CJournalEntryPtr &operator =(const CJournalEntryPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CJournalEntry *operator ->()
		{
			return _Ptr;
		}
		const CJournalEntry *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CJournalEntryPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CJournalEntryPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CJournalEntry *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CJournalEntry *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CJournalEntryPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CJournalEntryPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CFolderPtr
	{
		friend class CFolder;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CFolderPtr	*_NextPtr;
		CFolderPtr	*_PrevPtr;

		CFolder	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CFolderPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CFolderPtr(const CFolderPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CFolderPtr(const CFolderPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CFolderPtr(CFolder *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CFolderPtr &assign(const CFolderPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CFolderPtr()
		{
			unlinkPtr();
		}

		CFolderPtr &assign(CFolder *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CFolderPtr &operator =(const CFolderPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CFolder *operator ->()
		{
			return _Ptr;
		}
		const CFolder *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CFolderPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CFolderPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CFolder *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CFolder *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CFolderPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CFolderPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CFolderAccessPtr
	{
		friend class CFolderAccess;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CFolderAccessPtr	*_NextPtr;
		CFolderAccessPtr	*_PrevPtr;

		CFolderAccess	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CFolderAccessPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CFolderAccessPtr(const CFolderAccessPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CFolderAccessPtr(const CFolderAccessPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CFolderAccessPtr(CFolderAccess *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CFolderAccessPtr &assign(const CFolderAccessPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CFolderAccessPtr()
		{
			unlinkPtr();
		}

		CFolderAccessPtr &assign(CFolderAccess *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CFolderAccessPtr &operator =(const CFolderAccessPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CFolderAccess *operator ->()
		{
			return _Ptr;
		}
		const CFolderAccess *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CFolderAccessPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CFolderAccessPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CFolderAccess *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CFolderAccess *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CFolderAccessPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CFolderAccessPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CScenarioPtr
	{
		friend class CScenario;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CScenarioPtr	*_NextPtr;
		CScenarioPtr	*_PrevPtr;

		CScenario	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CScenarioPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CScenarioPtr(const CScenarioPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CScenarioPtr(const CScenarioPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CScenarioPtr(CScenario *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CScenarioPtr &assign(const CScenarioPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CScenarioPtr()
		{
			unlinkPtr();
		}

		CScenarioPtr &assign(CScenario *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CScenarioPtr &operator =(const CScenarioPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CScenario *operator ->()
		{
			return _Ptr;
		}
		const CScenario *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CScenarioPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CScenarioPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CScenario *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CScenario *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CScenarioPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CScenarioPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CSessionLogPtr
	{
		friend class CSessionLog;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CSessionLogPtr	*_NextPtr;
		CSessionLogPtr	*_PrevPtr;

		CSessionLog	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CSessionLogPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CSessionLogPtr(const CSessionLogPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionLogPtr(const CSessionLogPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CSessionLogPtr(CSessionLog *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CSessionLogPtr &assign(const CSessionLogPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CSessionLogPtr()
		{
			unlinkPtr();
		}

		CSessionLogPtr &assign(CSessionLog *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CSessionLogPtr &operator =(const CSessionLogPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CSessionLog *operator ->()
		{
			return _Ptr;
		}
		const CSessionLog *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CSessionLogPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CSessionLogPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CSessionLog *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CSessionLog *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CSessionLogPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CSessionLogPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};





	class CGmStatusPtr
	{
		friend class CGmStatus;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		CGmStatusPtr	*_NextPtr;
		CGmStatusPtr	*_PrevPtr;

		CGmStatus	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		CGmStatusPtr()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		CGmStatusPtr(const CGmStatusPtr &other, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGmStatusPtr(const CGmStatusPtr &other)
			: _FileName(other._FileName),
			_LineNum(other._LineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			// point the same object
			_Ptr = other._Ptr;
			// insert the pointer in the list
			linkPtr();
		}

		CGmStatusPtr(CGmStatus *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;
			
			linkPtr();
		}

		CGmStatusPtr &assign(const CGmStatusPtr &other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~CGmStatusPtr()
		{
			unlinkPtr();
		}

		CGmStatusPtr &assign(CGmStatus *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		CGmStatusPtr &operator =(const CGmStatusPtr &other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		CGmStatus *operator ->()
		{
			return _Ptr;
		}
		const CGmStatus *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const CGmStatusPtr &other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const CGmStatusPtr &other) const
		{
			return !operator ==(other);
		}

		bool operator == (const CGmStatus *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const CGmStatus *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator < (const CGmStatusPtr &other) const
		{
			return _Ptr < other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		CGmStatusPtr *getNextPtr()
		{
			return _NextPtr;
		}
	};




	struct TUserType
	{
		enum TValues
		{
			ut_character = 1,
			ut_pioneer,
			/// the highest valid value in the enum
			last_enum_item = ut_pioneer,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 2
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ut_character, 0));
				indexTable.insert(std::make_pair(ut_pioneer, 1));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ut_character)
				NL_STRING_CONVERSION_TABLE_ENTRY(ut_pioneer)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TUserType()
			: _Value(invalid_val)
		{
		}
		TUserType(TValues value)
			: _Value(value)
		{
		}

		TUserType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TUserType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TUserType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TUserType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TUserType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TUserType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TUserType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TKnownUserRelation
	{
		enum TValues
		{
			rt_friend = 1,
			rt_banned,
			rt_friend_dm,
			/// the highest valid value in the enum
			last_enum_item = rt_friend_dm,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 3
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(rt_friend, 0));
				indexTable.insert(std::make_pair(rt_banned, 1));
				indexTable.insert(std::make_pair(rt_friend_dm, 2));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_friend)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_banned)
				NL_STRING_CONVERSION_TABLE_ENTRY(rt_friend_dm)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TKnownUserRelation()
			: _Value(invalid_val)
		{
		}
		TKnownUserRelation(TValues value)
			: _Value(value)
		{
		}

		TKnownUserRelation(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TKnownUserRelation &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TKnownUserRelation &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TKnownUserRelation &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TKnownUserRelation &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TKnownUserRelation &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TKnownUserRelation &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CKnownUser
	{
	protected:
		// 
		uint32	_RelationId;
		// 
		uint32	_OwnerId;
		// 
		uint32	_TargetUser;
		// 
		uint32	_TargetCharacter;
		// 
		TKnownUserRelation	_Relation;
		// 
		std::string	_Comments;
	public:
		// 
		uint32 getOwnerId() const
		{
			return _OwnerId;
		}

		void setOwnerId(uint32 value)
		{

			if (_OwnerId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_OwnerId = value;

			}

		}
			// 
		uint32 getTargetUser() const
		{
			return _TargetUser;
		}

		void setTargetUser(uint32 value)
		{

			if (_TargetUser != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_TargetUser = value;

			}

		}
			// 
		uint32 getTargetCharacter() const
		{
			return _TargetCharacter;
		}

		void setTargetCharacter(uint32 value)
		{

			if (_TargetCharacter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_TargetCharacter = value;

			}

		}
			// 
		TKnownUserRelation getRelation() const
		{
			return _Relation;
		}

		void setRelation(TKnownUserRelation value)
		{

			if (_Relation != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Relation = value;

			}

		}
			// 
		const std::string &getComments() const
		{
			return _Comments;
		}



		void setComments(const std::string &value)
		{

			if (_Comments != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Comments = value;

				
			}

		}
	
		bool operator == (const CKnownUser &other) const
		{
			return _RelationId == other._RelationId
				&& _OwnerId == other._OwnerId
				&& _TargetUser == other._TargetUser
				&& _TargetCharacter == other._TargetCharacter
				&& _Relation == other._Relation
				&& _Comments == other._Comments;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CKnownUser()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_RelationId(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CKnownUser();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CKnownUserPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CKnownUserPtr(new CKnownUser(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CKnownUserPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRingUser and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CKnownUserPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CCharacter and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CKnownUserPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CKnownUserPtr;

		typedef std::map<uint32, CKnownUser*>	TObjectCache;
		typedef std::set<CKnownUser*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CKnownUserPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CKnownUser *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CKnownUserPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CKnownUserPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _RelationId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_RelationId == NOPE::INVALID_OBJECT_ID);
			_RelationId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CSessionParticipant
	{
	protected:
		// 
		uint32	_Id;
		// 
		TSessionId	_SessionId;
		// 
		uint32	_CharId;
		// 
		TSessionPartStatus	_Status;
		// 
		bool	_Kicked;
	public:
		// 
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

			if (_SessionId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SessionId = value;

			}

		}
			// 
		uint32 getCharId() const
		{
			return _CharId;
		}

		void setCharId(uint32 value)
		{

			if (_CharId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CharId = value;

			}

		}
			// 
		TSessionPartStatus getStatus() const
		{
			return _Status;
		}

		void setStatus(TSessionPartStatus value)
		{

			if (_Status != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Status = value;

			}

		}
			// 
		bool getKicked() const
		{
			return _Kicked;
		}

		void setKicked(bool value)
		{

			if (_Kicked != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Kicked = value;

			}

		}
	
		bool operator == (const CSessionParticipant &other) const
		{
			return _Id == other._Id
				&& _SessionId == other._SessionId
				&& _CharId == other._CharId
				&& _Status == other._Status
				&& _Kicked == other._Kicked;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CSessionParticipant()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_Kicked = false;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CSessionParticipant();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CSessionParticipantPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CSessionParticipantPtr(new CSessionParticipant(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CSessionParticipantPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CCharacter and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionParticipantPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CSession and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionParticipantPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CSessionParticipantPtr;

		typedef std::map<uint32, CSessionParticipant*>	TObjectCache;
		typedef std::set<CSessionParticipant*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CSessionParticipantPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CSessionParticipant *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CSessionParticipantPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CSessionParticipantPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CCharacter
	{
	protected:
		// 
		uint32	_CharId;
		// 
		std::string	_CharName;
		// 
		uint32	_UserId;
		// 
		uint32	_GuildId;
		// 
		uint32	_BestCombatLevel;
		// 
		uint32	_HomeMainlandSessionId;
		// 
		std::string	_RingAccess;
		// 
		CHARSYNC::TRace	_Race;
		// 
		CHARSYNC::TCivilisation	_Civilisation;
		// 
		CHARSYNC::TCult	_Cult;
		// 
		uint32	_CurrentSession;
		// 
		uint32	_RRPAM;
		// 
		uint32	_RRPMasterless;
		// 
		uint32	_RRPAuthor;
		// 
		bool	_Newcomer;
		// 
		uint32	_CreationDate;
		// 
		uint32	_LastPlayedDate;

		friend class CSession;

		std::vector < CSessionPtr >	*_Sessions;

		friend class CSessionParticipant;

		std::vector < CSessionParticipantPtr >	*_SessionParticipants;

		friend class CKnownUser;

		std::vector < CKnownUserPtr >	*_KnownBy;

		friend class CPlayerRating;

		std::vector < CPlayerRatingPtr >	*_PlayerRatings;
	public:
		// 
		const std::string &getCharName() const
		{
			return _CharName;
		}



		void setCharName(const std::string &value)
		{

			if (_CharName != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_CharName = value;

				
			}

		}
			// 
		uint32 getUserId() const
		{
			return _UserId;
		}

		void setUserId(uint32 value)
		{

			if (_UserId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_UserId = value;

			}

		}
			// 
		uint32 getGuildId() const
		{
			return _GuildId;
		}

		void setGuildId(uint32 value)
		{

			if (_GuildId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_GuildId = value;

			}

		}
			// 
		uint32 getBestCombatLevel() const
		{
			return _BestCombatLevel;
		}

		void setBestCombatLevel(uint32 value)
		{

			if (_BestCombatLevel != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_BestCombatLevel = value;

			}

		}
			// 
		uint32 getHomeMainlandSessionId() const
		{
			return _HomeMainlandSessionId;
		}

		void setHomeMainlandSessionId(uint32 value)
		{

			if (_HomeMainlandSessionId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_HomeMainlandSessionId = value;

			}

		}
			// 
		const std::string &getRingAccess() const
		{
			return _RingAccess;
		}



		void setRingAccess(const std::string &value)
		{

			if (_RingAccess != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_RingAccess = value;

				
			}

		}
			// 
		CHARSYNC::TRace getRace() const
		{
			return _Race;
		}

		void setRace(CHARSYNC::TRace value)
		{

			if (_Race != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Race = value;

			}

		}
			// 
		CHARSYNC::TCivilisation getCivilisation() const
		{
			return _Civilisation;
		}

		void setCivilisation(CHARSYNC::TCivilisation value)
		{

			if (_Civilisation != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Civilisation = value;

			}

		}
			// 
		CHARSYNC::TCult getCult() const
		{
			return _Cult;
		}

		void setCult(CHARSYNC::TCult value)
		{

			if (_Cult != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Cult = value;

			}

		}
			// 
		uint32 getCurrentSession() const
		{
			return _CurrentSession;
		}

		void setCurrentSession(uint32 value)
		{

			if (_CurrentSession != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentSession = value;

			}

		}
			// 
		uint32 getRRPAM() const
		{
			return _RRPAM;
		}

		void setRRPAM(uint32 value)
		{

			if (_RRPAM != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RRPAM = value;

			}

		}
			// 
		uint32 getRRPMasterless() const
		{
			return _RRPMasterless;
		}

		void setRRPMasterless(uint32 value)
		{

			if (_RRPMasterless != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RRPMasterless = value;

			}

		}
			// 
		uint32 getRRPAuthor() const
		{
			return _RRPAuthor;
		}

		void setRRPAuthor(uint32 value)
		{

			if (_RRPAuthor != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RRPAuthor = value;

			}

		}
			// 
		bool getNewcomer() const
		{
			return _Newcomer;
		}

		void setNewcomer(bool value)
		{

			if (_Newcomer != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Newcomer = value;

			}

		}
			// 
		uint32 getCreationDate() const
		{
			return _CreationDate;
		}

		void setCreationDate(uint32 value)
		{

			if (_CreationDate != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CreationDate = value;

			}

		}
			// 
		uint32 getLastPlayedDate() const
		{
			return _LastPlayedDate;
		}

		void setLastPlayedDate(uint32 value)
		{

			if (_LastPlayedDate != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_LastPlayedDate = value;

			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CSessionPtr> &getSessions() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CSessionPtr &getSessionsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CSessionPtr &getSessionsById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CSessionParticipantPtr> &getSessionParticipants() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CSessionParticipantPtr &getSessionParticipantsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CSessionParticipantPtr &getSessionParticipantsById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CKnownUserPtr> &getKnownBy() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CKnownUserPtr &getKnownByByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CKnownUserPtr &getKnownByById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CPlayerRatingPtr> &getPlayerRatings() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CPlayerRatingPtr &getPlayerRatingsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CPlayerRatingPtr &getPlayerRatingsById(uint32 id) const;

	
		bool operator == (const CCharacter &other) const
		{
			return _CharId == other._CharId
				&& _CharName == other._CharName
				&& _UserId == other._UserId
				&& _GuildId == other._GuildId
				&& _BestCombatLevel == other._BestCombatLevel
				&& _HomeMainlandSessionId == other._HomeMainlandSessionId
				&& _RingAccess == other._RingAccess
				&& _Race == other._Race
				&& _Civilisation == other._Civilisation
				&& _Cult == other._Cult
				&& _CurrentSession == other._CurrentSession
				&& _RRPAM == other._RRPAM
				&& _RRPMasterless == other._RRPMasterless
				&& _RRPAuthor == other._RRPAuthor
				&& _Newcomer == other._Newcomer
				&& _CreationDate == other._CreationDate
				&& _LastPlayedDate == other._LastPlayedDate;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CCharacter()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_CharId(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_GuildId = 0;
			_BestCombatLevel = 0;
			_HomeMainlandSessionId = 0;
			_CurrentSession = 0;
			_RRPAM = 0;
			_RRPMasterless = 0;
			_RRPAuthor = 0;
			_Newcomer = 1;
			_LastPlayedDate = 0;
			_Sessions = NULL;
			_SessionParticipants = NULL;
			_KnownBy = NULL;
			_PlayerRatings = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CCharacter();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CCharacterPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CCharacterPtr(new CCharacter(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CCharacterPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRingUser and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CCharacterPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CGuild and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCGuild(MSW::CConnection &connection, uint32 parentId, std::vector < CCharacterPtr > &children, const char *filename, uint32 lineNum);

		/// Load Sessions child(ren) object(s).
		bool loadSessions(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load SessionParticipants child(ren) object(s).
		bool loadSessionParticipants(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load KnownBy child(ren) object(s).
		bool loadKnownBy(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load PlayerRatings child(ren) object(s).
		bool loadPlayerRatings(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CCharacterPtr;

		typedef std::map<uint32, CCharacter*>	TObjectCache;
		typedef std::set<CCharacter*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CCharacterPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CCharacter *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CCharacterPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CCharacterPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _CharId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_CharId == NOPE::INVALID_OBJECT_ID);
			_CharId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	

	struct TCurrentActivity
	{
		enum TValues
		{
			ca_none = 1,
			ca_play,
			ca_edit,
			ca_anim,
			/// the highest valid value in the enum
			last_enum_item = ca_anim,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 4
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ca_none, 0));
				indexTable.insert(std::make_pair(ca_play, 1));
				indexTable.insert(std::make_pair(ca_edit, 2));
				indexTable.insert(std::make_pair(ca_anim, 3));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ca_none)
				NL_STRING_CONVERSION_TABLE_ENTRY(ca_play)
				NL_STRING_CONVERSION_TABLE_ENTRY(ca_edit)
				NL_STRING_CONVERSION_TABLE_ENTRY(ca_anim)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TCurrentActivity()
			: _Value(invalid_val)
		{
		}
		TCurrentActivity(TValues value)
			: _Value(value)
		{
		}

		TCurrentActivity(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCurrentActivity &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCurrentActivity &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCurrentActivity &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TCurrentActivity &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TCurrentActivity &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TCurrentActivity &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TCurrentStatus
	{
		enum TValues
		{
			cs_offline = 1,
			cs_logged,
			cs_online,
			/// the highest valid value in the enum
			last_enum_item = cs_online,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 3
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(cs_offline, 0));
				indexTable.insert(std::make_pair(cs_logged, 1));
				indexTable.insert(std::make_pair(cs_online, 2));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(cs_offline)
				NL_STRING_CONVERSION_TABLE_ENTRY(cs_logged)
				NL_STRING_CONVERSION_TABLE_ENTRY(cs_online)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TCurrentStatus()
			: _Value(invalid_val)
		{
		}
		TCurrentStatus(TValues value)
			: _Value(value)
		{
		}

		TCurrentStatus(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TCurrentStatus &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TCurrentStatus &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TCurrentStatus &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TCurrentStatus &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TCurrentStatus &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TCurrentStatus &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TPublicLevel
	{
		enum TValues
		{
			ul_none = 1,
			ul_public,
			/// the highest valid value in the enum
			last_enum_item = ul_public,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 2
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ul_none, 0));
				indexTable.insert(std::make_pair(ul_public, 1));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ul_none)
				NL_STRING_CONVERSION_TABLE_ENTRY(ul_public)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TPublicLevel()
			: _Value(invalid_val)
		{
		}
		TPublicLevel(TValues value)
			: _Value(value)
		{
		}

		TPublicLevel(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TPublicLevel &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TPublicLevel &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TPublicLevel &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TPublicLevel &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TPublicLevel &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TPublicLevel &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TAccountType
	{
		enum TValues
		{
			at_normal = 1,
			at_gold,
			/// the highest valid value in the enum
			last_enum_item = at_gold,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 2
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(at_normal, 0));
				indexTable.insert(std::make_pair(at_gold, 1));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(at_normal)
				NL_STRING_CONVERSION_TABLE_ENTRY(at_gold)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TAccountType()
			: _Value(invalid_val)
		{
		}
		TAccountType(TValues value)
			: _Value(value)
		{
		}

		TAccountType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TAccountType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TAccountType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TAccountType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TAccountType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TAccountType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TAccountType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TLanguage
	{
		enum TValues
		{
			lang_en = 1,
			lang_fr,
			lang_de,
			lang_other,
			/// the highest valid value in the enum
			last_enum_item = lang_other,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 4
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(lang_en, 0));
				indexTable.insert(std::make_pair(lang_fr, 1));
				indexTable.insert(std::make_pair(lang_de, 2));
				indexTable.insert(std::make_pair(lang_other, 3));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(lang_en)
				NL_STRING_CONVERSION_TABLE_ENTRY(lang_fr)
				NL_STRING_CONVERSION_TABLE_ENTRY(lang_de)
				NL_STRING_CONVERSION_TABLE_ENTRY(lang_other)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TLanguage()
			: _Value(invalid_val)
		{
		}
		TLanguage(TValues value)
			: _Value(value)
		{
		}

		TLanguage(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TLanguage &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TLanguage &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TLanguage &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TLanguage &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TLanguage &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TLanguage &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CRingUser
	{
	protected:
		// 
		uint32	_UserId;
		// 
		std::string	_UserName;
		// 
		uint32	_CurrentCharacter;
		// 
		uint32	_CurrentSession;
		// 
		TCurrentActivity	_CurrentActivity;
		// 
		TCurrentStatus	_CurrentStatus;
		// 
		TPublicLevel	_PublicLevel;
		// 
		TAccountType	_AccountType;
		// 
		std::string	_ContentAccessLevel;
		// 
		std::string	_Description;
		// 
		TLanguage	_Lang;
		// 
		std::string	_Cookie;
		// 
		sint32	_CurrentDomainId;
		// 
		std::string	_AddedPrivileges;

		friend class CKnownUser;

		std::vector < CKnownUserPtr >	*_KnownUsers;

		friend class CCharacter;

		std::map < uint32,  CCharacterPtr >	*_Characters;

		friend class CFolder;

		std::vector < CFolderPtr >	*_Folders;

		friend class CFolderAccess;

		std::vector < CFolderAccessPtr >	*_FolderAccess;
		friend class CGmStatus;
		bool								_GMStatusLoaded;
		CGmStatusPtr	_GMStatus;
	public:
		// 
		const std::string &getUserName() const
		{
			return _UserName;
		}



		void setUserName(const std::string &value)
		{

			if (_UserName != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_UserName = value;

				
			}

		}
			// 
		uint32 getCurrentCharacter() const
		{
			return _CurrentCharacter;
		}

		void setCurrentCharacter(uint32 value)
		{

			if (_CurrentCharacter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentCharacter = value;

			}

		}
			// 
		uint32 getCurrentSession() const
		{
			return _CurrentSession;
		}

		void setCurrentSession(uint32 value)
		{

			if (_CurrentSession != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentSession = value;

			}

		}
			// 
		TCurrentActivity getCurrentActivity() const
		{
			return _CurrentActivity;
		}

		void setCurrentActivity(TCurrentActivity value)
		{

			if (_CurrentActivity != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentActivity = value;

			}

		}
			// 
		TCurrentStatus getCurrentStatus() const
		{
			return _CurrentStatus;
		}

		void setCurrentStatus(TCurrentStatus value)
		{

			if (_CurrentStatus != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentStatus = value;

			}

		}
			// 
		TPublicLevel getPublicLevel() const
		{
			return _PublicLevel;
		}

		void setPublicLevel(TPublicLevel value)
		{

			if (_PublicLevel != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_PublicLevel = value;

			}

		}
			// 
		TAccountType getAccountType() const
		{
			return _AccountType;
		}

		void setAccountType(TAccountType value)
		{

			if (_AccountType != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_AccountType = value;

			}

		}
			// 
		const std::string &getContentAccessLevel() const
		{
			return _ContentAccessLevel;
		}



		void setContentAccessLevel(const std::string &value)
		{

			if (_ContentAccessLevel != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_ContentAccessLevel = value;

				
			}

		}
			// 
		const std::string &getDescription() const
		{
			return _Description;
		}



		void setDescription(const std::string &value)
		{

			if (_Description != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Description = value;

				
			}

		}
			// 
		TLanguage getLang() const
		{
			return _Lang;
		}

		void setLang(TLanguage value)
		{

			if (_Lang != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Lang = value;

			}

		}
			// 
		const std::string &getCookie() const
		{
			return _Cookie;
		}



		void setCookie(const std::string &value)
		{

			if (_Cookie != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Cookie = value;

				
			}

		}
			// 
		sint32 getCurrentDomainId() const
		{
			return _CurrentDomainId;
		}

		void setCurrentDomainId(sint32 value)
		{

			if (_CurrentDomainId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_CurrentDomainId = value;

			}

		}
			// 
		const std::string &getAddedPrivileges() const
		{
			return _AddedPrivileges;
		}



		void setAddedPrivileges(const std::string &value)
		{

			if (_AddedPrivileges != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_AddedPrivileges = value;

				
			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CKnownUserPtr> &getKnownUsers() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CKnownUserPtr &getKnownUsersByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CKnownUserPtr &getKnownUsersById(uint32 id) const;

	
		/** Return a const reference to the map of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following method who return non const pointer
		 *	on contained elements.
		 */
		const std::map<uint32, CCharacterPtr> &getCharacters() const;
		/** Return the identified element by looking in the map
		 *	If no element match the id, NULL pointer is returned
		 */
		CCharacterPtr &getCharactersById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CFolderPtr> &getFolders() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CFolderPtr &getFoldersByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CFolderPtr &getFoldersById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CFolderAccessPtr> &getFolderAccess() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CFolderAccessPtr &getFolderAccessByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CFolderAccessPtr &getFolderAccessById(uint32 id) const;

	
		/** Return the one child object (or null if not) */
		CGmStatusPtr getGMStatus();

		bool operator == (const CRingUser &other) const
		{
			return _UserId == other._UserId
				&& _UserName == other._UserName
				&& _CurrentCharacter == other._CurrentCharacter
				&& _CurrentSession == other._CurrentSession
				&& _CurrentActivity == other._CurrentActivity
				&& _CurrentStatus == other._CurrentStatus
				&& _PublicLevel == other._PublicLevel
				&& _AccountType == other._AccountType
				&& _ContentAccessLevel == other._ContentAccessLevel
				&& _Description == other._Description
				&& _Lang == other._Lang
				&& _Cookie == other._Cookie
				&& _CurrentDomainId == other._CurrentDomainId
				&& _AddedPrivileges == other._AddedPrivileges;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CRingUser()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_UserId(NOPE::INVALID_OBJECT_ID)
		{
			_KnownUsers = NULL;
			_Characters = NULL;
			_Folders = NULL;
			_FolderAccess = NULL;
			_GMStatusLoaded = false;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CRingUser();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CRingUserPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CRingUserPtr(new CRingUser(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CRingUserPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/// Load KnownUsers child(ren) object(s).
		bool loadKnownUsers(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load Characters child(ren) object(s).
		bool loadCharacters(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load Folders child(ren) object(s).
		bool loadFolders(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load FolderAccess child(ren) object(s).
		bool loadFolderAccess(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load GMStatus child(ren) object(s).
		bool loadGMStatus(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CRingUserPtr;

		typedef std::map<uint32, CRingUser*>	TObjectCache;
		typedef std::set<CRingUser*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CRingUserPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CRingUser *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CRingUserPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CRingUserPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _UserId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_UserId == NOPE::INVALID_OBJECT_ID);
			_UserId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	

	struct TRelationToParent
	{
		enum TValues
		{
			rtp_same = 1,
			rtp_variant,
			rtp_different,
			/// the highest valid value in the enum
			last_enum_item = rtp_different,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 3
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(rtp_same, 0));
				indexTable.insert(std::make_pair(rtp_variant, 1));
				indexTable.insert(std::make_pair(rtp_different, 2));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(rtp_same)
				NL_STRING_CONVERSION_TABLE_ENTRY(rtp_variant)
				NL_STRING_CONVERSION_TABLE_ENTRY(rtp_different)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TRelationToParent()
			: _Value(invalid_val)
		{
		}
		TRelationToParent(TValues value)
			: _Value(value)
		{
		}

		TRelationToParent(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TRelationToParent &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TRelationToParent &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TRelationToParent &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TRelationToParent &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TRelationToParent &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TRelationToParent &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
	

	struct TPlayType
	{
		enum TValues
		{
			pt_rp = 1,
			pt_pvp,
			/// the highest valid value in the enum
			last_enum_item = pt_pvp,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 2
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(pt_rp, 0));
				indexTable.insert(std::make_pair(pt_pvp, 1));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(pt_rp)
				NL_STRING_CONVERSION_TABLE_ENTRY(pt_pvp)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TPlayType()
			: _Value(invalid_val)
		{
		}
		TPlayType(TValues value)
			: _Value(value)
		{
		}

		TPlayType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TPlayType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TPlayType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TPlayType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TPlayType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TPlayType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TPlayType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CSession
	{
	protected:
		// 
		TSessionId	_SessionId;
		// 
		TSessionType	_SessionType;
		// 
		std::string	_Title;
		// 
		uint32	_OwnerId;
		// 
		uint32	_PlanDate;
		// 
		uint32	_StartDate;
		// 
		std::string	_Description;
		// 
		TSessionOrientation	_Orientation;
		// 
		R2::TSessionLevel	_Level;
		// 
		TRuleType	_RuleType;
		// 
		TAccessType	_AccessType;
		// 
		TSessionState	_State;
		// 
		uint32	_HostShardId;
		// 
		uint32	_SubscriptionSlots;
		// 
		uint32	_ReservedSlots;
		// 
		TEstimatedDuration	_EstimatedDuration;
		// 
		uint32	_FinalDuration;
		// 
		uint32	_FolderId;
		// 
		std::string	_Lang;
		// 
		std::string	_Icone;
		// 
		TAnimMode	_AnimMode;
		// 
		TRaceFilter	_RaceFilter;
		// 
		TReligionFilter	_ReligionFilter;
		// 
		TGuildFilter	_GuildFilter;
		// 
		TShardFilter	_ShardFilter;
		// 
		TLevelFilter	_LevelFilter;
		// 
		bool	_SubscriptionClosed;
		// 
		bool	_Newcomer;

		friend class CSessionParticipant;

		std::vector < CSessionParticipantPtr >	*_SessionParticipants;

		friend class CGuildInvite;

		std::vector < CGuildInvitePtr >	*_GuildInvites;

		friend class CJournalEntry;

		std::vector < CJournalEntryPtr >	*_JournalEntries;
	public:
		// 
		TSessionType getSessionType() const
		{
			return _SessionType;
		}

		void setSessionType(TSessionType value)
		{

			if (_SessionType != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SessionType = value;

			}

		}
			// 
		const std::string &getTitle() const
		{
			return _Title;
		}



		void setTitle(const std::string &value)
		{

			if (_Title != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Title = value;

				
			}

		}
			// 
		uint32 getOwnerId() const
		{
			return _OwnerId;
		}

		void setOwnerId(uint32 value)
		{

			if (_OwnerId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_OwnerId = value;

			}

		}
			// 
		uint32 getPlanDate() const
		{
			return _PlanDate;
		}

		void setPlanDate(uint32 value)
		{

			if (_PlanDate != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_PlanDate = value;

			}

		}
			// 
		uint32 getStartDate() const
		{
			return _StartDate;
		}

		void setStartDate(uint32 value)
		{

			if (_StartDate != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_StartDate = value;

			}

		}
			// 
		const std::string &getDescription() const
		{
			return _Description;
		}



		void setDescription(const std::string &value)
		{

			if (_Description != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Description = value;

				
			}

		}
			// 
		TSessionOrientation getOrientation() const
		{
			return _Orientation;
		}

		void setOrientation(TSessionOrientation value)
		{

			if (_Orientation != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Orientation = value;

			}

		}
			// 
		R2::TSessionLevel getLevel() const
		{
			return _Level;
		}

		void setLevel(R2::TSessionLevel value)
		{

			if (_Level != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Level = value;

			}

		}
			// 
		TRuleType getRuleType() const
		{
			return _RuleType;
		}

		void setRuleType(TRuleType value)
		{

			if (_RuleType != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RuleType = value;

			}

		}
			// 
		TAccessType getAccessType() const
		{
			return _AccessType;
		}

		void setAccessType(TAccessType value)
		{

			if (_AccessType != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_AccessType = value;

			}

		}
			// 
		TSessionState getState() const
		{
			return _State;
		}

		void setState(TSessionState value)
		{

			if (_State != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_State = value;

			}

		}
			// 
		uint32 getHostShardId() const
		{
			return _HostShardId;
		}

		void setHostShardId(uint32 value)
		{

			if (_HostShardId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_HostShardId = value;

			}

		}
			// 
		uint32 getSubscriptionSlots() const
		{
			return _SubscriptionSlots;
		}

		void setSubscriptionSlots(uint32 value)
		{

			if (_SubscriptionSlots != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SubscriptionSlots = value;

			}

		}
			// 
		uint32 getReservedSlots() const
		{
			return _ReservedSlots;
		}

		void setReservedSlots(uint32 value)
		{

			if (_ReservedSlots != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ReservedSlots = value;

			}

		}
			// 
		TEstimatedDuration getEstimatedDuration() const
		{
			return _EstimatedDuration;
		}

		void setEstimatedDuration(TEstimatedDuration value)
		{

			if (_EstimatedDuration != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_EstimatedDuration = value;

			}

		}
			// 
		uint32 getFinalDuration() const
		{
			return _FinalDuration;
		}

		void setFinalDuration(uint32 value)
		{

			if (_FinalDuration != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_FinalDuration = value;

			}

		}
			// 
		uint32 getFolderId() const
		{
			return _FolderId;
		}

		void setFolderId(uint32 value)
		{

			if (_FolderId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_FolderId = value;

			}

		}
			// 
		const std::string &getLang() const
		{
			return _Lang;
		}



		void setLang(const std::string &value)
		{

			if (_Lang != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Lang = value;

				
			}

		}
			// 
		const std::string &getIcone() const
		{
			return _Icone;
		}



		void setIcone(const std::string &value)
		{

			if (_Icone != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Icone = value;

				
			}

		}
			// 
		TAnimMode getAnimMode() const
		{
			return _AnimMode;
		}

		void setAnimMode(TAnimMode value)
		{

			if (_AnimMode != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_AnimMode = value;

			}

		}
			// 
		TRaceFilter getRaceFilter() const
		{
			return _RaceFilter;
		}

		void setRaceFilter(TRaceFilter value)
		{

			if (_RaceFilter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RaceFilter = value;

			}

		}
			// 
		TReligionFilter getReligionFilter() const
		{
			return _ReligionFilter;
		}

		void setReligionFilter(TReligionFilter value)
		{

			if (_ReligionFilter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ReligionFilter = value;

			}

		}
			// 
		TGuildFilter getGuildFilter() const
		{
			return _GuildFilter;
		}

		void setGuildFilter(TGuildFilter value)
		{

			if (_GuildFilter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_GuildFilter = value;

			}

		}
			// 
		TShardFilter getShardFilter() const
		{
			return _ShardFilter;
		}

		void setShardFilter(TShardFilter value)
		{

			if (_ShardFilter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ShardFilter = value;

			}

		}
			// 
		TLevelFilter getLevelFilter() const
		{
			return _LevelFilter;
		}

		void setLevelFilter(TLevelFilter value)
		{

			if (_LevelFilter != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_LevelFilter = value;

			}

		}
			// 
		bool getSubscriptionClosed() const
		{
			return _SubscriptionClosed;
		}

		void setSubscriptionClosed(bool value)
		{

			if (_SubscriptionClosed != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SubscriptionClosed = value;

			}

		}
			// 
		bool getNewcomer() const
		{
			return _Newcomer;
		}

		void setNewcomer(bool value)
		{

			if (_Newcomer != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Newcomer = value;

			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CSessionParticipantPtr> &getSessionParticipants() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CSessionParticipantPtr &getSessionParticipantsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CSessionParticipantPtr &getSessionParticipantsById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CGuildInvitePtr> &getGuildInvites() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CGuildInvitePtr &getGuildInvitesByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CGuildInvitePtr &getGuildInvitesById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CJournalEntryPtr> &getJournalEntries() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CJournalEntryPtr &getJournalEntriesByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CJournalEntryPtr &getJournalEntriesById(uint32 id) const;

	
		bool operator == (const CSession &other) const
		{
			return _SessionId == other._SessionId
				&& _SessionType == other._SessionType
				&& _Title == other._Title
				&& _OwnerId == other._OwnerId
				&& _PlanDate == other._PlanDate
				&& _StartDate == other._StartDate
				&& _Description == other._Description
				&& _Orientation == other._Orientation
				&& _Level == other._Level
				&& _RuleType == other._RuleType
				&& _AccessType == other._AccessType
				&& _State == other._State
				&& _HostShardId == other._HostShardId
				&& _SubscriptionSlots == other._SubscriptionSlots
				&& _ReservedSlots == other._ReservedSlots
				&& _EstimatedDuration == other._EstimatedDuration
				&& _FinalDuration == other._FinalDuration
				&& _FolderId == other._FolderId
				&& _Lang == other._Lang
				&& _Icone == other._Icone
				&& _AnimMode == other._AnimMode
				&& _RaceFilter == other._RaceFilter
				&& _ReligionFilter == other._ReligionFilter
				&& _GuildFilter == other._GuildFilter
				&& _ShardFilter == other._ShardFilter
				&& _LevelFilter == other._LevelFilter
				&& _SubscriptionClosed == other._SubscriptionClosed
				&& _Newcomer == other._Newcomer;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CSession()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_SessionId(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_AccessType = TAccessType::at_private;
			_Newcomer = 1;
			_SessionParticipants = NULL;
			_GuildInvites = NULL;
			_JournalEntries = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CSession();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CSessionPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CSessionPtr(new CSession(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CSessionPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CCharacter and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CFolder and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCFolder(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionPtr > &children, const char *filename, uint32 lineNum);

		/// Load SessionParticipants child(ren) object(s).
		bool loadSessionParticipants(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load GuildInvites child(ren) object(s).
		bool loadGuildInvites(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load JournalEntries child(ren) object(s).
		bool loadJournalEntries(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CSessionPtr;

		typedef std::map<uint32, CSession*>	TObjectCache;
		typedef std::set<CSession*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CSessionPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CSession *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CSessionPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CSessionPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _SessionId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_SessionId == NOPE::INVALID_OBJECT_ID);
			_SessionId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	

	struct TAccessLevel
	{
		enum TValues
		{
			ds_close,
			ds_dev,
			ds_restricted,
			ds_open,
			/// the highest valid value in the enum
			last_enum_item = ds_open,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 4
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(ds_close, 0));
				indexTable.insert(std::make_pair(ds_dev, 1));
				indexTable.insert(std::make_pair(ds_restricted, 2));
				indexTable.insert(std::make_pair(ds_open, 3));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(ds_close)
				NL_STRING_CONVERSION_TABLE_ENTRY(ds_dev)
				NL_STRING_CONVERSION_TABLE_ENTRY(ds_restricted)
				NL_STRING_CONVERSION_TABLE_ENTRY(ds_open)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TAccessLevel()
			: _Value(invalid_val)
		{
		}
		TAccessLevel(TValues value)
			: _Value(value)
		{
		}

		TAccessLevel(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TAccessLevel &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TAccessLevel &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TAccessLevel &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TAccessLevel &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TAccessLevel &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TAccessLevel &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CShard
	{
	protected:
		// 
		uint32	_ShardId;
		// 
		bool	_WSOnline;
		// 
		TAccessLevel	_RequiredState;
		// 
		std::string	_MOTD;

		friend class CGuild;

		std::map < uint32,  CGuildPtr >	*_Guilds;
	public:
		// 
		bool getWSOnline() const
		{
			return _WSOnline;
		}

		void setWSOnline(bool value)
		{

			if (_WSOnline != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_WSOnline = value;

			}

		}
			// 
		const TAccessLevel &getRequiredState() const
		{
			return _RequiredState;
		}



		void setRequiredState(const TAccessLevel &value)
		{

			if (_RequiredState != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_RequiredState = value;

				
			}

		}
			// 
		const std::string &getMOTD() const
		{
			return _MOTD;
		}



		void setMOTD(const std::string &value)
		{

			if (_MOTD != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_MOTD = value;

				
			}

		}
	
		/** Return a const reference to the map of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following method who return non const pointer
		 *	on contained elements.
		 */
		const std::map<uint32, CGuildPtr> &getGuilds() const;
		/** Return the identified element by looking in the map
		 *	If no element match the id, NULL pointer is returned
		 */
		CGuildPtr &getGuildsById(uint32 id) const;

	
		bool operator == (const CShard &other) const
		{
			return _ShardId == other._ShardId
				&& _WSOnline == other._WSOnline
				&& _RequiredState == other._RequiredState
				&& _MOTD == other._MOTD;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CShard()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_ShardId(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_WSOnline = false;
			_Guilds = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CShard();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CShardPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CShardPtr(new CShard(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CShardPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/// Load Guilds child(ren) object(s).
		bool loadGuilds(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CShardPtr;

		typedef std::map<uint32, CShard*>	TObjectCache;
		typedef std::set<CShard*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CShardPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CShard *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CShardPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CShardPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _ShardId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_ShardId == NOPE::INVALID_OBJECT_ID);
			_ShardId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuild
	{
	protected:
		// 
		uint32	_GuildId;
		// 
		std::string	_GuildName;
		// 
		uint32	_ShardId;

		friend class CCharacter;

		std::vector < CCharacterPtr >	*_Characters;

		friend class CGuildInvite;

		std::vector < CGuildInvitePtr >	*_Invites;
	public:
		// 
		const std::string &getGuildName() const
		{
			return _GuildName;
		}



		void setGuildName(const std::string &value)
		{

			if (_GuildName != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_GuildName = value;

				
			}

		}
			// 
		uint32 getShardId() const
		{
			return _ShardId;
		}

		void setShardId(uint32 value)
		{

			if (_ShardId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ShardId = value;

			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CCharacterPtr> &getCharacters() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CCharacterPtr &getCharactersByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CCharacterPtr &getCharactersById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CGuildInvitePtr> &getInvites() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CGuildInvitePtr &getInvitesByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CGuildInvitePtr &getInvitesById(uint32 id) const;

	
		bool operator == (const CGuild &other) const
		{
			return _GuildId == other._GuildId
				&& _GuildName == other._GuildName
				&& _ShardId == other._ShardId;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CGuild()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_GuildId(NOPE::INVALID_OBJECT_ID)
		{
			_Characters = NULL;
			_Invites = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CGuild();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CGuildPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CGuildPtr(new CGuild(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CGuildPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CShard and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCShard(MSW::CConnection &connection, uint32 parentId, std::map < uint32, CGuildPtr > &children, const char *filename, uint32 lineNum);

		/// Load Characters child(ren) object(s).
		bool loadCharacters(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load Invites child(ren) object(s).
		bool loadInvites(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CGuildPtr;

		typedef std::map<uint32, CGuild*>	TObjectCache;
		typedef std::set<CGuild*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CGuildPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CGuild *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CGuildPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CGuildPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _GuildId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_GuildId == NOPE::INVALID_OBJECT_ID);
			_GuildId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGuildInvite
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_GuildId;
		// 
		TSessionId	_SessionId;
	public:
		// 
		uint32 getGuildId() const
		{
			return _GuildId;
		}

		void setGuildId(uint32 value)
		{

			if (_GuildId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_GuildId = value;

			}

		}
			// 
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

			if (_SessionId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SessionId = value;

			}

		}
	
		bool operator == (const CGuildInvite &other) const
		{
			return _Id == other._Id
				&& _GuildId == other._GuildId
				&& _SessionId == other._SessionId;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CGuildInvite()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CGuildInvite();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CGuildInvitePtr createTransient(const char *filename, uint32 lineNum)
		{
			return CGuildInvitePtr(new CGuildInvite(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CGuildInvitePtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CGuild and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCGuild(MSW::CConnection &connection, uint32 parentId, std::vector < CGuildInvitePtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CSession and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CGuildInvitePtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CGuildInvitePtr;

		typedef std::map<uint32, CGuildInvite*>	TObjectCache;
		typedef std::set<CGuildInvite*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CGuildInvitePtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CGuildInvite *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CGuildInvitePtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CGuildInvitePtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CPlayerRating
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_ScenarioId;
		// 
		uint32	_Author;
		// 
		uint32	_RateFun;
		// 
		uint32	_RateDifficulty;
		// 
		uint32	_RateAccessibility;
		// 
		uint32	_RateOriginality;
		// 
		uint32	_RateDirection;
	public:
		// 
		uint32 getScenarioId() const
		{
			return _ScenarioId;
		}

		void setScenarioId(uint32 value)
		{

			if (_ScenarioId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ScenarioId = value;

			}

		}
			// 
		uint32 getAuthor() const
		{
			return _Author;
		}

		void setAuthor(uint32 value)
		{

			if (_Author != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Author = value;

			}

		}
			// 
		uint32 getRateFun() const
		{
			return _RateFun;
		}

		void setRateFun(uint32 value)
		{

			if (_RateFun != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RateFun = value;

			}

		}
			// 
		uint32 getRateDifficulty() const
		{
			return _RateDifficulty;
		}

		void setRateDifficulty(uint32 value)
		{

			if (_RateDifficulty != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RateDifficulty = value;

			}

		}
			// 
		uint32 getRateAccessibility() const
		{
			return _RateAccessibility;
		}

		void setRateAccessibility(uint32 value)
		{

			if (_RateAccessibility != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RateAccessibility = value;

			}

		}
			// 
		uint32 getRateOriginality() const
		{
			return _RateOriginality;
		}

		void setRateOriginality(uint32 value)
		{

			if (_RateOriginality != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RateOriginality = value;

			}

		}
			// 
		uint32 getRateDirection() const
		{
			return _RateDirection;
		}

		void setRateDirection(uint32 value)
		{

			if (_RateDirection != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RateDirection = value;

			}

		}
	
		bool operator == (const CPlayerRating &other) const
		{
			return _Id == other._Id
				&& _ScenarioId == other._ScenarioId
				&& _Author == other._Author
				&& _RateFun == other._RateFun
				&& _RateDifficulty == other._RateDifficulty
				&& _RateAccessibility == other._RateAccessibility
				&& _RateOriginality == other._RateOriginality
				&& _RateDirection == other._RateDirection;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CPlayerRating()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CPlayerRating();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CPlayerRatingPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CPlayerRatingPtr(new CPlayerRating(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CPlayerRatingPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CScenario and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCScenario(MSW::CConnection &connection, uint32 parentId, std::vector < CPlayerRatingPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CCharacter and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCCharacter(MSW::CConnection &connection, uint32 parentId, std::vector < CPlayerRatingPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CPlayerRatingPtr;

		typedef std::map<uint32, CPlayerRating*>	TObjectCache;
		typedef std::set<CPlayerRating*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CPlayerRatingPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CPlayerRating *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CPlayerRatingPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CPlayerRatingPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	

	struct TJournalEntryType
	{
		enum TValues
		{
			jet_credits = 1,
			jet_notes,
			/// the highest valid value in the enum
			last_enum_item = jet_notes,
			/// a value equal to the last enum item +1
			end_of_enum,

			invalid_val,
			
			/// Number of enumerated values
			nb_enum_items = 2
		};
		
		/// Index table to convert enum value to linear index table
		const std::map<TValues, uint32> &getIndexTable() const
		{
			static std::map<TValues, uint32> indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
				indexTable.insert(std::make_pair(jet_credits, 0));
				indexTable.insert(std::make_pair(jet_notes, 1));
			
				init = true;
			}

			return indexTable;
		}
		

		static const NLMISC::CStringConversion<TValues> &getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
				NL_STRING_CONVERSION_TABLE_ENTRY(jet_credits)
				NL_STRING_CONVERSION_TABLE_ENTRY(jet_notes)
				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};                                                                                             
			static NLMISC::CStringConversion<TValues>                                                                
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)   
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		TJournalEntryType()
			: _Value(invalid_val)
		{
		}
		TJournalEntryType(TValues value)
			: _Value(value)
		{
		}

		TJournalEntryType(const std::string &str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const TJournalEntryType &other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const TJournalEntryType &other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator < (const TJournalEntryType &other) const
		{
			return _Value < other._Value;
		}

		bool operator <= (const TJournalEntryType &other) const
		{
			return _Value <= other._Value;
		}

		bool operator > (const TJournalEntryType &other) const
		{
			return !(_Value <= other._Value);
		}
		bool operator >= (const TJournalEntryType &other) const
		{
			return !(_Value < other._Value);
		}

		const std::string &toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &toString(TValues value)
		{
			return getConversionTable().toString(value);
		}

		TValues getValue() const
		{
			return _Value;
		}

		// return true if the actual value of the enum is valid, otherwise false
		bool isValid()
		{
			if (_Value == invalid_val)
				return false;

			// not invalid, check other enum value
			return getConversionTable().isValid(_Value);
		}

		
		uint32 asIndex()
		{
			std::map<TValues, uint32>::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		
	};
		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CJournalEntry
	{
	protected:
		// 
		uint32	_Id;
		// 
		TSessionId	_SessionId;
		// 
		uint32	_Author;
		// 
		TJournalEntryType	_Type;
		// 
		std::string	_Text;
		// 
		uint32	_TimeStamp;
	public:
		// 
		TSessionId getSessionId() const
		{
			return _SessionId;
		}

		void setSessionId(TSessionId value)
		{

			if (_SessionId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_SessionId = value;

			}

		}
			// 
		uint32 getAuthor() const
		{
			return _Author;
		}

		void setAuthor(uint32 value)
		{

			if (_Author != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Author = value;

			}

		}
			// 
		TJournalEntryType getType() const
		{
			return _Type;
		}

		void setType(TJournalEntryType value)
		{

			if (_Type != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Type = value;

			}

		}
			// 
		const std::string &getText() const
		{
			return _Text;
		}



		void setText(const std::string &value)
		{

			if (_Text != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Text = value;

				
			}

		}
			// 
		uint32 getTimeStamp() const
		{
			return _TimeStamp;
		}

		void setTimeStamp(uint32 value)
		{

			if (_TimeStamp != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_TimeStamp = value;

			}

		}
	
		bool operator == (const CJournalEntry &other) const
		{
			return _Id == other._Id
				&& _SessionId == other._SessionId
				&& _Author == other._Author
				&& _Type == other._Type
				&& _Text == other._Text
				&& _TimeStamp == other._TimeStamp;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CJournalEntry()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CJournalEntry();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CJournalEntryPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CJournalEntryPtr(new CJournalEntry(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CJournalEntryPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CSession and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCSession(MSW::CConnection &connection, uint32 parentId, std::vector < CJournalEntryPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CJournalEntryPtr;

		typedef std::map<uint32, CJournalEntry*>	TObjectCache;
		typedef std::set<CJournalEntry*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CJournalEntryPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CJournalEntry *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CJournalEntryPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CJournalEntryPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFolder
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_Author;
		// 
		std::string	_Title;
		// 
		std::string	_Comments;

		friend class CFolderAccess;

		std::vector < CFolderAccessPtr >	*_FolderAccess;

		friend class CSession;

		std::vector < CSessionPtr >	*_Sessions;
	public:
		// 
		uint32 getAuthor() const
		{
			return _Author;
		}

		void setAuthor(uint32 value)
		{

			if (_Author != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Author = value;

			}

		}
			// 
		const std::string &getTitle() const
		{
			return _Title;
		}



		void setTitle(const std::string &value)
		{

			if (_Title != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Title = value;

				
			}

		}
			// 
		const std::string &getComments() const
		{
			return _Comments;
		}



		void setComments(const std::string &value)
		{

			if (_Comments != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Comments = value;

				
			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CFolderAccessPtr> &getFolderAccess() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CFolderAccessPtr &getFolderAccessByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CFolderAccessPtr &getFolderAccessById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CSessionPtr> &getSessions() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CSessionPtr &getSessionsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CSessionPtr &getSessionsById(uint32 id) const;

	
		bool operator == (const CFolder &other) const
		{
			return _Id == other._Id
				&& _Author == other._Author
				&& _Title == other._Title
				&& _Comments == other._Comments;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CFolder()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{
			_FolderAccess = NULL;
			_Sessions = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CFolder();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CFolderPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CFolderPtr(new CFolder(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CFolderPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRingUser and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderPtr > &children, const char *filename, uint32 lineNum);

		/// Load FolderAccess child(ren) object(s).
		bool loadFolderAccess(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load Sessions child(ren) object(s).
		bool loadSessions(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CFolderPtr;

		typedef std::map<uint32, CFolder*>	TObjectCache;
		typedef std::set<CFolder*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CFolderPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CFolder *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CFolderPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CFolderPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CFolderAccess
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_FolderId;
		// 
		uint32	_UserId;
	public:
		// 
		uint32 getFolderId() const
		{
			return _FolderId;
		}

		void setFolderId(uint32 value)
		{

			if (_FolderId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_FolderId = value;

			}

		}
			// 
		uint32 getUserId() const
		{
			return _UserId;
		}

		void setUserId(uint32 value)
		{

			if (_UserId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_UserId = value;

			}

		}
	
		bool operator == (const CFolderAccess &other) const
		{
			return _Id == other._Id
				&& _FolderId == other._FolderId
				&& _UserId == other._UserId;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CFolderAccess()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CFolderAccess();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CFolderAccessPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CFolderAccessPtr(new CFolderAccess(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CFolderAccessPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CRingUser and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCRingUser(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderAccessPtr > &children, const char *filename, uint32 lineNum);

		/** Load all objects children of CFolder and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCFolder(MSW::CConnection &connection, uint32 parentId, std::vector < CFolderAccessPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CFolderAccessPtr;

		typedef std::map<uint32, CFolderAccess*>	TObjectCache;
		typedef std::set<CFolderAccess*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CFolderAccessPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CFolderAccess *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CFolderAccessPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CFolderAccessPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CScenario
	{
	protected:
		// 
		uint32	_Id;
		// 
		NLMISC::CHashKeyMD5	_MD5;
		// 
		std::string	_Title;
		// 
		std::string	_Description;
		// 
		std::string	_Author;
		// 
		uint32	_RRPTotal;
		// 
		TAnimMode	_AnimMode;
		// 
		std::string	_Language;
		// 
		TSessionOrientation	_Orientation;
		// 
		R2::TSessionLevel	_Level;
		// 
		bool	_AllowFreeTrial;

		friend class CSessionLog;

		std::vector < CSessionLogPtr >	*_SessionLogs;

		friend class CPlayerRating;

		std::vector < CPlayerRatingPtr >	*_PlayerRatings;
	public:
		// 
		const NLMISC::CHashKeyMD5 &getMD5() const
		{
			return _MD5;
		}



		void setMD5(const NLMISC::CHashKeyMD5 &value)
		{

			if (_MD5 != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_MD5 = value;

				
			}

		}
			// 
		const std::string &getTitle() const
		{
			return _Title;
		}



		void setTitle(const std::string &value)
		{

			if (_Title != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Title = value;

				
			}

		}
			// 
		const std::string &getDescription() const
		{
			return _Description;
		}



		void setDescription(const std::string &value)
		{

			if (_Description != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Description = value;

				
			}

		}
			// 
		const std::string &getAuthor() const
		{
			return _Author;
		}



		void setAuthor(const std::string &value)
		{

			if (_Author != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Author = value;

				
			}

		}
			// 
		uint32 getRRPTotal() const
		{
			return _RRPTotal;
		}

		void setRRPTotal(uint32 value)
		{

			if (_RRPTotal != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RRPTotal = value;

			}

		}
			// 
		TAnimMode getAnimMode() const
		{
			return _AnimMode;
		}

		void setAnimMode(TAnimMode value)
		{

			if (_AnimMode != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_AnimMode = value;

			}

		}
			// 
		const std::string &getLanguage() const
		{
			return _Language;
		}



		void setLanguage(const std::string &value)
		{

			if (_Language != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Language = value;

				
			}

		}
			// 
		TSessionOrientation getOrientation() const
		{
			return _Orientation;
		}

		void setOrientation(TSessionOrientation value)
		{

			if (_Orientation != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Orientation = value;

			}

		}
			// 
		R2::TSessionLevel getLevel() const
		{
			return _Level;
		}

		void setLevel(R2::TSessionLevel value)
		{

			if (_Level != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Level = value;

			}

		}
			// 
		bool getAllowFreeTrial() const
		{
			return _AllowFreeTrial;
		}

		void setAllowFreeTrial(bool value)
		{

			if (_AllowFreeTrial != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_AllowFreeTrial = value;

			}

		}
	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CSessionLogPtr> &getSessionLogs() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CSessionLogPtr &getSessionLogsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CSessionLogPtr &getSessionLogsById(uint32 id) const;

	
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector<CPlayerRatingPtr> &getPlayerRatings() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		CPlayerRatingPtr &getPlayerRatingsByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		CPlayerRatingPtr &getPlayerRatingsById(uint32 id) const;

	
		bool operator == (const CScenario &other) const
		{
			return _Id == other._Id
				&& _MD5 == other._MD5
				&& _Title == other._Title
				&& _Description == other._Description
				&& _Author == other._Author
				&& _RRPTotal == other._RRPTotal
				&& _AnimMode == other._AnimMode
				&& _Language == other._Language
				&& _Orientation == other._Orientation
				&& _Level == other._Level
				&& _AllowFreeTrial == other._AllowFreeTrial;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CScenario()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_RRPTotal = 0;
			_AllowFreeTrial = false;
			_SessionLogs = NULL;
			_PlayerRatings = NULL;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CScenario();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CScenarioPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CScenarioPtr(new CScenario(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CScenarioPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/// Load SessionLogs child(ren) object(s).
		bool loadSessionLogs(MSW::CConnection &connection, const char *filename, uint32 lineNum);

		/// Load PlayerRatings child(ren) object(s).
		bool loadPlayerRatings(MSW::CConnection &connection, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CScenarioPtr;

		typedef std::map<uint32, CScenario*>	TObjectCache;
		typedef std::set<CScenario*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CScenarioPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CScenario *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CScenarioPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CScenarioPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			nlassert(getPersistentState() != NOPE::os_transient);
			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CSessionLog
	{
	protected:
		// 
		uint32	_Id;
		// 
		uint32	_ScenarioId;
		// 
		uint32	_RRPScored;
		// 
		uint32	_ScenarioPointScored;
		// 
		uint32	_TimeTaken;
		// 
		std::string	_Participants;
		// 
		uint32	_LaunchDate;
		// 
		std::string	_Owner;
		// 
		std::string	_GuildName;
	public:
		// 
		uint32 getScenarioId() const
		{
			return _ScenarioId;
		}

		void setScenarioId(uint32 value)
		{

			if (_ScenarioId != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ScenarioId = value;

			}

		}
			// 
		uint32 getRRPScored() const
		{
			return _RRPScored;
		}

		void setRRPScored(uint32 value)
		{

			if (_RRPScored != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_RRPScored = value;

			}

		}
			// 
		uint32 getScenarioPointScored() const
		{
			return _ScenarioPointScored;
		}

		void setScenarioPointScored(uint32 value)
		{

			if (_ScenarioPointScored != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_ScenarioPointScored = value;

			}

		}
			// 
		uint32 getTimeTaken() const
		{
			return _TimeTaken;
		}

		void setTimeTaken(uint32 value)
		{

			if (_TimeTaken != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_TimeTaken = value;

			}

		}
			// 
		const std::string &getParticipants() const
		{
			return _Participants;
		}



		void setParticipants(const std::string &value)
		{

			if (_Participants != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Participants = value;

				
			}

		}
			// 
		uint32 getLaunchDate() const
		{
			return _LaunchDate;
		}

		void setLaunchDate(uint32 value)
		{

			if (_LaunchDate != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_LaunchDate = value;

			}

		}
			// 
		const std::string &getOwner() const
		{
			return _Owner;
		}



		void setOwner(const std::string &value)
		{

			if (_Owner != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_Owner = value;

				
			}

		}
			// 
		const std::string &getGuildName() const
		{
			return _GuildName;
		}



		void setGuildName(const std::string &value)
		{

			if (_GuildName != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);


				_GuildName = value;

				
			}

		}
	
		bool operator == (const CSessionLog &other) const
		{
			return _Id == other._Id
				&& _ScenarioId == other._ScenarioId
				&& _RRPScored == other._RRPScored
				&& _ScenarioPointScored == other._ScenarioPointScored
				&& _TimeTaken == other._TimeTaken
				&& _Participants == other._Participants
				&& _LaunchDate == other._LaunchDate
				&& _Owner == other._Owner
				&& _GuildName == other._GuildName;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CSessionLog()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_Id(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_RRPScored = 0;
			_ScenarioPointScored = 0;
			_TimeTaken = 0;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CSessionLog();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CSessionLogPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CSessionLogPtr(new CSessionLog(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CSessionLogPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load all objects children of CScenario and
		 *	return them by using the specified output iterator.
		 */

		static bool loadChildrenOfCScenario(MSW::CConnection &connection, uint32 parentId, std::vector < CSessionLogPtr > &children, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CSessionLogPtr;

		typedef std::map<uint32, CSessionLog*>	TObjectCache;
		typedef std::set<CSessionLog*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CSessionLogPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CSessionLog *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CSessionLogPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CSessionLogPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _Id;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_Id == NOPE::INVALID_OBJECT_ID);
			_Id = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


		/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class CGmStatus
	{
	protected:
		// 
		uint32	_UserId;
		// 
		bool	_Available;
	public:
		// 
		bool getAvailable() const
		{
			return _Available;
		}

		void setAvailable(bool value)
		{

			if (_Available != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);

				_Available = value;

			}

		}
	
		bool operator == (const CGmStatus &other) const
		{
			return _UserId == other._UserId
				&& _Available == other._Available;
		}


	private:
		// private constructor, you must use 'createTransient' to get an instance
		CGmStatus()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_UserId(NOPE::INVALID_OBJECT_ID)
		{
			// Default initialisation
			_Available = 1;

			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~CGmStatus();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static CGmStatusPtr createTransient(const char *filename, uint32 lineNum)
		{
			return CGmStatusPtr(new CGmStatus(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static CGmStatusPtr load(MSW::CConnection &connection, uint32 id, const char *filename, uint32 lineNum);


		/** Load the object child of CRingUser and
		 *	return true if no error, false in case of error (in SQL maybe).
		 *	If no such object is found, fill the child pointer with NULL.
		 */
		static bool loadChildOfCRingUser(MSW::CConnection &connection, uint32 parentId, CGmStatusPtr &childPtr, const char *filename, uint32 lineNum);


	private:
	
	private:
		friend class CPersistentCache;
		friend class CGmStatusPtr;

		typedef std::map<uint32, CGmStatus*>	TObjectCache;
		typedef std::set<CGmStatus*>			TObjectSet;
		typedef std::map<time_t, TObjectSet>	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		CGmStatusPtr		*_PtrList;			

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static CGmStatus *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(CGmStatusPtr *ptr);

		// return the first pointer of the pointer list (can be null)
		CGmStatusPtr *getFirstPtr()
		{
			return _PtrList;
		}

	public:
	
		/** Return the object identifier (witch is unique)
		 *	You can only call this method on a persistent instance.
		 *	(because transient instance can have invalid id)
		 */
		uint32 getObjectId() const
		{

			return _UserId;
		}

		/** Set the object unique ID.
		 *	You can only set the object id on a transient object
		 *	having a non autogenerated unique id.
		 *	Furthermore, you MUST set the id before calling create()
		 *	if the id is not autogenerated.
		 */
		void setObjectId(uint32 objectId)
		{
			// can only be set when in transient state
			nlassert(getPersistentState() == NOPE::os_transient);
			// can only be set once
			nlassert(_UserId == NOPE::INVALID_OBJECT_ID);
			_UserId = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);



	};


	
}
	
#endif

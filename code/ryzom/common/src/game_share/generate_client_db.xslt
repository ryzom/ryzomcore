<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="text" indent="no"/>

	<!-- Output : can be 'header', 'cpp' or 'php' -->
	<xsl:param name="output"/>		<!-- select="'header' -->
	<xsl:param name="side"/>		<!-- The generation side (my be 'server' or 'client' -->
	<xsl:param name="filename"/>
	<xsl:param name="bank"/>		<!-- The database bank to generate (must meet an entry in <bank_superclass> banks) -->

	<!-- A special template applyer that is mode aware -->
	<xsl:template name="myApplyTemplate">
		<xsl:choose>
			<xsl:when test="$output = 'header'">
				<xsl:if test="$side = 'server'">
					<xsl:apply-templates mode="header-server"/>
				</xsl:if>
				<xsl:if test="$side = 'client'">
					<xsl:apply-templates mode="header-client"/>
				</xsl:if>
			</xsl:when>
			<xsl:when test="$output = 'cpp'">
				<xsl:if test="$side = 'server'">
					<xsl:apply-templates mode="cpp-server"/>
				</xsl:if>
				<xsl:if test="$side = 'client'">
					<xsl:apply-templates mode="cpp-client"/>
				</xsl:if>
			</xsl:when>
		</xsl:choose>
	</xsl:template>

	<!-- some stupide template to remove unwanted text from output -->
	<xsl:template match="text()" mode="cpp-server"/>
	<xsl:template match="text()" mode="header-server"/>
	<xsl:template match="text()" mode="cpp-client"/>
	<xsl:template match="text()" mode="header-client"/>

	<xsl:template match="/"><xsl:call-template name="myApplyTemplate"/></xsl:template>

	<!-- ######################################################### -->
	<!-- #####   Recursive build of full class name        ####### -->
	<!-- ######################################################### -->
	<xsl:template name="makeFullClassName">
		<xsl:if test="name() = 'branch'">
			<xsl:for-each select="..">
				<xsl:call-template name="makeFullClassName"/>
			</xsl:for-each>
			<xsl:text>::</xsl:text><xsl:call-template name="makeBranchType"/>
		</xsl:if>
		<xsl:if test="name() = 'database_description'">
			<xsl:text>CBankAccessor_</xsl:text><xsl:value-of select="$bank"/>
		</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Root template matcher  (cpp-server) ####### -->
	<!-- ######################################################### -->
	<xsl:template match="database_description" mode="cpp-server">
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////
#include "stdpch.h"
#include "database_<xsl:value-of select="translate($bank, 'ABCDEFGHIJKLMNOPQRSTUVWXYZ', 'abcdefghijklmnopqrstuvwxyz')"/>.h"

	<xsl:for-each select="bank_superclass/bank[@name = $bank]">
		<xsl:variable name="bankName" select="@name"/>

TCDBBank CBankAccessor_<xsl:value-of select="@name"/>::BankTag;

		<!-- generate root class members-->
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
<xsl:call-template name="makeFullClassName"/><xsl:text>	</xsl:text><xsl:for-each select=".."><xsl:call-template name="makeFullClassName"/></xsl:for-each>::_<xsl:call-template name="makeBranchName"/><xsl:if test="@count">[<xsl:value-of select="@count"/>]</xsl:if><xsl:text>;
</xsl:text>
		</xsl:for-each>

<!-- generate init method -->
void CBankAccessor_<xsl:value-of select="@name"/>::init()
{
	static bool inited = false;
	if (!inited)
	{
		// retreive the bank structure
		CCDBStructBanks	*bank = CCDBStructBanks::instance();
		BankTag = CCDBStructBanks::readBankName("<xsl:value-of select="$bankName"/>");

		ICDBStructNode *node;

		// branch init
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
		<xsl:if test="not(@count)">
		node  = bank->getICDBStructNodeFromName( BankTag, "<xsl:value-of select="@name"/>" );
		nlassert(node != NULL);
		// call sub branch init
		_<xsl:call-template name="makeBranchName"/>.init(node);
		</xsl:if>
		<xsl:if test="@count">
		for (uint i=0; i&lt;<xsl:value-of select="@count"/>; ++i)
		{
			node  = bank->getICDBStructNodeFromName( BankTag, NLMISC::toString("<xsl:value-of select="@name"/>%u", i) );
			nlassert(node != NULL);
			// call sub branch init
			_<xsl:call-template name="makeBranchName"/>[i].init(node, i);
		}
		</xsl:if>
		</xsl:for-each>

		inited = true;
	}
}
		<!-- generate inner classes methods -->
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
			<xsl:call-template name="branch_cpp"/>
		</xsl:for-each>

	</xsl:for-each>
	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         Root template matcher               ####### -->
	<!-- ######################################################### -->
	<xsl:template match="database_description" mode="header-server">

#ifndef INCLUDED_<xsl:value-of select="concat($filename, '_', $bank)"/>_H
#define INCLUDED_<xsl:value-of select="concat($filename, '_', $bank)"/>_H
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////

#include "nel/misc/string_common.h"
#include "cdb_group.h"
#include "player_manager/cdb.h"
#include "player_manager/cdb_synchronised.h"

<xsl:for-each select="bank_superclass/bank[@name = $bank]">
	<xsl:for-each select="include">
#include "<xsl:value-of select="@file"/>"
	</xsl:for-each>
<!-- copy any verbatime code -->
<xsl:value-of select="verbatime"/>
</xsl:for-each>

<![CDATA[
#ifndef _SET_PROP_ACCESSOR_
#define _SET_PROP_ACCESSOR_
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, bool value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint8 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint16 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint32 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, uint64 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint8 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint16 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint32 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, sint64 value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value), forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const std::string &value, bool forceSending = false)
{
	db.x_setPropString(node, value, forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const ucstring &value, bool forceSending = false)
{
	db.x_setPropString(node, value, forceSending);
}
inline void _setProp(CCDBSynchronised &db, ICDBStructNode *node, const NLMISC::CSheetId &value, bool forceSending = false)
{
	db.x_setProp(node, uint64(value.asInt()), forceSending);
}


inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, bool &value)
{
	value = db.x_getProp(node) != 0;
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint8 &value)
{
	value = uint8(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint16 &value)
{
	value = uint16(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint32 &value)
{
	value = uint32(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, uint64 &value)
{
	value = db.x_getProp(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint8 &value)
{
	value = uint8(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint16 &value)
{
	value = uint16(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint32 &value)
{
	value = uint32(db.x_getProp(node));
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, sint64 &value)
{
	value = db.x_getProp(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, std::string &value)
{
	value = db.x_getPropString(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, ucstring &value)
{
	value = db.x_getPropUcstring(node);
}
inline void _getProp(const CCDBSynchronised &db, ICDBStructNode *node, NLMISC::CSheetId &value)
{
	value = uint32(db.x_getProp(node));
}
#endif // _SET_PROP_ACCESSOR_
]]>

	<xsl:for-each select="bank_superclass/bank[@name = $bank]">
		<xsl:variable name="bankName" select="@name"/>
	class CBankAccessor_<xsl:value-of select="@name"/> : public <xsl:value-of select="@class"/>
	{
	public:
		static TCDBBank BankTag;

		<!-- generate inner classes -->
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
			<xsl:call-template name="branch_header"/>
		</xsl:for-each>

		<!-- generate branch class instance -->
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
		static <xsl:call-template name="makeBranchType"/>	_<xsl:call-template name="makeBranchName"/><xsl:if test="@count">[<xsl:value-of select="@count"/>]</xsl:if><xsl:text>;
</xsl:text>
		</xsl:for-each>

	public:

		// Constructor
		CBankAccessor_<xsl:value-of select="@name"/>()
		{
			// make sure the static tree is initialised (some kind of lazy initialisation)
			init();

			// init the base class
			<xsl:value-of select="@class"/>::init(BankTag);
		}

		<!-- generate init method -->
		static void init();

		<!-- generate branch accessors -->
		<xsl:for-each select="/*/branch[@bank = $bankName and not(@clientonly)]">
			<xsl:call-template name="branchAccess"/>
		</xsl:for-each>

	};
	</xsl:for-each>
<xsl:call-template name="myApplyTemplate"/>

#endif // INCLUDED_<xsl:value-of select="concat($filename, '_', $bank)"/><xsl:text>_H
</xsl:text>

	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         branch template	(header)		   ####### -->
	<!-- ######################################################### -->
	<xsl:template name="branch_header" match="IDONTKNOWWHATSHOULDGOHERE" mode="header-server">
		<xsl:if test="not(@clientonly='1')">
	class <xsl:call-template name="makeBranchType"/>
	{
	public:
		<!-- generate sub class member -->
		<xsl:for-each select="branch[not(@clientonly)]"><xsl:call-template name="branch_header" /></xsl:for-each>

	private:
		ICDBStructNode	*_BranchNode;

		<!-- generate leaf -->
		<xsl:for-each select="leaf[not(@clientonly)]">
			<xsl:call-template name="leaf"/>
		</xsl:for-each>

		<!-- generate branch -->
		<xsl:for-each select="branch[not(@clientonly)]">
			<xsl:call-template name="branchInstance"/>
		</xsl:for-each>

	public:
		void init(ICDBStructNode *parent<xsl:if test="@count">, uint index</xsl:if>);

		// accessor to branch node
		ICDBStructNode *getCDBNode()
		{
			return _BranchNode;
		}

		<!-- generate accessors -->
		<xsl:for-each select="leaf[not(@clientonly)]">
			<xsl:call-template name="leafAccess"/>
		</xsl:for-each>
		<xsl:for-each select="branch[not(@clientonly)]">
			<xsl:call-template name="branchAccess"/>
		</xsl:for-each>
	};
		</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         branch template	(cpp)			   ####### -->
	<!-- ######################################################### -->
	<xsl:template name="branch_cpp">
		<xsl:if test="not(@clientonly='1')">


<!--ICDBStructNode	*<xsl:call-template name="makeFullClassName"/>::_BranchNode;-->

		<!-- generate class members-->
<!--
		<xsl:for-each select="branch[not(@clientonly)]">
<xsl:call-template name="makeFullClassName"/><xsl:text>	</xsl:text><xsl:for-each select=".."><xsl:call-template name="makeFullClassName"/></xsl:for-each>::_<xsl:call-template name="makeBranchName"/><xsl:if test="@count">[<xsl:value-of select="@count"/>]</xsl:if><xsl:text>;
</xsl:text>
		</xsl:for-each>
		<xsl:for-each select="leaf[not(@clientonly)]">
ICDBStructNode	*<xsl:for-each select=".."><xsl:call-template name="makeFullClassName"/>::</xsl:for-each>_<xsl:value-of select="@name"/><xsl:if test="@count">[<xsl:value-of select="@count"/>]</xsl:if><xsl:text>;
</xsl:text>
		</xsl:for-each>
-->

<!-- generate init method -->
void <xsl:call-template name="makeFullClassName"/>::init(ICDBStructNode *parent<xsl:if test="@count">, uint index</xsl:if>)
{
	ICDBStructNode *node = parent;

	_BranchNode = node;

	// leaf init
	<xsl:for-each select="leaf[not(@clientonly)]">
	<xsl:if test="not(@count)">
	node  = parent->getNode( ICDBStructNode::CTextId("<xsl:value-of select="@name"/>"), false );
	nlassert(node != NULL);
	_<xsl:value-of select="@name"/> = node;
	</xsl:if>
	<xsl:if test="@count">
	for (uint i=0; i&lt;<xsl:value-of select="@count"/>; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("<xsl:value-of select="@name"/>%u", i)), false );
		nlassert(node != NULL);
		_<xsl:value-of select="@name"/>[i] = node;
	}
	</xsl:if>
	</xsl:for-each>

	// branch init
	<xsl:for-each select="branch[not(@clientonly)]">
	<xsl:if test="not(@count)">
	node  = parent->getNode( ICDBStructNode::CTextId("<xsl:value-of select="@name"/>"), false );
	nlassert(node != NULL);
	_<xsl:call-template name="makeBranchName"/>.init(node);
	</xsl:if>
	<xsl:if test="@count">
	for (uint i=0; i&lt;<xsl:value-of select="@count"/>; ++i)
	{
		node  = parent->getNode( ICDBStructNode::CTextId(NLMISC::toString("<xsl:value-of select="@name"/>%u", i)), false );
		nlassert(node != NULL);
		_<xsl:call-template name="makeBranchName"/>[i].init(node, i);
	}
	</xsl:if>
	</xsl:for-each>
}

<!-- generate inner classes methods -->
<xsl:for-each select="branch[not(@clientonly)]">
	<xsl:call-template name="branch_cpp"/>
</xsl:for-each>

	</xsl:if>
	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         generate branch accessor			###### -->
	<!-- ######################################################### -->
	<xsl:template name="branchAccess">
		<xsl:if test="not(@count)">
		<xsl:if test="name(..) = 'database_description'">static </xsl:if><xsl:call-template name="makeBranchType"/> &amp;get<xsl:call-template name="makeBranchName"/>()
		{
			return _<xsl:call-template name="makeBranchName"/>;
		}
		</xsl:if>
		<xsl:if test="@count">
		<xsl:if test="name(..) = 'database_description'">static </xsl:if><xsl:call-template name="makeBranchType"/> &amp;get<xsl:call-template name="makeBranchName"/>(uint32 index)
		{
			nlassert(index &lt; <xsl:value-of select="@count"/>);
			return _<xsl:call-template name="makeBranchName"/>[index];
		}
		</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         generate leaf accessor			###### -->
	<!-- ######################################################### -->
	<xsl:template name="leafAccess">
		<xsl:variable name="bankName" select="ancestor::*/@bank"/>
		void set<xsl:value-of select="@name"/>(<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@class"/> &amp;dbGroup, <xsl:call-template name="makeLeafParamType"><xsl:with-param name="leafType" select="@type"/><xsl:with-param name="cppType" select="@cppType"/></xsl:call-template> value, bool forceSending = false)
		{
			<xsl:if test="not(@cppType) and not(substring(@type, 2) = '1' or substring(@type, 2) =  '8' or substring(@type, 2) = '16' or substring(@type, 2) = '32' or substring(@type, 2) = '64')">
				<!-- we can only test validity on simple types that are not of standard bit size-->
				<xsl:if test="(substring(@type, 1, 1) = 'I' or substring(@type, 1, 1) = 'U')">
					<!-- unsigned value -->
			// Check that the value is not out of database precision
			STOP_IF(value &gt; (1&lt;&lt;<xsl:value-of select="substring(@type, 2)"/>)-1, "set<xsl:value-of select="@name"/> : Value out of bound : trying to store "&lt;&lt;value&lt;&lt;" in a unsigned field limited to <xsl:value-of select="substring(@type, 2)"/> bits");
				</xsl:if>
				<xsl:if test="substring(@type, 1, 1) = 'S'">
					<!-- signed value -->
			// Check that the value is not out of database precision
			if (value > 0)
				STOP_IF(value &gt; (1&lt;&lt;(<xsl:value-of select="substring(@type, 2)"/>-1))-1, "set<xsl:value-of select="@name"/> : Value out of bound : trying to store "&lt;&lt;value&lt;&lt;" in a signed field limited to <xsl:value-of select="substring(@type, 2)"/> bits");
			else
				STOP_IF(value &lt; -(1&lt;&lt;(<xsl:value-of select="substring(@type, 2)"/>-1)), "set<xsl:value-of select="@name"/> : Value out of bound : trying to store "&lt;&lt;value&lt;&lt;" in a signed field limited to <xsl:value-of select="substring(@type, 2)"/> bits");
				</xsl:if>
			</xsl:if>

			_setProp(dbGroup<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@dbAccessor"/>, _<xsl:value-of select="@name"/>, value, forceSending);
		}

		<xsl:call-template name="makeLeafParamType"><xsl:with-param name="leafType" select="@type"/><xsl:with-param name="cppType" select="@cppType"/></xsl:call-template> get<xsl:value-of select="@name"/>(const <xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@class"/> &amp;dbGroup)
		{
			<xsl:call-template name="makeLeafType"><xsl:with-param name="leafType" select="@type"/><xsl:with-param name="cppType" select="@cppType"/></xsl:call-template> value;
			_getProp(dbGroup<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@dbAccessor"/>, _<xsl:value-of select="@name"/>, value);

			return value;
		}
		<xsl:if test="@type = 'TEXT'">
		<!-- generate int accessor for text type -->
		void set<xsl:value-of select="@name"/>(<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@class"/> &amp;dbGroup, uint32 stringId, bool forceSending = false)
		{
			_setProp(dbGroup<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@dbAccessor"/>, _<xsl:value-of select="@name"/>, stringId, forceSending);
		}
		uint32 get<xsl:value-of select="@name"/>_id(const <xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@class"/> &amp;dbGroup)
		{
			uint32 value;
			_getProp(dbGroup<xsl:value-of select="/*/bank_superclass/bank[@name = $bankName]/@dbAccessor"/>, _<xsl:value-of select="@name"/>, value);

			return value;
		}
		</xsl:if>
		ICDBStructNode *get<xsl:value-of select="@name"/>CDBNode()
		{
			return _<xsl:value-of select="@name"/>;
		}
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         generate leaf declaration			###### -->
	<!-- ######################################################### -->
	<xsl:template name="leaf">
		<xsl:if test="@count">
			<xsl:text>ICDBStructNode	*_</xsl:text><xsl:value-of select="@name"/>[<xsl:value-of select="@count"/>];
		</xsl:if>
		<xsl:if test="not(@count)">
			<xsl:text>ICDBStructNode	*_</xsl:text><xsl:value-of select="@name"/>;
		</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         generate branch declaration			###### -->
	<!-- ######################################################### -->
	<xsl:template name="branchInstance">
		<xsl:if test="@count">
			<xsl:text></xsl:text><xsl:call-template name="makeBranchType"/> _<xsl:call-template name="makeBranchName"/>[<xsl:value-of select="@count"/>];
		</xsl:if>
		<xsl:if test="not(@count)">
			<xsl:text></xsl:text><xsl:call-template name="makeBranchType"/>	_<xsl:call-template name="makeBranchName"/>;
		</xsl:if>
	</xsl:template>

	<!-- ################################################################## -->
	<!-- ##### generate leaf C++ group name (mainly for anonimous group	 ## -->
	<!-- ################################################################## -->
	<xsl:template name="makeBranchName">
		<xsl:if test="@name = ''"><xsl:text>Array</xsl:text></xsl:if>
		<xsl:if test="@name != ''"><xsl:value-of select="@name"/></xsl:if>
	</xsl:template>

	<xsl:template name="makeBranchType">
		<xsl:if test="@name = ''"><xsl:text>TArray</xsl:text></xsl:if>
		<xsl:if test="@name != ''">T<xsl:value-of select="@name"/></xsl:if>
	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         generate leaf C++ convertion			###### -->
	<!-- ######################################################### -->

	<xsl:template name="convToUint64">
	<xsl:choose>
		<xsl:when test="@type = 'TEXT'">	<xsl:text>std::string</xsl:text></xsl:when>

	</xsl:choose>

	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         generate C++ param type				###### -->
	<!-- ######################################################### -->
	<xsl:template name="makeLeafParamType">
		<xsl:param name="leafType"/>
		<xsl:param name="cppType"/>

		<xsl:if test="$cppType = 'TEXT'"><xsl:text>const </xsl:text></xsl:if>
		<xsl:call-template name="makeLeafType"><xsl:with-param name="leafType" select="$leafType"/><xsl:with-param name="cppType" select="$cppType"/></xsl:call-template>
		<xsl:if test="$cppType = 'TEXT'"><xsl:text> &amp;</xsl:text></xsl:if>
	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         generate leaf C++ type				###### -->
	<!-- ######################################################### -->
	<xsl:template name="makeLeafType">
		<xsl:param name="leafType"/>
		<xsl:param name="cppType"/>

		<xsl:if test="$cppType"><xsl:value-of select="$cppType"/></xsl:if>
		<xsl:if test="not($cppType)">
			<xsl:choose>
				<xsl:when test="$leafType = 'I1'">		<xsl:text>bool</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I2'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I3'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I4'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I5'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I6'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I7'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I8'">		<xsl:text>uint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I9'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I10'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I11'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I12'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I13'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I14'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I15'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I16'">		<xsl:text>uint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I17'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I18'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I19'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I20'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I21'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I22'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I23'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I24'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I25'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I26'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I27'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I28'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I29'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I30'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I31'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I32'">		<xsl:text>uint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I33'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I34'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I35'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I36'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I37'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I38'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I39'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I40'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I41'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I42'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I43'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I44'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I45'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I46'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I47'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I48'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I49'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I50'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I51'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I52'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I53'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I54'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I55'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I56'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I57'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I58'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I59'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I59'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I60'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I61'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I62'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I63'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'I64'">		<xsl:text>uint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S1'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S2'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S3'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S4'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S5'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S6'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S7'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S8'">		<xsl:text>sint8</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S9'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S10'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S11'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S12'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S13'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S14'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S15'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S16'">		<xsl:text>sint16</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S17'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S18'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S19'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S20'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S21'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S22'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S23'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S24'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S25'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S26'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S27'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S28'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S29'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S30'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S31'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S32'">		<xsl:text>sint32</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S33'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S34'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S35'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S36'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S37'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S38'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S39'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S40'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S41'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S42'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S43'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S44'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S45'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S46'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S47'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S48'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S49'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S50'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S51'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S52'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S53'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S54'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S55'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S56'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S57'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S58'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S59'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S59'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S60'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S61'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S62'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S63'">		<xsl:text>sint64</xsl:text>	</xsl:when>
				<xsl:when test="$leafType = 'S64'">		<xsl:text>sint64</xsl:text>	</xsl:when>

				<xsl:when test="$leafType = 'TEXT'">	<xsl:text>ucstring</xsl:text>		</xsl:when>
				<xsl:otherwise>	<xsl:message terminate="yes">Unsupported leaf type <xsl:value-of select="$leafType"/></xsl:message></xsl:otherwise>
			</xsl:choose>
		</xsl:if>
	</xsl:template>
</xsl:stylesheet>
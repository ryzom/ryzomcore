<?xml version="1.0"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

	<xsl:output method="text" indent="no"/>

	<!-- Output : can be 'header', 'cpp' or 'php' -->
	<xsl:param name="output" select="'header'"/>
	<xsl:param name="filename"/>

	<!-- A special template applyer that is mode aware -->
	<xsl:template name="myApplyTemplate"><xsl:choose><xsl:when test="$output = 'header'"><xsl:apply-templates mode="header"/></xsl:when><xsl:when test="$output = 'cpp'"><xsl:apply-templates  mode="cpp"/></xsl:when><xsl:when test="$output = 'php'"><xsl:apply-templates mode="php"/></xsl:when></xsl:choose></xsl:template>

	<!-- some stupide template to remove unwanted text from output -->
	<xsl:template match="text()" mode="php"/>
	<xsl:template match="text()" mode="cpp"/>
	<xsl:template match="text()" mode="header"/>

	<!-- ######################################################### -->
	<!-- #####         Root template matcher               ####### -->
	<!-- ######################################################### -->
	<xsl:template match="generator">
<xsl:if test="$output != 'php'">
/////////////////////////////////////////////////////////////////
// WARNING : this is a generated file, don't change it !
/////////////////////////////////////////////////////////////////
</xsl:if>
<xsl:if test="$output = 'header'">
#ifndef <xsl:value-of select="@header_tag"/>
#define <xsl:value-of select="@header_tag"/>
#include "nel/misc/types_nl.h"
#ifdef NL_COMP_VC8
  #include &lt;memory&gt;
#endif
#include "nel/misc/hierarchical_timer.h"
#include "nel/misc/string_conversion.h"
#include "nel/net/message.h"
#include "nel/net/module.h"
#include "nel/net/module_builder_parts.h"
#include "nel/net/module_message.h"
#include "nel/net/module_gateway.h"
<xsl:if test="//callback_interface">
<!--#include "nel/net/callback_server.h"
#include "nel/net/callback_client.h"-->
#include "game_share/callback_adaptor.h"
</xsl:if>
<xsl:if test="//class/database">
#include "nel/misc/string_common.h"
#include "game_share/mysql_wrapper.h"
</xsl:if>
<xsl:if test="//class/message">
#include "game_share/synchronised_message.h"
</xsl:if>
</xsl:if>
<!--<xsl:if test="$output = 'cpp'">
<xsl:apply-templates select="cpp-include"/>
#include "<xsl:value-of select="$filename"/>.h"
</xsl:if>-->
<xsl:call-template name="myApplyTemplate"/>
<xsl:if test="$output = 'header'">
#endif
</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Namespace specification             ####### -->
	<!-- ######################################################### -->
	<xsl:template match="namespace" mode="header">
<!--#ifndef NLNET_INTERFACE_GET_MODULE
# define NLNET_INTERFACE_GET_MODULE	NLNET::IModule *getModuleInstance() { return this; }
#endif
-->
namespace <xsl:value-of select="@name"/>
{
	<!-- forward declaration of class -->
<xsl:for-each select="class">
	class <xsl:value-of select="@name"/>;
<xsl:if test="database">
	class <xsl:value-of select="@name"/>Ptr;</xsl:if></xsl:for-each>
<xsl:text>
</xsl:text>
	<!-- declaration of smart ptr class -->
<xsl:for-each select="class">
<xsl:if test="database">
	<xsl:call-template name="makePersistentPtrHeader">
		<xsl:with-param name="className" select="@name"/>
	</xsl:call-template>
<xsl:text>
</xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:call-template name="myApplyTemplate"/>
}
	</xsl:template>

	<!-- _______________________________________ -->
	<xsl:template match="namespace" mode="cpp">
#include "<xsl:value-of select="$filename"/>.h"

namespace <xsl:value-of select="@name"/>
{
<xsl:call-template name="myApplyTemplate"/>
}
</xsl:template>

	<!-- _______________________________________ -->
	<xsl:template match="namespace" mode="php"><xsl:apply-templates mode="php"/></xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Additionnal includes                ####### -->
	<!-- ######################################################### -->
	<xsl:template match="include" mode="header">
#include "<xsl:value-of select="@file"/>"
	</xsl:template>
	<xsl:template match="sys-include" mode="header">
#include &lt;<xsl:value-of select="@file"/>&gt;
	</xsl:template>

	<xsl:template match="cpp-include" mode="cpp">
#include "<xsl:value-of select="@file"/>"
	</xsl:template>

	<xsl:template match="php-include" mode="php">
<xsl:text>&lt;?php
	require_once('</xsl:text><xsl:value-of select="@file"/><xsl:text>');
?&gt;
</xsl:text>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Verbatim code                       ####### -->
	<!-- ######################################################### -->
	<xsl:template match="verbatim_header" mode="header">
<xsl:value-of select="."/>
	</xsl:template>

	<!-- ################################################################## -->
	<!-- #####        Generate a module interface classes (Header Part)#### -->
	<!-- ################################################################## -->
	<xsl:template match="module_interface" mode="header">
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class <xsl:value-of select="@name"/>Skel
	{
	public:
		/// the interceptor type
		typedef NLNET::CInterceptorForwarder &lt; <xsl:value-of select="@name"/>Skel&gt;	TInterceptor;
	protected:
		<xsl:value-of select="@name"/>Skel()
		{
			// do early run time check for message table
			getMessageHandlers();
		}
		virtual ~<xsl:value-of select="@name"/>Skel()
		{
		}

		void init(NLNET::IModule *module)
		{
			_Interceptor.init(this, module);
		}
<!-- If the interface contains two-way invocation methods, we need a virtual to retreive the module instance -->
<xsl:if test="method/return">
	public:

<!--		// Virtual to retreive the instance of the implementation module
		// Use the macro NLNET_INTERFACE_GET_MODULE to simply declare the implementation in your module
		virtual NLNET::IModule *getModuleInstance() =0;
-->
</xsl:if>
		// unused interceptors
		std::string			fwdBuildModuleManifest() const	{ return std::string(); }
		void				fwdOnModuleUp(NLNET::IModuleProxy *moduleProxy)  {}
		void				fwdOnModuleDown(NLNET::IModuleProxy *moduleProxy) {}
		void				fwdOnModuleSecurityChange(NLNET::IModuleProxy *moduleProxy) {}

		// process module message interceptor
		bool fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;message);
	private:

		typedef void (<xsl:value-of select="@name"/>Skel::*TMessageHandler)(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;message);
		typedef std::map&lt;std::string, TMessageHandler&gt;	TMessageHandlerMap;

		const TMessageHandlerMap &amp;getMessageHandlers() const;

		<xsl:for-each select="method">
<xsl:choose>
<xsl:when test="not(return)">
		void <xsl:value-of select="@name"/>_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;__message);
</xsl:when>
<xsl:when test="return">
		void <xsl:value-of select="@name"/>_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;__message);
</xsl:when>
</xsl:choose>
		</xsl:for-each>
<xsl:text>
		</xsl:text>// declare one interceptor member of the skeleton
		TInterceptor	_Interceptor;

		// declare the interceptor forwarder as friend of this class
		friend 		class NLNET::CInterceptorForwarder &lt; <xsl:value-of select="@name"/>Skel&gt;;
<xsl:text>	public:
		/////////////////////////////////////////////////////////////////
		// WARNING : this is a generated file, don't change it !
		/////////////////////////////////////////////////////////////////

</xsl:text>
		<xsl:for-each select="method">
<xsl:choose>
<xsl:when test="not(return)">
		<xsl:call-template name="makeMethodDoc"/>
<xsl:text>		</xsl:text>virtual void <xsl:value-of select="@name"/>(NLNET::IModuleProxy *sender<xsl:call-template name="makeParamList"/>)<xsl:text> =0;
</xsl:text>
</xsl:when>
<xsl:when test="return">
		<xsl:call-template name="makeMethodDoc"/>
<xsl:text>		</xsl:text>virtual <xsl:value-of select="return/@type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>(NLNET::IModuleProxy *sender<xsl:call-template name="makeParamList"/>)<xsl:text> =0;
</xsl:text>
</xsl:when>
</xsl:choose>
		</xsl:for-each>

	};

	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
<xsl:text>	</xsl:text>class <xsl:value-of select="@name"/>Proxy
	{
		/// Smart pointer on the module proxy
		NLNET::TModuleProxyPtr	_ModuleProxy;

		// Pointer on the local module that implement the interface (if the proxy is for a local module)
		NLNET::TModulePtr					_LocalModule;
		// Direct pointer on the server implementation interface for collocated module
		<xsl:value-of select="@name"/>Skel	*_LocalModuleSkel;


	public:
		<xsl:value-of select="@name"/>Proxy(NLNET::IModuleProxy *proxy)
		{
<xsl:if test="@module_class">			nlassert(proxy->getModuleClassName() == <xsl:value-of select="@module_class"/>);</xsl:if>
			_ModuleProxy = proxy;

			// initialize collocated servant interface
			if (proxy->getModuleDistance() == 0)
			{
				_LocalModule = proxy->getLocalModule();
				nlassert(_LocalModule != NULL);
				<xsl:value-of select="@name"/>Skel::TInterceptor *interceptor = NULL;
				interceptor = static_cast &lt; NLNET::CModuleBase* &gt;(_LocalModule.getPtr())->getInterceptor(interceptor);
				nlassert(interceptor != NULL);

				_LocalModuleSkel = interceptor->getParent();
				nlassert(_LocalModuleSkel != NULL);
			}
			else
				_LocalModuleSkel = 0;

		}
		virtual ~<xsl:value-of select="@name"/>Proxy()
		{
		}

		NLNET::IModuleProxy *getModuleProxy()
		{
			return _ModuleProxy;
		}

<xsl:for-each select="method">
<xsl:call-template name="makeMethodDoc"/>
<xsl:choose>
<xsl:when test="not(return)">
<xsl:text>		</xsl:text>void <xsl:value-of select="@name"/>(NLNET::IModule *sender<xsl:call-template name="makeParamList"/>);
</xsl:when>
<xsl:when test="return">
<xsl:text>		</xsl:text><xsl:value-of select="return/@type"/><xsl:text> </xsl:text><xsl:value-of select="@name"/>(NLNET::IModule *sender<xsl:call-template name="makeParamList"/>);
</xsl:when>
</xsl:choose>
	</xsl:for-each>

		<xsl:for-each select="method[@broadcast = 'true']">
<xsl:call-template name="makeMethodDoc"/>
<xsl:text>
		// This is the broadcast version of the method.
		template &lt; class ProxyIterator &gt;
		</xsl:text>static void broadcast_<xsl:value-of select="@name"/>(ProxyIterator first, ProxyIterator last, NLNET::IModule *sender<xsl:call-template name="makeParamList"/>)
		{
			NLNET::CMessage message;

			// create the message to send to multiple dest
			buildMessageFor_<xsl:value-of select="@name"/>(message <xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);

			for (; first != last; ++first)
			{
				NLNET::IModuleProxy *proxy = *first;

				proxy->sendModuleMessage(sender, message);
			}

		}<xsl:text>
</xsl:text>
	</xsl:for-each>


		<xsl:for-each select="method">
<xsl:text>
		// Message serializer. Return the message received in reference for easier integration
		static const NLNET::CMessage &amp;buildMessageFor_</xsl:text><xsl:value-of select="@name"/>(NLNET::CMessage &amp;__message<xsl:call-template name="makeParamList"/>);
	</xsl:for-each>



	};<xsl:text>
</xsl:text>

	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Generate module interface, cpp part ####### -->
	<!-- ######################################################### -->
	<xsl:template match="module_interface" mode="cpp">
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	<xsl:variable name="skelName" select="concat(@name, 'Skel')"/>
	<xsl:variable name="proxyName" select="concat(@name, 'Proxy')"/>

	const <xsl:value-of select="$skelName"/>::TMessageHandlerMap &amp;<xsl:value-of select="$skelName"/>::getMessageHandlers() const
	{
		static TMessageHandlerMap handlers;
		static bool init = false;

		if (!init)
		{
			std::pair &lt; TMessageHandlerMap::iterator, bool &gt; res;
			<xsl:for-each select="method">
			res = handlers.insert(std::make_pair(std::string("<xsl:value-of select="@msg"/>"), &amp;<xsl:value-of select="$skelName"/>::<xsl:value-of select="@name"/>_skel));
			// if this assert, you have a doubly message name in your interface definition !
			nlassert(res.second);
			</xsl:for-each>
			init = true;
		}

		return handlers;
	}
	bool <xsl:value-of select="@name"/>Skel::fwdOnProcessModuleMessage(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;message)
	{
		const TMessageHandlerMap &amp;mh = getMessageHandlers();

		TMessageHandlerMap::const_iterator it(mh.find(message.getName()));

		if (it == mh.end())
		{
			return false;
		}

		TMessageHandler cmd = it->second;
		(this->*cmd)(sender, message);

		return true;
	}

	<xsl:for-each select="method">
<xsl:choose>
<xsl:when test="not(return)">
	void <xsl:value-of select="$skelName"/>::<xsl:value-of select="@name"/>_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;__message)
	{
		H_AUTO(<xsl:value-of select="$skelName"/>_<xsl:value-of select="@name"/>_<xsl:value-of select="@msg"/>);
<xsl:for-each select="param">
<xsl:text>		</xsl:text><xsl:value-of select="@type"/><xsl:text>	</xsl:text><xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
<xsl:call-template name="serialRead"><xsl:with-param name="message" select="'__message'"/></xsl:call-template>
</xsl:for-each>

<xsl:text>		</xsl:text><xsl:value-of select="@name"/>(sender<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);<xsl:text>
</xsl:text>
<xsl:text>	}
</xsl:text>
</xsl:when>
<xsl:when test="return">
	void <xsl:value-of select="$skelName"/>::<xsl:value-of select="@name"/>_skel(NLNET::IModuleProxy *sender, const NLNET::CMessage &amp;__message)
	{
		H_AUTO(<xsl:value-of select="$skelName"/>_<xsl:value-of select="@name"/>_<xsl:value-of select="@msg"/>);
<xsl:for-each select="param">
<xsl:text>		</xsl:text><xsl:value-of select="@type"/><xsl:text>	</xsl:text><xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
<xsl:call-template name="serialRead"><xsl:with-param name="message" select="'__message'"/></xsl:call-template>
</xsl:for-each>

<xsl:text>		</xsl:text><xsl:value-of select="return/@type"/> __ret = <xsl:value-of select="@name"/>(sender<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);
		// encode the return message
		NLNET::CMessage __retMsg;
		__retMsg.setType("R_<xsl:value-of select="@msg"/>", NLNET::CMessage::Response);
		nlWrite(__retMsg, serial<xsl:value-of select="return/@serial"/>, __ret);

		// and send back the response
		sender->sendModuleMessage(static_cast&lt;NLNET::IModule*&gt;(_Interceptor.getRegistrar()), __retMsg);
<xsl:text>
</xsl:text>
<xsl:text>	}
</xsl:text>
</xsl:when>
</xsl:choose>
</xsl:for-each>

<xsl:for-each select="method">
<xsl:call-template name="makeMethodDoc"/>
<xsl:choose>
<xsl:when test="not(return)">
<xsl:text>	</xsl:text>void <xsl:value-of select="$proxyName"/>::<xsl:value-of select="@name"/>(NLNET::IModule *sender<xsl:call-template name="makeParamList"/>)
	{
		if (_LocalModuleSkel &amp;&amp; _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			_LocalModuleSkel-><xsl:value-of select="@name"/>(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender)<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);
		}
		else
		{
			// send the message for remote dispatching and execution or local queing
			NLNET::CMessage __message;

			buildMessageFor_<xsl:value-of select="@name"/>(__message<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);

			_ModuleProxy->sendModuleMessage(sender, __message);
		}
	}<xsl:text>
</xsl:text>
</xsl:when>
<xsl:when test="return">
<xsl:text>	</xsl:text><xsl:value-of select="return/@type"/><xsl:text> </xsl:text><xsl:value-of select="$proxyName"/>::<xsl:value-of select="@name"/>(NLNET::IModule *sender<xsl:call-template name="makeParamList"/>)
	{
		if (_LocalModuleSkel &amp;&amp; _LocalModule->isImmediateDispatchingSupported())
		{
			// immediate local synchronous dispatching
			return _LocalModuleSkel-><xsl:value-of select="@name"/>(_ModuleProxy->getModuleGateway()->getPluggedModuleProxy(sender)<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);
		}
		else
		{
			// send the message for remote dispatching and execution

			NLNET::CMessage __message;

			buildMessageFor_<xsl:value-of select="@name"/>(__message<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);

			NLNET::CMessage __retMsg;
			sender->invokeModuleOperation(_ModuleProxy, __message, __retMsg);

			// check the return message type
			if (__retMsg.getName() != "R_<xsl:value-of select="@msg"/>")
				throw NLNET::IModule::EInvokeBadReturn();

			<xsl:value-of select="return/@type"/> __ret;
			nlRead(__retMsg, serial, __ret);

			return __ret;
		}
	}<xsl:text>
</xsl:text>
</xsl:when>
</xsl:choose>
	</xsl:for-each>

	<xsl:for-each select="method">
<xsl:text>
	// Message serializer. Return the message received in reference for easier integration
	const NLNET::CMessage &amp;</xsl:text><xsl:value-of select="$proxyName"/>::buildMessageFor_<xsl:value-of select="@name"/>(NLNET::CMessage &amp;__message<xsl:call-template name="makeParamList"/>)
	{
		__message.setType("<xsl:value-of select="@msg"/>"<xsl:if test="return">, NLNET::CMessage::Request</xsl:if>);
<xsl:for-each select="param"><xsl:call-template name="serialWrite"> <xsl:with-param name="message" select="'__message'"/></xsl:call-template>
</xsl:for-each>

		return __message;
	}<xsl:text>
</xsl:text>
</xsl:for-each>


	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Generate documentation              ####### -->
	<!-- ######################################################### -->
	<xsl:template name="makeMethodDoc">
		<xsl:for-each select="doc">
		<xsl:text>		// </xsl:text><xsl:value-of select="@line"/><xsl:text>
</xsl:text>
		</xsl:for-each>
	</xsl:template>

	<xsl:template name="makeMasterDoc">
		<xsl:for-each select="doc">
		<xsl:text>	// </xsl:text><xsl:value-of select="@line"/><xsl:text>
</xsl:text>
		</xsl:for-each>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Generate parameter list             ####### -->
	<!-- #########################################################-->
	<xsl:template name="makeParamList">
		<xsl:if test="param">
			<xsl:text>, </xsl:text><xsl:call-template name="makeParamListNoStartingComma"/>
		</xsl:if>
	</xsl:template>

	<xsl:template name="makeParamListNoStartingComma">
		<xsl:if test="param">
			<xsl:for-each select="param">
				<xsl:choose>
					<xsl:when test="@array = 'true'">
		<!-- generate vector for callback interface-->
	<xsl:text>const std::vector&lt;</xsl:text><xsl:value-of select="@type"/>&gt; &amp;<xsl:value-of select="@name"/><xsl:if test="position() != last()">, </xsl:if>
					</xsl:when>
					<xsl:otherwise>
	<xsl:if test="@byref = 'true' or @enum='smart'">const </xsl:if><xsl:value-of select="@type"/><xsl:text> </xsl:text><xsl:if test="@byref = 'true' or @enum = 'smart'">&amp;</xsl:if><xsl:value-of select="@name"/><xsl:if test="position() != last()">, </xsl:if>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>
		</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Write serialisation                 ####### -->
	<!-- ######################################################### -->
	<xsl:template name="serialWrite">
		<xsl:param name="message" select="'message'"/>
		<xsl:choose>
			<xsl:when test="@byref = 'true'">
				<xsl:text>			</xsl:text>nlWrite(<xsl:value-of select="$message"/>, serial<xsl:value-of select="@serial"/>, const_cast &lt; <xsl:value-of select="@type"/>&amp; &gt; (<xsl:value-of select="@name"/>));<xsl:text>
</xsl:text>
			</xsl:when>
			<xsl:otherwise>
				<xsl:if test="not(@array)">
					<xsl:text>			</xsl:text>nlWrite(<xsl:value-of select="$message"/>, serial<xsl:value-of select="@serial"/>, <xsl:value-of select="@name"/>);<xsl:text>
</xsl:text>
				</xsl:if>

				<xsl:if test="@array = 'true'">
					<xsl:text>			</xsl:text>nlWrite(<xsl:value-of select="$message"/>, serialCont, <xsl:value-of select="@name"/>);<xsl:text>
</xsl:text>
				</xsl:if>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Read serialisation                  ####### -->
	<!-- ######################################################### -->
	<xsl:template name="serialRead">
		<xsl:param name="message" select="'message'"/>
<xsl:if test="not(@array)">
<xsl:text>			</xsl:text>nlRead(<xsl:value-of select="$message"/>, serial<xsl:value-of select="@serial"/>, <xsl:value-of select="@name"/>);<xsl:text>
</xsl:text>
</xsl:if>
<xsl:if test="@array = 'true'">
<xsl:text>			</xsl:text>nlRead(<xsl:value-of select="$message"/>, serialCont, <xsl:value-of select="@name"/>);<xsl:text>
</xsl:text>
</xsl:if>
	</xsl:template>

	<!-- ######################################################### -->
	<!-- #####         Enum generation                     ####### -->
	<!-- ######################################################### -->


<xsl:template match="enum" mode="php">
<xsl:text>&lt;?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	$arrayCounter = 0;
</xsl:text>
<xsl:for-each select="item">
<xsl:text>	$</xsl:text><xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="../@name"/>_EnumValues[$arrayCounter++] = "<xsl:value-of select="@name"/><xsl:text>";
</xsl:text></xsl:for-each>
<xsl:text>	$</xsl:text><xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues[$arrayCounter] = "invalid";
	$<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue = $arrayCounter;

	class <xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>
	{
		var $Value;

		function <xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>()
		{
			global $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue;
			$this->Value = $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue;
		}

		function toString()
		{
			global $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues;
			return $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues[$this->Value];
		}

		function fromString($strValue)
		{
			global $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues;
			foreach ($<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues as $k => $v)
			{
				if ($strValue === $v)
				{
					$this->Value = $k;
					return;
				}
			}

			$this->Value = $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue;
		}

		function toInt()
		{
			return $this->Value;
		}

		function fromInt($intValue)
		{
			global $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue;
			global $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues;
			if (array_key_exists($intValue, $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_EnumValues))
				$this->Value = $intValue;
			else
				$this->Value = $<xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@name"/>_InvalidValue;
		}
	}
<xsl:text>?&gt;
</xsl:text></xsl:template>


	<xsl:template match="enum" mode="header">
<xsl:call-template name="makeMasterDoc"/>
<xsl:if test="@bitset='true'">
	<xsl:call-template name="enumGen">
		<xsl:with-param name="enumName" select="concat(@name, 'Enum')"/>
	</xsl:call-template>
	typedef NLMISC::CEnumBitset &lt; <xsl:value-of select="@name"/>Enum, uint32, <xsl:value-of select="@name"/>Enum::invalid_val, ',', NLMISC::TContainedEnum &lt; <xsl:value-of select="@name"/>Enum, uint32 &gt;, <xsl:value-of select="@name"/>Enum::TValues &gt; <xsl:value-of select="@name"/>;
</xsl:if>
<xsl:if test="not(@bitset='true')">
	<xsl:call-template name="enumGen">
		<xsl:with-param name="enumName" select="@name"/>
	</xsl:call-template>
</xsl:if>
	</xsl:template>


<xsl:template name="enumGen">
	<xsl:param name="enumName"/>

	<!-- check that we don't set a base value AND a value for the 1st enum item -->
	<xsl:if test="@base and item[1]/@value">
			<xsl:message terminate="yes">
				ERROR : You can't set a base value AND a value for the first item on enum definition <xsl:value-of select="$enumName"/>
			</xsl:message>
	</xsl:if>

	struct <xsl:value-of select="$enumName"/>
	{
		enum TValues
		{
			<xsl:for-each select="item">
			<xsl:value-of select="@name"/><xsl:if test="@value"> = <xsl:value-of select="@value"/></xsl:if>
			<xsl:if test="position() = 1 and ../@base"> = <xsl:value-of select="../@base"/></xsl:if>,
			</xsl:for-each>
			<!-- generate a value 'past the end' and 'last_enum_item' if no values are set other than on first item-->
			<xsl:if test="count(item/@value) = 0 or (count(item/@value) = 1 and item[1]/@value)">
<xsl:text>/// the highest valid value in the enum
			last_enum_item = </xsl:text><xsl:value-of select="item[last()]/@name"/>,
			/// a value equal to the last enum item +1
			end_of_enum,
</xsl:if>
			<!-- generate an invalid and undefined value-->
			invalid_val,
			<!-- generate a count of node -->
			/// Number of enumerated values
			nb_enum_items = <xsl:value-of select="count(item)"/>
		};
		<!-- generate an index table if the enum is 'linear' -->
		<xsl:if test="count(item/@value) = 0 or (count(item/@value) = 1 and item[1]/@value)">
		/// Index table to convert enum value to linear index table
		const std::map&lt;TValues, uint32&gt; &amp;getIndexTable() const
		{
			static std::map&lt;TValues, uint32&gt; indexTable;
			static bool init = false;
			if (!init)
			{
				// fill the index table
			<xsl:for-each select="item">
<xsl:text>	indexTable.insert(std::make_pair(</xsl:text><xsl:value-of select="@name"/>, <xsl:value-of select="position()-1"/>));
			</xsl:for-each>
				init = true;
			}

			return indexTable;
		}
		</xsl:if>

		static const NLMISC::CStringConversion&lt;TValues&gt; &amp;getConversionTable()
		{
			NL_BEGIN_STRING_CONVERSION_TABLE(TValues)
<xsl:for-each select="item">				NL_STRING_CONVERSION_TABLE_ENTRY(<xsl:value-of select="@name"/>)
</xsl:for-each>				NL_STRING_CONVERSION_TABLE_ENTRY(invalid_val)
			};
			static NLMISC::CStringConversion&lt;TValues&gt;
			conversionTable(TValues_nl_string_conversion_table, sizeof(TValues_nl_string_conversion_table)
			/ sizeof(TValues_nl_string_conversion_table[0]),  invalid_val);

			return conversionTable;
		}

		TValues	_Value;

	public:
		<xsl:value-of select="$enumName"/>()
			: _Value(invalid_val)
		{
		}
		<xsl:value-of select="$enumName"/>(TValues value)
			: _Value(value)
		{
		}

		<xsl:value-of select="$enumName"/>(const std::string &amp;str)
		{
			_Value = getConversionTable().fromString(str);
		}

		void serial(NLMISC::IStream &amp;s)
		{
			s.serialEnum(_Value);
		}

		bool operator == (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return _Value == other._Value;
		}
		bool operator != (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return ! (_Value == other._Value);
		}
		bool operator &lt; (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return _Value &lt; other._Value;
		}

		bool operator &lt;= (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return _Value &lt;= other._Value;
		}

		bool operator &gt; (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return !(_Value &lt;= other._Value);
		}
		bool operator &gt;= (const <xsl:value-of select="$enumName"/> &amp;other) const
		{
			return !(_Value &lt; other._Value);
		}

		const std::string &amp;toString() const
		{
			return getConversionTable().toString(_Value);
		}
		static const std::string &amp;toString(TValues value)
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

		<!-- generate an index table if the enum is 'linear' -->
		<xsl:if test="count(item/@value) = 0 or (count(item/@value) = 1 and item[1]/@value)">
		uint32 asIndex()
		{
			std::map&lt;TValues, uint32&gt;::const_iterator it(getIndexTable().find(_Value));
			nlassert(it != getIndexTable().end());
			return it->second;
		}
		</xsl:if>

		<!-- insert user code if any -->
		<xsl:if test="header_code">
<xsl:value-of select="header_code"/>
		</xsl:if>
	};
	</xsl:template>



	<!-- ######################################################### -->
	<!-- #####         Class generation (db mapping, serial and message)### -->
	<!-- ######################################################### -->

	<xsl:template name="makeProperty">
		<xsl:param name="property"/>
		<xsl:text>		</xsl:text>// <xsl:value-of select="$property/@doc"/><xsl:text>
</xsl:text>
		<xsl:text>		</xsl:text><xsl:value-of select="$property/@type"/><xsl:text>	_</xsl:text><xsl:value-of select="$property/@name"/><xsl:text>;
</xsl:text>
	</xsl:template>

	<!--/////////////////////////////////////////////////////////-->
	<xsl:template name="makePropertyAccessor">
		<xsl:param name="property"/>
		<xsl:text>		</xsl:text>// <xsl:value-of select="$property/@doc"/><xsl:text>
</xsl:text>
<xsl:choose>
	<xsl:when test="$property/@byref = 'true'">
		<xsl:text>		</xsl:text>const <xsl:value-of select="$property/@type"/> &amp;get<xsl:value-of select="$property/@name"/>() const
		{
			return _<xsl:value-of select="$property/@name"/>;
		}

<xsl:if test="not(../database) and $property/@byref = 'true'">
	<!-- for non database 'by reference' property, generate a non const get accessor -->
		<xsl:text>		</xsl:text><xsl:value-of select="$property/@type"/> &amp;get<xsl:value-of select="$property/@name"/>()
		{
			return _<xsl:value-of select="$property/@name"/>;
		}
</xsl:if>

		void set<xsl:value-of select="$property/@name"/>(const <xsl:value-of select="$property/@type"/> &amp;value)
		{
<xsl:if test="../database">
			if (_<xsl:value-of select="$property/@name"/> != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);
</xsl:if>

				<xsl:variable name="relation" select="parent[@db_col = $property/@db_col]"/>
				<xsl:if test="$relation">
					<!-- this property is a child/parent relation, update the parent -->

				<xsl:value-of select="$relation/@class"/>Ptr parent = <xsl:value-of select="$relation/@class"/>::loadFromCache(_<xsl:value-of select="$property/@name"/>);

				if (parent &amp;&amp; getPersistentState() != NOPE::os_transient )
					parent->remove<xsl:value-of select="$relation/@child_name"/>Child(this);
				</xsl:if>

				_<xsl:value-of select="$property/@name"/> = value;

				<xsl:if test="$relation">
					<!-- this property is a child/parent relation, update the parent -->

				<xsl:value-of select="$relation/@class"/>Ptr parent = <xsl:value-of select="$relation/@class"/>::loadFromCache(_<xsl:value-of select="$property/@name"/>);
				if (parent &amp;&amp; getPersistentState() != NOPE::os_transient)
					parent->insert<xsl:value-of select="$relation/@child_name"/>Child(this);
				</xsl:if>
<xsl:if test="../database">
			}
</xsl:if>
		}
	</xsl:when>
	<xsl:otherwise>
		<xsl:text>		</xsl:text><xsl:value-of select="$property/@type"/> get<xsl:value-of select="$property/@name"/>() const
		{
			return _<xsl:value-of select="$property/@name"/>;
		}

		void set<xsl:value-of select="$property/@name"/>(<xsl:value-of select="$property/@type"/> value)
		{
<xsl:if test="../database">
			if (_<xsl:value-of select="$property/@name"/> != value)
			{
				if (getPersistentState() != NOPE::os_transient)
					setPersistentState(NOPE::os_dirty);
</xsl:if>
				_<xsl:value-of select="$property/@name"/> = value;
<xsl:if test="../database">
			}
</xsl:if>
		}
	</xsl:otherwise>
</xsl:choose>
	</xsl:template>


	<!--/////////////////////////////////////////////////////////-->
	<xsl:template name="makeChildAccessor">
		<xsl:param name="childClass"/>
<xsl:if test="@cont">
<xsl:choose>
	<xsl:when test="@cont = 'vector'">
		/** Return a const reference to the vector of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following methods who return non const pointer
		 *	on contained elements.
		 */
		const std::vector&lt;<xsl:value-of select="@type"/>Ptr&gt; &amp;get<xsl:value-of select="@name"/>() const;
		/** Return the ith element of the child vector
		 *	index must be valid (ie less than size of the vector)
		 */
		<xsl:value-of select="@type"/>Ptr &amp;get<xsl:value-of select="@name"/>ByIndex(uint32 index) const;
		/** Return the identified element by looking in the vector
		 *	If no element match the id, NULL pointer is returned
		 */
		<xsl:value-of select="@type"/>Ptr &amp;get<xsl:value-of select="@name"/>ById(uint32 id) const;

	</xsl:when>
	<xsl:when test="@cont = 'map'">
		/** Return a const reference to the map of child.
		 *	If you want to modify the element inside, you need to
		 *	use on of the two following method who return non const pointer
		 *	on contained elements.
		 */
		const std::map&lt;uint32, <xsl:value-of select="@type"/>Ptr&gt; &amp;get<xsl:value-of select="@name"/>() const;
		/** Return the identified element by looking in the map
		 *	If no element match the id, NULL pointer is returned
		 */
		<xsl:value-of select="@type"/>Ptr &amp;get<xsl:value-of select="@name"/>ById(uint32 id) const;

	</xsl:when>
	<xsl:otherwise>
			<xsl:message terminate="yes">
				ERROR : Invalide container, child must be either map or vector for child <xsl:value-of select="@name"/> in class <xsl:value-of select="../@name"/>

			</xsl:message>
	</xsl:otherwise>
</xsl:choose>
</xsl:if>

<xsl:if test="not(@cont)">
		/** Return the one child object (or null if not) */
		<xsl:value-of select="@type"/>Ptr get<xsl:value-of select="@name"/>();
</xsl:if>
	</xsl:template>


	<!--/////////////////////////////////////////////////////////-->
	<xsl:template name="makeColumList">
		<xsl:param name="uniqueId"/>
<xsl:choose>
	<xsl:when test="property[@name = $uniqueId and @db_col]/@autogen = 'true'">
		qs += "<xsl:for-each select="property[@name != $uniqueId]">
			<xsl:value-of select="@db_col"/>
			<xsl:if test="position() != last()"><xsl:text>, </xsl:text></xsl:if>
		</xsl:for-each><xsl:text>";</xsl:text>
	</xsl:when>
	<xsl:otherwise>
		qs += "<xsl:for-each select="property[@db_col]">
			<xsl:value-of select="@db_col"/>
			<xsl:if test="position() != last()">, </xsl:if>
		</xsl:for-each><xsl:text>";</xsl:text>
	</xsl:otherwise>
</xsl:choose>
	</xsl:template>

	<xsl:template name="makeColumListWithId">
		qs += "<xsl:for-each select="property[@db_col]">
			<xsl:value-of select="@db_col"/>
			<xsl:if test="position() != last()">, </xsl:if>
		</xsl:for-each>
<xsl:text>";
</xsl:text>
	</xsl:template>

	<xsl:template name="makeValueList">
		<xsl:param name="uniqueId"/>
<xsl:choose>
	<xsl:when test="property[@name = $uniqueId]/@autogen = 'true'">
		<xsl:for-each select="property[@name != $uniqueId and @db_col]">
			<xsl:choose>
				<xsl:when test="@enum='true' or @enum='smart'">
		qs += "'"+_<xsl:value-of select="@name"/><xsl:text>.toString()+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@date='true'">
		qs += "'"+MSW::encodeDate(_<xsl:value-of select="@name"/>)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@md5='true'">
		qs += "'"+MSW::escapeString(_<xsl:value-of select="@name"/>.toString(), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:otherwise>
		qs += "'"+MSW::escapeString(NLMISC::toString(_<xsl:value-of select="@name"/>), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
		<xsl:if test="position() != last()"><xsl:text>		qs += ", ";</xsl:text></xsl:if>
		</xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
		<xsl:for-each select="property[@db_col]">
			<xsl:choose>
				<xsl:when test="@enum='true' or @enum='smart'">
		qs += "'"+_<xsl:value-of select="@name"/><xsl:text>.toString()+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@date='true'">
		qs += "'"+MSW::encodeDate(_<xsl:value-of select="@name"/>)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@md5='true'">
		qs += "'"+MSW::escapeString(_<xsl:value-of select="@name"/>.toString(), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:otherwise>
		qs += "'"+MSW::escapeString(NLMISC::toString(_<xsl:value-of select="@name"/>), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="position() != last()"><xsl:text>		qs += ", ";</xsl:text></xsl:if>
		</xsl:for-each>
	</xsl:otherwise>
</xsl:choose>
	</xsl:template>

	<xsl:template name="makeValueListWithId">
		<xsl:for-each select="property[@db_col]">
			<xsl:choose>
				<xsl:when test="@enum='true' or @enum='smart'">
			qs += "'"+_<xsl:value-of select="@name"/>.toString()+"'";
				</xsl:when>
				<xsl:when test="@date='true'">
			qs += "'"+MSW::encodeDate(_<xsl:value-of select="@name"/>)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@md5='true'">
			qs += "'"+MSW::escapeString(_<xsl:value-of select="@name"/>.toString(), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:otherwise>
			qs += "'"+MSW::escapeString(NLMISC::toString(_<xsl:value-of select="@name"/>), connection)+"'";
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="position() != last()">qs += ", ";</xsl:if>
		</xsl:for-each>
	</xsl:template>

	<xsl:template name="makeSetList">
		<xsl:param name="uniqueId"/>
<xsl:choose>
	<xsl:when test="property[@name = $uniqueId and @db_col]/@autogen = 'true'">
		<xsl:for-each select="property[@name != $uniqueId and @db_col]">
			<xsl:choose>
				<xsl:when test="@enum='true' or @enum='smart'">
		qs += "<xsl:value-of select="@db_col"/> = '"+_<xsl:value-of select="@name"/><xsl:text>.toString()+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@date='true'">
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::encodeDate(_<xsl:value-of select="@name"/>)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@md5='true'">
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::escapeString(_<xsl:value-of select="@name"/>.toString(), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:otherwise>
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::escapeString(NLMISC::toString(_<xsl:value-of select="@name"/>), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="position() != last()"><xsl:text>		qs += ", ";</xsl:text></xsl:if>
		</xsl:for-each>
	</xsl:when>
	<xsl:otherwise>
		<xsl:for-each select="property[@db_col]">
			<xsl:choose>
				<xsl:when test="@enum='true' or @enum='smart'">
		qs += "<xsl:value-of select="@db_col"/> = '"+_<xsl:value-of select="@name"/><xsl:text>.toString()+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@date='true'">
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::encodeDate(_<xsl:value-of select="@name"/>)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:when test="@md5='true'">
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::escapeString(_<xsl:value-of select="@name"/>.toString(), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:when>
				<xsl:otherwise>
		qs += "<xsl:value-of select="@db_col"/> = '"+MSW::escapeString(NLMISC::toString(_<xsl:value-of select="@name"/>), connection)<xsl:text>+"'";
</xsl:text>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:if test="position() != last()"><xsl:text>		qs += ", ";</xsl:text></xsl:if>
		</xsl:for-each>
	</xsl:otherwise>
</xsl:choose>
	</xsl:template>

	<xsl:template name="makeWhereClause">
		<xsl:param name="uniqueId"/>
		qs += " WHERE <xsl:value-of select="property[@name = $uniqueId]/@db_col"/> = '"+NLMISC::toString(_<xsl:value-of select="property[@name = $uniqueId]/@name"/>)+"'";
	</xsl:template>

	<xsl:template name="makeWhereClauseWithId">
		<xsl:param name="uniqueId"/>
		<xsl:param name="id"/>
		qs += " WHERE <xsl:value-of select="property[@name = $uniqueId]/@db_col"/> = '"+NLMISC::toString(<xsl:value-of select="$id"/>)+"'";
	</xsl:template>



	<!-- ######################################################### -->
	<!-- #####         Class generation (db mapping, serial and message)### -->
	<!-- ######################################################### -->
	<!-- #####  Include file #### -->
	<xsl:template match="class" mode="header">
		<xsl:if test="count(property[@unique_id = 'true' and @db_col]) != 1 and datase">
			<xsl:message terminate="yes">
				ERROR : You must have ONE and ONLY one unique_id property in class '_<xsl:value-of select="@name"/>'
			</xsl:message>
		</xsl:if>

		<xsl:choose>
			<xsl:when test="database">
				<xsl:call-template name="makeClassHeader">
					<xsl:with-param name="className" select="@name"/>
					<xsl:with-param name="uniqueId" select="property[@unique_id = 'true']/@name"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="makeClassHeader">
					<xsl:with-param name="className" select="@name"/>
					<xsl:with-param name="uniqueId" select="''"/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="makeClassHeader">
		<xsl:param name="className"/>
		<xsl:param name="uniqueId"/>

<!--<xsl:if test="database">
<xsl:call-template name="makePersistentPtrHeader">
	<xsl:with-param name="className" select="@name"/>
</xsl:call-template>
</xsl:if>
-->
<xsl:call-template name="makeMasterDoc"/>
<xsl:text>	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////
	class </xsl:text><xsl:value-of select="$className"/>
	{
<xsl:text>	protected:
</xsl:text>		<xsl:for-each select="property">
			<xsl:call-template name="makeProperty">
				<xsl:with-param name="property" select="."/>
			</xsl:call-template>
		</xsl:for-each>
<xsl:for-each select="child_class[@relation = 'one-to-many']">
		friend class <xsl:value-of select="@type"/>;
<xsl:choose>
<xsl:when test="@cont = 'map'">
		std::map &lt; uint32,  <xsl:value-of select="@type"/>Ptr &gt;	*_<xsl:value-of select="@name"/>;
</xsl:when>
<xsl:when test="@cont = 'vector'">
		std::vector &lt; <xsl:value-of select="@type"/>Ptr &gt;	*_<xsl:value-of select="@name"/>;
</xsl:when>
<xsl:otherwise>
	<xsl:message terminate="yes">
		ERROR : You must define the cont="" property with either 'vector' or 'map' value in the child_class element for class '<xsl:value-of select="../@name"/>' , child '_<xsl:value-of select="@name"/>'
	</xsl:message>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
<xsl:for-each select="child_class[@relation = 'one-to-one']">
<xsl:text>		friend class </xsl:text><xsl:value-of select="@type"/>;
		bool								_<xsl:value-of select="@name"/>Loaded;
		<xsl:value-of select="@type"/>Ptr	_<xsl:value-of select="@name"/>;
</xsl:for-each>

<xsl:text>	public:
</xsl:text>		<xsl:for-each select="property[@name != $uniqueId]">
			<xsl:call-template name="makePropertyAccessor">
				<xsl:with-param name="property" select="."/>
			</xsl:call-template>
		</xsl:for-each>

<xsl:for-each select="child_class">
			<xsl:call-template name="makeChildAccessor">
				<xsl:with-param name="childClass" select="."/>
			</xsl:call-template>
</xsl:for-each>
		bool operator == (const <xsl:value-of select="@name"/> &amp;other) const
		{
			return <xsl:for-each select="property">_<xsl:value-of select="@name"/> == other._<xsl:value-of select="@name"/><xsl:if test="position() != last()">
				&amp;&amp; </xsl:if></xsl:for-each>;
		}

<xsl:if test="not(database)">
<xsl:text>
		// constructor
		</xsl:text><xsl:value-of select="$className"/>()
		{
<xsl:if test="property[@default]">			// Default initialisation
</xsl:if>
<xsl:for-each select="property[@default]">
<xsl:text>			</xsl:text>_<xsl:value-of select="@name"/> = <xsl:value-of select="@default"/>;
</xsl:for-each>
		}
</xsl:if>

<xsl:if test="message">
	<!-- generate a serialisable and sendable nel message -->
		void send(const std::string &amp;serviceName)
		{
			NLNET::CMessage msg("<xsl:value-of select="@name"/>");
			serial(msg);
			sendMessageViaMirror( serviceName, msg );
		}

		void send(NLNET::TServiceId serviceId)
		{
			NLNET::CMessage msg("<xsl:value-of select="@name"/>");
			serial(msg);
			sendMessageViaMirror( serviceId, msg );
		}
</xsl:if>

<xsl:if test="database">
<xsl:text>
	private:
		// private constructor, you must use 'createTransient' to get an instance
		</xsl:text><xsl:value-of select="$className"/>()
			: _PtrList(NULL),
			_ObjectState(NOPE::os_transient),
			_<xsl:value-of select="$uniqueId"/>(NOPE::INVALID_OBJECT_ID)
		{
<xsl:if test="property[@default]">			// Default initialisation
</xsl:if>
<xsl:for-each select="property[@default]">
<xsl:text>			</xsl:text>_<xsl:value-of select="@name"/> = <xsl:value-of select="@default"/>;
</xsl:for-each>
<xsl:for-each select="child_class[@relation = 'one-to-many']">
<xsl:text>			_</xsl:text><xsl:value-of select="@name"/> = NULL;
</xsl:for-each>
<xsl:for-each select="child_class[@relation = 'one-to-one']">
<xsl:text>			_</xsl:text><xsl:value-of select="@name"/>Loaded = false;
</xsl:for-each>
			// register the cache for this class (if not already done)
			registerUpdatable();
		}

		// Destructor, delete any children
		~<xsl:value-of select="@name"/>();

		/// utility func to remove this object from the released object container
		void removeFromReleased();


	public:
		/// Create a new instance in the transient space
		static <xsl:value-of select="$className"/>Ptr createTransient(const char *filename, uint32 lineNum)
		{
			return <xsl:value-of select="$className"/>Ptr(new <xsl:value-of select="$className"/>(), filename, lineNum);
		}

		/** Create a new object in the database from the current object data.
		 *	The object MUST be in transient state (i.e, it must be a new
		 *	object created by user and not comming from the databse).
		 *	If identifier is autogenerated, the generated id can be read after
		 *	this call.
		 */
		bool create(MSW::CConnection &amp;connection);
		/** Update the database with the current object state.
		 *	The object MUST be either in clean or dirty state.
		 *	Return true if the object has been effectively stored
		 *	in the database.
		 *	After this call, the object is in 'clean' state.
		 */
		bool update(MSW::CConnection &amp;connection);
		/** Remove the current object from the persistent storage.
		 *	The object must be in clean or dirty state.
		 *	Return true if the object has been correctly
		 *	updated in the database.
		 *	After the call, the object is in 'clean' state.
		 */
		bool remove(MSW::CConnection &amp;connection);
		/** Remove an object from the persistent storage by specifying
		 *	the id to remove.
		 *	Return true if an entry as been effectively removed from
		 *	the database.
		 *	After the call, the object is in 'removed' state and should
		 *	no more be used. A good pratice should be to delete
		 *	the transient object just after the remove call.
		 */
		static bool removeById(MSW::CConnection &amp;connection, uint32 id);

		/** Load an instance from the database.
		 *	This call allocate a new object and load the property value
		 *	from the database.
		 *	Return NULL if the object id is not found.
		 */
		static <xsl:value-of select="$className"/>Ptr load(MSW::CConnection &amp;connection, uint32 id, const char *filename, uint32 lineNum);

<xsl:for-each select="parent[@relation = 'one-to-many']">
		/** Load all objects children of <xsl:value-of select="@class"/> and
		 *	return them by using the specified output iterator.
		 */
<xsl:choose>
<xsl:when test="@cont = 'vector'">
		static bool loadChildrenOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, std::vector &lt; <xsl:value-of select="$className"/>Ptr &gt; &amp;children, const char *filename, uint32 lineNum);
</xsl:when>
<xsl:when test="@cont = 'map'">
		static bool loadChildrenOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, std::map &lt; uint32, <xsl:value-of select="$className"/>Ptr &gt; &amp;children, const char *filename, uint32 lineNum);
</xsl:when>
<xsl:otherwise>
	<xsl:message terminate="yes">
				ERROR : parent/child relation support only 'map' or 'vector' cont specification in <xsl:value-of select="$className"/> definition
	</xsl:message>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
<xsl:for-each select="parent[@relation = 'one-to-one']">
		/** Load the object child of <xsl:value-of select="@class"/> and
		 *	return true if no error, false in case of error (in SQL maybe).
		 *	If no such object is found, fill the child pointer with NULL.
		 */
		static bool loadChildOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, <xsl:value-of select="../@name"/>Ptr &amp;childPtr, const char *filename, uint32 lineNum);
</xsl:for-each>


<!--			<child_class	type="CKnownUser"	name="KnownUsers" relation="one-to-many" cont="vector"/>-->
<xsl:for-each select="child_class">
		/// Load <xsl:value-of select="@name"/> child(ren) object(s).
		bool load<xsl:value-of select="@name"/>(MSW::CConnection &amp;connection, const char *filename, uint32 lineNum);
</xsl:for-each>
</xsl:if>

		<!-- generate serial -->
		<xsl:if test="serial or message">
<xsl:text>
		void serial(NLMISC::IStream &amp;s)
		{
</xsl:text>
	<xsl:for-each select="property">
<xsl:text>			s.serial</xsl:text><xsl:value-of select="@serial"/>(_<xsl:value-of select="@name"/>)<xsl:text>;
</xsl:text>
	</xsl:for-each>
		}
		</xsl:if>

	private:
	<!-- generate private child container modifier -->
<xsl:for-each select="child_class[relation = 'one-to-many' and cont='vector']">

		/// add a child in the container
		void insert<xsl:value-of select="@name"/>Child(<xsl:value-of select="@type"/> &amp;child)
		{
			nlassert(std::find(_<xsl:value-of select="@name"/>.begin(), _<xsl:value-of select="@name"/>.end(), child) == _<xsl:value-of select="@name"/>.end());

			_<xsl:value-of select="@name"/>.push_back(child);
		}

		// remove a child from the container
		void removeXXChild()
		{
		}

</xsl:for-each>


<xsl:if test="database">
	private:
		friend class CPersistentCache;
		friend class <xsl:value-of select="$className"/>Ptr;

		typedef std::map&lt;uint32, <xsl:value-of select="$className"/>*&gt;	TObjectCache;
		typedef std::set&lt;<xsl:value-of select="$className"/>*&gt;			TObjectSet;
		typedef std::map&lt;time_t, TObjectSet&gt;	TReleasedObject;

		/// The complete set of object currently in memory (either active or released) excluding transient instance
		static TObjectCache		_ObjectCache;
		/// The set of object in memory ut released (no pointer on them) and waiting for decommit
		static TReleasedObject	_ReleasedObject;

		/// The current object state
		NOPE::TObjectState		_ObjectState;

		/// For object in released state, this is the release date (used to trigger deletion of the object from memory)
		time_t					_ReleaseDate;

		/// The linked list of pointer on this object
		<xsl:value-of select="$className"/>Ptr		*_PtrList;

		// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
		static <xsl:value-of select="$className"/> *loadFromCache(uint32 objectId, bool unrelease);

		// Receive and execute command from the cache manager.
		static uint32 cacheCmd(NOPE::TCacheCmd cmd);

		static void dump();

		static void updateCache();

	public:
		static void clearCache();
	private:
		void registerUpdatable();

		// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
		void setFirstPtr(<xsl:value-of select="$className"/>Ptr *ptr);

		// return the first pointer of the pointer list (can be null)
		<xsl:value-of select="$className"/>Ptr *getFirstPtr()
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
<xsl:if test="property[@name = $uniqueId]/@autogen = 'true'">
			nlassert(getPersistentState() != NOPE::os_transient);</xsl:if>
			return _<xsl:value-of select="$uniqueId"/>;
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
			nlassert(_<xsl:value-of select="$uniqueId"/> == NOPE::INVALID_OBJECT_ID);
			_<xsl:value-of select="$uniqueId"/> = objectId;
		}

		/** Return the current persistent state of the object.*/
		NOPE::TObjectState getPersistentState() const
		{
			return _ObjectState;
		}

	private:
		// Set the persistent state of the object and do some house keeping
		void setPersistentState(NOPE::TObjectState state);

</xsl:if>

	};


	</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         Class generation (db mapping, serial and message)### -->
	<!-- ######################################################### -->
	<!-- #####  cpp file #### -->
	<xsl:template match="class" mode="cpp">
		<xsl:if test="count(property[@unique_id = 'true' and @db_col]) != 1 and database">
			<xsl:message terminate="yes">
				ERROR : You must have ONE and ONLY one unique_id property in class '_<xsl:value-of select="@name"/>'
			</xsl:message>
		</xsl:if>

		<xsl:choose>
			<xsl:when test="database">
				<xsl:call-template name="makeClassCpp">
					<xsl:with-param name="className" select="@name"/>
					<xsl:with-param name="uniqueId" select="property[@unique_id = 'true']/@name"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="makeClassCpp">
					<xsl:with-param name="className" select="@name"/>
					<xsl:with-param name="uniqueId" select="''"/>
				</xsl:call-template>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template name="makeClassCpp">
		<xsl:param name="className"/>
		<xsl:param name="uniqueId"/>

<xsl:if test="database">
<xsl:call-template name="makePersistentPtrCpp">
	<xsl:with-param name="className" select="@name"/>
</xsl:call-template>
</xsl:if>


<xsl:if test="database">
<xsl:text>

	</xsl:text>	<xsl:value-of select="$className"/>::TObjectCache		<xsl:value-of select="$className"/>::_ObjectCache;
	<xsl:value-of select="$className"/>::TReleasedObject	<xsl:value-of select="$className"/>::_ReleasedObject;
<xsl:text>

	// Destructor, delete any children
	</xsl:text><xsl:value-of select="$className"/>::~<xsl:value-of select="@name"/>()
	{
		// release childs reference
<xsl:for-each select="child_class[@relation = 'one-to-many']">
<xsl:text>			if (_</xsl:text><xsl:value-of select="@name"/> != NULL)
						delete _<xsl:value-of select="@name"/>;
</xsl:for-each>

		if (_PtrList != NULL)
		{
			nlwarning("ERROR : someone try to delete this object, but there are still ptr on it !");
			<xsl:value-of select="$className"/>Ptr *ptr = _PtrList;
			do
			{
				nlwarning("  Pointer created from '%s', line %u", ptr->_FileName, ptr->_LineNum);
				ptr = _PtrList->getNextPtr();
			} while(ptr != _PtrList);
			nlstop;
		}
		// remove object from cache map
		if (_<xsl:value-of select="$uniqueId"/> != NOPE::INVALID_OBJECT_ID
			&amp;&amp; _ObjectState != NOPE::os_removed
			&amp;&amp; _ObjectState != NOPE::os_transient)
		{
			nldebug("NOPE: clearing <xsl:value-of select="$className"/> @%p from cache with id %u", this, static_cast&lt;uint32&gt;(_<xsl:value-of select="$uniqueId"/>));
			nlverify(_ObjectCache.erase(_<xsl:value-of select="$uniqueId"/>) == 1);
		}
		else if (_ObjectState != NOPE::os_transient)
		{
			nlassert(_ObjectCache.find(_<xsl:value-of select="$uniqueId"/>) == _ObjectCache.end());
		}
		if (_ObjectState == NOPE::os_released)
		{
			removeFromReleased();
		}
		else
		{
			TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
			if (it != _ReleasedObject.end())
			{
				nlassert(it->second.find(this) == it->second.end());
			}
		}
	}

	void <xsl:value-of select="$className"/>::removeFromReleased()
	{
		TReleasedObject::iterator it(_ReleasedObject.find(_ReleaseDate));
		nlassert(it != _ReleasedObject.end());
		TObjectSet &amp;os = it->second;

		nlverify(os.erase(this) == 1);

		// nb : _ReleasedObject time entry are removed by the cache update
	}

	bool <xsl:value-of select="$className"/>::create(MSW::CConnection &amp;connection)
	{
		nlassert(getPersistentState() == NOPE::os_transient);
<xsl:if test="not(property[@name = $uniqueId]/@autogen = 'true')">
		nlassert(_<xsl:value-of select="property[@name = $uniqueId]/@name"/> != 0);</xsl:if>
		std::string qs;
		qs = "INSERT INTO <xsl:value-of select="database/@table"/> (";
		<xsl:call-template name="makeColumList">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
		</xsl:call-template>
		qs += ") VALUES (";
		<xsl:call-template name="makeValueList">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
		</xsl:call-template>
		qs += ")";

		if (connection.query(qs))
		{
<xsl:if test="property[@name = $uniqueId]/@autogen = 'true'">
<xsl:text>			uint32 _id_ = connection.getLastGeneratedId();
			setObjectId(_id_);
</xsl:text></xsl:if>

			setPersistentState(NOPE::os_clean);

			// update the parent class instance in cache if any
<xsl:for-each select="parent[@relation = 'one-to-many']">
	<xsl:variable name="db_col" select="@db_col"/>
	<xsl:variable name="parentId" select="concat('_', ../property[@db_col = $db_col]/@name)"/>
			if (<xsl:value-of select="$parentId"/> != 0)
			{
				// need to update the parent class child list if it is in the cache
				<xsl:value-of select="@class"/> *parent = <xsl:value-of select="@class"/>::loadFromCache(<xsl:value-of select="$parentId"/>, false);
				if (parent &amp;&amp; parent->_<xsl:value-of select="@child_name"/> != NULL)
				{
<xsl:choose>
 <xsl:when test="@cont = 'map'">
						nlverify(parent->_<xsl:value-of select="@child_name"/>->insert(std::make_pair(getObjectId(), <xsl:value-of select="$className"/>Ptr(this, __FILE__, __LINE__))).second);
 </xsl:when>
 <xsl:when test="@cont = 'vector'">
						nlassert(std::find(parent->_<xsl:value-of select="@child_name"/>->begin(), parent->_<xsl:value-of select="@child_name"/>->end(), <xsl:value-of select="$className"/>Ptr(this, __FILE__, __LINE__)) == parent->_<xsl:value-of select="@child_name"/>->end());
						parent->_<xsl:value-of select="@child_name"/>->push_back(<xsl:value-of select="$className"/>Ptr(this, __FILE__, __LINE__));
 </xsl:when>
</xsl:choose>
				}
			}
</xsl:for-each>

<xsl:for-each select="parent[@relation = 'one-to-one']">
	<xsl:variable name="db_col" select="@db_col"/>
	<xsl:variable name="parentId" select="concat('_', ../property[@db_col = $db_col]/@name)"/>
			if (<xsl:value-of select="$parentId"/> != 0)
			{
				// need to update the parent class child if it is in the cache
				<xsl:value-of select="@class"/> *parent = <xsl:value-of select="@class"/>::loadFromCache(<xsl:value-of select="$parentId"/>, false);
				if (parent &amp;&amp; parent->_<xsl:value-of select="@child_name"/>Loaded)
				{
					nlassert(parent->_<xsl:value-of select="@child_name"/> == NULL);
					parent->_<xsl:value-of select="@child_name"/> = <xsl:value-of select="$className"/>Ptr(this, __FILE__, __LINE__);
				}
			}
</xsl:for-each>
			return true;
		}

		return false;
	}

	bool <xsl:value-of select="$className"/>::update(MSW::CConnection &amp;connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		if (getPersistentState() == NOPE::os_clean)
			// the object is clean, just ignore the save
			return true;

		std::string qs;
		qs = "UPDATE <xsl:value-of select="database/@table"/> SET ";
		<xsl:call-template name="makeSetList">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
		</xsl:call-template>
		<xsl:call-template name="makeWhereClause">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
		</xsl:call-template>

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				setPersistentState(NOPE::os_clean);
				return true;
			}
		}

		return false;
	}

	bool <xsl:value-of select="$className"/>::remove(MSW::CConnection &amp;connection)
	{
		nlassert(getPersistentState() == NOPE::os_dirty || getPersistentState() == NOPE::os_clean);

		std::string qs;
		qs = "DELETE FROM <xsl:value-of select="database/@table"/> ";
		<xsl:call-template name="makeWhereClause">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
		</xsl:call-template>

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
<xsl:for-each select="child_class[@on-delete = 'cascade' and @cont='vector']">
				{
					// cascading deletion for vector child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					const std::vector &lt; <xsl:value-of select="@type"/>Ptr &gt; &amp; childs = get<xsl:value-of select="@name"/>();

					while (!childs.empty())
					{
						get<xsl:value-of select="@name"/>ByIndex(childs.size()-1)->remove(connection);
					}
				}

</xsl:for-each>
<xsl:for-each select="child_class[@on-delete = 'cascade' and @cont='map']">
				{
					// cascading deletion for map child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					const std::map &lt; uint32, <xsl:value-of select="@type"/>Ptr &gt; &amp; childs = get<xsl:value-of select="@name"/>();

					while (!childs.empty())
					{
						get<xsl:value-of select="@name"/>ById(childs.begin()->first)->remove(connection);
					}
				}
</xsl:for-each>
<xsl:for-each select="child_class[@on-delete = 'cascade' and @relation='one-to-one']">
				{
					// cascading deletion for single child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					if (get<xsl:value-of select="@name"/>() != NULL)
						get<xsl:value-of select="@name"/>()->remove(connection);
				}
</xsl:for-each>
<xsl:for-each select="child_class[@on-delete = 'update' and @cont='vector']">
				{
					// unreference (and update) for vector child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					const std::vector &lt; <xsl:value-of select="@type"/>Ptr &gt; &amp; childs = get<xsl:value-of select="@name"/>();

					for (uint i=0; i &lt; childs.size(); ++i)
					{
						<xsl:variable name="type" select="@type"/>
						<xsl:variable name="child_name" select="@name"/>
						<xsl:variable name="parent_prop" select="//class[@name = $type]/property[@db_col = //class[@name = $type]/parent[@class = $className]/@db_col]/@name"/>
						get<xsl:value-of select="@name"/>ByIndex(i)->set<xsl:value-of select="$parent_prop"/>(0);
						get<xsl:value-of select="@name"/>ByIndex(i)->update(connection);
					}
				}
</xsl:for-each>
<xsl:for-each select="child_class[@on-delete = 'update' and @cont='map']">
				{
					// unreference (and update) for map child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					const std::map &lt; uint32, <xsl:value-of select="@type"/>Ptr &gt; &amp; childs = get<xsl:value-of select="@name"/>();
					std::map&lt; uint32, <xsl:value-of select="@type"/>Ptr &gt;::const_iterator first(childs.begin()), last(childs.end());

					for (; first != last; ++first)
					{
						<xsl:variable name="type" select="@type"/>
						<xsl:variable name="child_name" select="@name"/>
						<xsl:variable name="parent_prop" select="//class[@name = $type]/property[@db_col = //class[@name = $type]/parent[@class = $className]/@db_col]/@name"/>
						get<xsl:value-of select="@name"/>ById(it->first)->set<xsl:value-of select="$parent_prop"/>(0);
						get<xsl:value-of select="@name"/>ById(it->first)->update(connection);
					}
				}
</xsl:for-each>
<xsl:for-each select="child_class[@on-delete = 'update' and @relation='one-to-one']">
				{
					// unreference (and update) for single child <xsl:value-of select="@name"/>
					nlassert(load<xsl:value-of select="@name"/>(connection, __FILE__, __LINE__));

					<xsl:variable name="type" select="@type"/>
					<xsl:variable name="child_name" select="@name"/>
					<xsl:variable name="parent_prop" select="//class[@name = $type]/property[@db_col = //class[@name = $type]/parent[@class = $className]/@db_col]/@name"/>
					if (get<xsl:value-of select="@name"/>() != NULL)
					{
						get<xsl:value-of select="@name"/>()->set<xsl:value-of select="parent_prop"/>(0);
						get<xsl:value-of select="@name"/>()->update(connection);
					}
				}
</xsl:for-each>

				// change the persistant state to 'removed'.
				setPersistentState(NOPE::os_removed);

				// need to remove ref from parent class container (if any)
				<xsl:for-each select="parent[@relation = 'one-to-many']">
				{
					<xsl:variable name="db_col" select="@db_col"/>
					<xsl:if test="count(../property[@db_col = $db_col]) = 0">
						<xsl:message terminate="yes">
							ERROR : parent/child relation : can't find parent param with db_col '<xsl:value-of select="$db_col"/>' in <xsl:value-of select="$className"/> definition
						</xsl:message>
					</xsl:if>
					<xsl:value-of select="@class"/>Ptr parent(<xsl:value-of select="@class"/>::loadFromCache(_<xsl:value-of select="../property[@db_col = $db_col]/@name"/>, true), __FILE__, __LINE__);
					if (parent != NULL &amp;&amp; parent->_<xsl:value-of select="@child_name"/> != NULL)
					{
<xsl:choose>
 <xsl:when test="@cont = 'map'">
						parent->_<xsl:value-of select="@child_name"/>->erase(getObjectId());
 </xsl:when>
 <xsl:when test="@cont = 'vector'">
						std::vector &lt; <xsl:value-of select="$className"/>Ptr &gt;::iterator it = std::find(parent->_<xsl:value-of select="@child_name"/>->begin(), parent->_<xsl:value-of select="@child_name"/>->end(), this);
						if (it != parent->_<xsl:value-of select="@child_name"/>->end())
						{
							parent->_<xsl:value-of select="@child_name"/>->erase(it);
						}
 </xsl:when>
 <xsl:otherwise>
	<xsl:message terminate="yes">
				ERROR : parent/child relation support only 'map' or 'vector' cont specification in <xsl:value-of select="$className"/> definition
	</xsl:message>
 </xsl:otherwise>
</xsl:choose>
					}
				}
				</xsl:for-each>
				// need to remove ref from parent (if any)
				<xsl:for-each select="parent[@relation = 'one-to-one']">
				{
					<xsl:variable name="db_col" select="@db_col"/>
					<xsl:value-of select="@class"/>Ptr parent(<xsl:value-of select="@class"/>::loadFromCache(_<xsl:value-of select="../property[@db_col = $db_col]/@name"/>, true), __FILE__, __LINE__);
					if (parent != NULL &amp;&amp; parent->_<xsl:value-of select="@child_name"/>Loaded)
					{
						// assign a new NULL pointer
						parent->_<xsl:value-of select="@child_name"/>.assign(<xsl:value-of select="$className"/>Ptr(), __FILE__, __LINE__);
					}
				}
				</xsl:for-each>

				return true;
			}
		}
		return false;
	}

	bool <xsl:value-of select="$className"/>::removeById(MSW::CConnection &amp;connection, uint32 id)
	{
		<xsl:value-of select="$className"/> *object = loadFromCache(id, true);
		if (object != NULL)
		{
			return object->remove(connection);
		}
		// not in cache, run a SQL query
		std::string qs;
		qs = "DELETE FROM <xsl:value-of select="database/@table"/> ";
		<xsl:call-template name="makeWhereClauseWithId">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
			<xsl:with-param name="id" select="'id'"/>
		</xsl:call-template>

		if (connection.query(qs))
		{
			if (connection.getAffectedRows() == 1)
			{
				// ok, the row is removed
				return true;
			}
		}

		return false;
	}


	// Try to load the specified object from the memory cache, return NULL if the object is not in the cache
	<xsl:value-of select="$className"/> *<xsl:value-of select="$className"/>::loadFromCache(uint32 objectId, bool unrelease)
	{
		// look in the cache
		TObjectCache::iterator it(_ObjectCache.find(objectId));
		if (it == _ObjectCache.end())
		{
			// not found, return null
			return NULL;
		}
		else
		{
			<xsl:value-of select="$className"/> *object = it->second;

			if (object->_ObjectState == NOPE::os_released)
			{
				if (unrelease)
				{
					// we need to remove this object from the released object set.
					object->removeFromReleased();
					object->_ObjectState = NOPE::os_clean;
				}
			}

			return it->second;
		}
	}
	// Receive and execute command from the cache manager.
	uint32 <xsl:value-of select="$className"/>::cacheCmd(NOPE::TCacheCmd cmd)
	{
		if (cmd == NOPE::cc_update)
		{
			updateCache();
		}
		else if (cmd == NOPE::cc_clear)
		{
			clearCache();
		}
		else if (cmd == NOPE::cc_dump)
		{
			dump();
		}
		else if (cmd == NOPE::cc_instance_count)
		{
			return _ObjectCache.size();
		}

		// default return value
		return 0;
	}

	void <xsl:value-of select="$className"/>::dump()
	{
		nlinfo("  Cache info for class <xsl:value-of select="$className"/> :");
		nlinfo("	There are %u object instances in cache", _ObjectCache.size());

		// count the number of object in the released object set
		uint32 nbReleased = 0;

		TReleasedObject::iterator first(_ReleasedObject.begin()), last(_ReleasedObject.end());
		for (; first != last; ++first)
		{
			nbReleased += first->second.size();
		}

		nlinfo("	There are %u object instances in cache not referenced (waiting deletion or re-use))", nbReleased);
	}

	void <xsl:value-of select="$className"/>::updateCache()
	{
		if (_ReleasedObject.empty())
			return;

		// 30 s hold in cache
		const time_t MAX_CACHE_OLD_TIME = 30;

		time_t now = NLMISC::CTime::getSecondsSince1970();

		// look for object set older than MAX_CACHE_OLD_TIME and delete them
		while (!_ReleasedObject.empty() &amp;&amp; _ReleasedObject.begin()-&gt;first &lt; now-MAX_CACHE_OLD_TIME)
		{
			TObjectSet &amp;delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				<xsl:value-of select="$className"/> *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void <xsl:value-of select="$className"/>::clearCache()
	{
		// remove any unreferenced object from the cache
		while (!_ReleasedObject.empty())
		{
			TObjectSet &amp;delSet = _ReleasedObject.begin()->second;
			// unload this objects
			while (!delSet.empty())
			{
				<xsl:value-of select="$className"/> *object = *delSet.begin();
				delete object;
			}

			_ReleasedObject.erase(_ReleasedObject.begin());
		}
	}

	void <xsl:value-of select="$className"/>::registerUpdatable()
	{
		static bool registered = false;
		if (!registered)
		{
			NOPE::CPersistentCache::getInstance().registerCache(&amp;<xsl:value-of select="$className"/>::cacheCmd);

			registered = true;
		}
	}

	// set the pointer on the first pointer of the pointer list (set to null when there is no more pointer)
	void <xsl:value-of select="$className"/>::setFirstPtr(<xsl:value-of select="$className"/>Ptr *ptr)
	{
		_PtrList = ptr;

		if (ptr == NULL)
		{
			// this is the last pointer !
			if (_ObjectState == NOPE::os_transient
				|| _ObjectState == NOPE::os_removed)
			{
				// not a persistent object, or removed object, just delet it
				delete this;
			}
			else if (_ObjectState != NOPE::os_removed)
			{
				setPersistentState(NOPE::os_released);
			}
		}
	}

	// Set the persistent state of the object and do some house keeping
	void <xsl:value-of select="$className"/>::setPersistentState(NOPE::TObjectState state)
	{
		nlassert(NOPE::AllowedTransition[_ObjectState][state] == true);

		if(_ObjectState == NOPE::os_released &amp;&amp; state == NOPE::os_removed)
		{
			// a release object gets removed (e.g. by remove by id)

			// delete the object
			delete this;

			// no more to do
			return;
		}

		if (_ObjectState == NOPE::os_transient &amp;&amp; state != NOPE::os_transient)
		{
			nldebug("NOPE: inserting <xsl:value-of select="$className"/> @%p in cache with id %u", this, static_cast&lt;uint32&gt;(_<xsl:value-of select="$uniqueId"/>));
			nlverify(_ObjectCache.insert(std::make_pair(_<xsl:value-of select="$uniqueId"/>, this)).second);
		}

		if (_ObjectState != NOPE::os_transient)
			nlassert(_ObjectCache.find(_<xsl:value-of select="$uniqueId"/>) != _ObjectCache.end());

		_ObjectState = state;

		if (state == NOPE::os_released)
		{
			_ReleaseDate = NLMISC::CTime::getSecondsSince1970();
			nlverify(_ReleasedObject[_ReleaseDate].insert(this).second);
		}
		else if (state == NOPE::os_removed)
		{
			nldebug("NOPE: erasing <xsl:value-of select="$className"/> @%p in cache with id %u", this, static_cast&lt;uint32&gt;(_<xsl:value-of select="$uniqueId"/>));
			nlverify(_ObjectCache.erase(_<xsl:value-of select="$uniqueId"/>) == 1);
		}
	}


	<xsl:value-of select="$className"/>Ptr <xsl:value-of select="$className"/>::load(MSW::CConnection &amp;connection, uint32 id, const char *filename, uint32 lineNum)
	{
		<xsl:value-of select="$className"/> *inCache = loadFromCache(id, true);
		if (inCache != NULL)
		{
			return <xsl:value-of select="$className"/>Ptr(inCache, filename, lineNum);
		}

		std::string qs;
		qs = "SELECT ";
		<xsl:call-template name="makeColumListWithId"/>
		qs += " FROM <xsl:value-of select="database/@table"/>";
		<xsl:call-template name="makeWhereClauseWithId">
			<xsl:with-param name="uniqueId" select="$uniqueId"/>
			<xsl:with-param name="id" select="'id'"/>
		</xsl:call-template>

		<xsl:value-of select="$className"/>Ptr ret;
		if (!connection.query(qs))
		{
			return ret;
		}

		MSW::CStoreResult *result = connection.storeResult().release();

		nlassert(result->getNumRows() &lt;= 1);
		if (result->getNumRows() == 1)
		{
			ret.assign(new <xsl:value-of select="$className"/>, filename, lineNum);
			// ok, we have an object
			result->fetchRow();<xsl:text>

</xsl:text>	<xsl:for-each select="property[@db_col]">
				<xsl:choose>
					<xsl:when test="@enum='true' or @enum='smart'">
<xsl:text>			{
				std::string s;
				result->getField(</xsl:text><xsl:value-of select="position()-1"/>, s);
				ret->_<xsl:value-of select="@name"/> = <xsl:value-of select="@type"/><xsl:text>(s);
			}
</xsl:text>
					</xsl:when>
					<xsl:when test="@date='true'">
<xsl:text>			result->getDateField(</xsl:text><xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>)<xsl:text>;
</xsl:text>
					</xsl:when>
					<xsl:when test="@md5='true'">
<xsl:text>			result->getMD5Field(</xsl:text><xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>)<xsl:text>;
</xsl:text>
					</xsl:when>
					<xsl:otherwise>
<xsl:text>			result->getField(</xsl:text><xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>)<xsl:text>;
</xsl:text>
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>

			ret->setPersistentState(NOPE::os_clean);
		}

		delete result;

		return ret;
	}

<xsl:for-each select="parent[@relation = 'one-to-many']">
<xsl:choose>
<xsl:when test="@cont='map'">
	bool <xsl:value-of select="$className"/>::loadChildrenOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, std::map &lt; uint32, <xsl:value-of select="$className"/>Ptr &gt; &amp; container, const char *filename, uint32 lineNum)
</xsl:when>
<xsl:when test="@cont='vector'">
	bool <xsl:value-of select="$className"/>::loadChildrenOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, std::vector &lt; <xsl:value-of select="$className"/>Ptr &gt; &amp; container, const char *filename, uint32 lineNum)
</xsl:when>
<xsl:otherwise>
	<xsl:message terminate="yes">
ERROR : parent/child relation support only 'map' or 'vector' cont specification in <xsl:value-of select="$className"/>, <xsl:value-of select="@class"/> parent definition
	</xsl:message>
</xsl:otherwise>
</xsl:choose>
	{
		std::string qs;
		qs = "SELECT ";
<xsl:for-each select="..">
		<xsl:call-template name="makeColumListWithId"/>
</xsl:for-each>
		qs += " FROM <xsl:value-of select="../database/@table"/>";
		qs += " WHERE <xsl:value-of select="@db_col"/> = '"+NLMISC::toString(parentId)+"'";

		if (!connection.query(qs))
		{
			return false;
		}

		std::auto_ptr&lt;MSW::CStoreResult&gt; result = connection.storeResult();

		for (uint i=0; i&lt;result->getNumRows(); ++i)
		{
			<xsl:value-of select="$className"/> *ret = new <xsl:value-of select="$className"/>();
			// ok, we have an object
			result->fetchRow();
			<xsl:for-each select="../property[@db_col]">
				<xsl:choose>
					<xsl:when test="@enum='true' or @enum='smart'">
			{
				std::string s;
				result->getField(<xsl:value-of select="position()-1"/>, s);
				ret->_<xsl:value-of select="@name"/> = <xsl:value-of select="@type"/>(s);
			}
					</xsl:when>
					<xsl:when test="@date='true'">
			result->getDateField(<xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>);
					</xsl:when>
					<xsl:when test="@md5='true'">
			result->getMD5Field(<xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>);
					</xsl:when>
					<xsl:otherwise>
			result->getField(<xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>);
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>

	<xsl:value-of select="$className"/> *inCache = loadFromCache(ret->_<xsl:value-of select="$uniqueId"/>, true);
			if (inCache != NULL)
			{
<xsl:choose>
<xsl:when test="@cont='map'">
				container.insert(std::make_pair(inCache->getObjectId(), <xsl:value-of select="$className"/>Ptr(inCache, filename, lineNum)));
</xsl:when>
<xsl:when test="@cont='vector'">
				container.push_back(<xsl:value-of select="$className"/>Ptr(inCache, filename, lineNum));
</xsl:when>
</xsl:choose>
				// no more needed
				delete ret;
			}
			else
			{
				ret->setPersistentState(NOPE::os_clean);
<xsl:choose>
<xsl:when test="@cont='map'">
				container.insert(std::make_pair(ret->getObjectId(), <xsl:value-of select="$className"/>Ptr(ret, filename, lineNum)));
</xsl:when>
<xsl:when test="@cont='vector'">
				container.push_back(<xsl:value-of select="$className"/>Ptr(ret, filename, lineNum));
</xsl:when>
</xsl:choose>
			}
		}

		return true;
	}
</xsl:for-each>

<xsl:for-each select="parent[@relation = 'one-to-one']">
<xsl:text>	bool </xsl:text><xsl:value-of select="$className"/>::loadChildOf<xsl:value-of select="@class"/>(MSW::CConnection &amp;connection, uint32 parentId, <xsl:value-of select="../@name"/>Ptr &amp;childPtr, const char *filename, uint32 lineNum)
	{
		std::string qs;
		qs = "SELECT ";
<xsl:for-each select="..">
		<xsl:call-template name="makeColumListWithId"/>
</xsl:for-each>
		qs += " FROM <xsl:value-of select="../database/@table"/>";
		qs += " WHERE <xsl:value-of select="@db_col"/> = '"+NLMISC::toString(parentId)+"'";

		<xsl:value-of select="$className"/>Ptr ret;
		if (!connection.query(qs))
		{
			childPtr = <xsl:value-of select="../@name"/>Ptr();
			return false;
		}

		std::auto_ptr&lt;MSW::CStoreResult&gt; result = connection.storeResult();

		// check that the data description is consistent with database content
		nlassert(result->getNumRows() &lt;= 1);

		if (result->getNumRows() == 1)
		{
			<xsl:value-of select="$className"/> *object = new <xsl:value-of select="$className"/>;
			// ok, we have an object
			result->fetchRow();
			<xsl:for-each select="../property[@db_col]">
				<xsl:choose>
					<xsl:when test="@enum='true' or @enum='smart'">
			{
				std::string s;
				result->getField(<xsl:value-of select="position()-1"/>, s);
				object->_<xsl:value-of select="@name"/> = <xsl:value-of select="@type"/>(s);
			}
					</xsl:when>
					<xsl:when test="@date='true'">
			result->getDateField(<xsl:value-of select="position()-1"/>, object->_<xsl:value-of select="@name"/>);
					</xsl:when>
					<xsl:when test="@md5='true'">
			result->getMD5Field(<xsl:value-of select="position()-1"/>, ret->_<xsl:value-of select="@name"/>);
					</xsl:when>
					<xsl:otherwise>
			result->getField(<xsl:value-of select="position()-1"/>, object->_<xsl:value-of select="@name"/>);
					</xsl:otherwise>
				</xsl:choose>
			</xsl:for-each>

	<xsl:value-of select="$className"/> *inCache = loadFromCache(object->_<xsl:value-of select="$uniqueId"/>, true);
			if (inCache != NULL)
			{
				ret.assign(inCache, filename, lineNum);
				// no more needed
				delete object;
			}
			else
			{
				object->setPersistentState(NOPE::os_clean);
				ret.assign(object, filename, lineNum);
			}

			childPtr = ret;
			return true;
		}

		// no result, but no error
		childPtr = <xsl:value-of select="../@name"/>Ptr();
		return true;
	}
</xsl:for-each>


<xsl:for-each select="child_class[@relation = 'one-to-many']">
	bool <xsl:value-of select="$className"/>::load<xsl:value-of select="@name"/>(MSW::CConnection &amp;connection, const char *filename, uint32 lineNum)
	{
		bool ret = true;
		if (_<xsl:value-of select="@name"/> != NULL)
		{
			// the children are already loaded, just return true
			return true;
		}

		// allocate the container
<xsl:choose>
<xsl:when test="@cont = 'map'">		_<xsl:value-of select="@name"/> = new std::map &lt; uint32,  <xsl:value-of select="@type"/>Ptr &gt;;</xsl:when>
<xsl:when test="@cont = 'vector'">		_<xsl:value-of select="@name"/> = new std::vector &lt; <xsl:value-of select="@type"/>Ptr &gt;;</xsl:when>
</xsl:choose>

		// load the childs
		ret &amp;= <xsl:value-of select="@type"/>::loadChildrenOf<xsl:value-of select="../@name"/>(connection, getObjectId(), *_<xsl:value-of select="@name"/>, filename, lineNum);
		return ret;
	}

<xsl:choose>
	<xsl:when test="@cont = 'vector'">
	const std::vector&lt;<xsl:value-of select="@type"/>Ptr&gt; &amp;<xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>() const
	{
		nlassert(_<xsl:value-of select="@name"/> != NULL);
		return *_<xsl:value-of select="@name"/>;
	}

	<xsl:value-of select="@type"/>Ptr &amp;<xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>ByIndex(uint32 index) const
	{
		nlassert(_<xsl:value-of select="@name"/> != NULL);
		nlassert(index &lt; _<xsl:value-of select="@name"/>->size());
		return const_cast&lt; <xsl:value-of select="@type"/>Ptr &amp; &gt;(_<xsl:value-of select="@name"/>->operator[](index));
	}

	<xsl:value-of select="@type"/>Ptr &amp;<xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>ById(uint32 id) const
	{
		nlassert(_<xsl:value-of select="@name"/> != NULL);
		std::vector&lt;<xsl:value-of select="@type"/>Ptr &gt;::const_iterator first(_<xsl:value-of select="@name"/>->begin()), last(_<xsl:value-of select="@name"/>->end());
		for (; first != last; ++first)
		{
			const <xsl:value-of select="@type"/>Ptr &amp;child = *first;
			if (child->getObjectId() == id)
			{
				return const_cast&lt; <xsl:value-of select="@type"/>Ptr &amp; &gt;(child);
			}
		}

		// no object with this id, return a null pointer
		static <xsl:value-of select="@type"/>Ptr nil;

		return nil;
	}

	</xsl:when>
	<xsl:when test="@cont = 'map'">
	const std::map&lt;uint32, <xsl:value-of select="@type"/>Ptr&gt; &amp;<xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>() const
	{
		nlassert(_<xsl:value-of select="@name"/> != NULL);
		return *_<xsl:value-of select="@name"/>;
	}

	<xsl:value-of select="@type"/>Ptr &amp;<xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>ById(uint32 id) const
	{
		nlassert(_<xsl:value-of select="@name"/> != NULL);
		std::map&lt;uint32, <xsl:value-of select="@type"/>Ptr&gt;::const_iterator it(_<xsl:value-of select="@name"/>->find(id));

		if (it == _<xsl:value-of select="@name"/>->end())
		{
			// no object with this id, return a null pointer
			static <xsl:value-of select="@type"/>Ptr nil;
			return nil;
		}

		return const_cast&lt; <xsl:value-of select="@type"/>Ptr &amp; &gt;(it->second);
	}

	</xsl:when>
	<xsl:otherwise>
			<xsl:message terminate="yes">
				ERROR : Invalide container, child must be either map or vector for child <xsl:value-of select="@name"/> in class <xsl:value-of select="../@name"/><xsl:text>
</xsl:text>
			</xsl:message>
	</xsl:otherwise>
</xsl:choose>


</xsl:for-each>
<xsl:for-each select="child_class[@relation = 'one-to-one']">
	bool <xsl:value-of select="$className"/>::load<xsl:value-of select="@name"/>(MSW::CConnection &amp;connection, const char *filename, uint32 lineNum)
	{
		if (_<xsl:value-of select="@name"/>Loaded)
		{
			// the child is already loaded, just return true
			return true;
		}
 		bool ret = <xsl:value-of select="@type"/>::loadChildOf<xsl:value-of select="../@name"/>(connection, getObjectId(), _<xsl:value-of select="@name"/>, filename, lineNum);
		_<xsl:value-of select="@name"/>Loaded = true;
		return ret;
	}

	/** Return the one child object (or null if not) */
	<xsl:value-of select="@type"/>Ptr <xsl:value-of select="$className"/>::get<xsl:value-of select="@name"/>()
	{
		nlassert(_<xsl:value-of select="@name"/>Loaded);
		return _<xsl:value-of select="@name"/>;
	}

</xsl:for-each>
</xsl:if>
	</xsl:template>



	<!-- ################################################################# -->
	<!-- #####         C++ Layer 3 interface generation (server mode)  ### -->
	<!-- ################################################################# -->
	<!-- #####  header file #### -->
	<xsl:template match="callback_interface" mode="header">
<xsl:call-template name="makeMasterDoc"/>
<xsl:choose>
<xsl:when test="@extend">
	class <xsl:value-of select="@name"/>Itf : public <xsl:value-of select="@extend"/>Itf
</xsl:when>
<xsl:otherwise>
	class <xsl:value-of select="@name"/>Itf <!--: public NLNET::CCallbackServer-->
</xsl:otherwise>
</xsl:choose>
	{
	protected:
<xsl:if test="not(@extend)">
		/// the callback server adaptor
		std::auto_ptr&lt;ICallbackServerAdaptor&gt;	_CallbackServer;
</xsl:if>
		void getCallbakArray(NLNET::TCallbackItem *&amp;arrayPtr, uint32 &amp;arraySize)
		{
			static NLNET::TCallbackItem callbackArray[] =
			{
<xsl:for-each select="invoke">				{	"<xsl:value-of select="@msg"/>",	<xsl:value-of select="../@name"/>Itf::cb_<xsl:value-of select="@name"/>	},
</xsl:for-each>
			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
		}

		static void _cbConnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(<xsl:value-of select="@name"/>__cbConnection);
			<xsl:value-of select="@name"/>Itf *_this = reinterpret_cast&lt;<xsl:value-of select="@name"/>Itf *&gt;(arg);

			_this->on_<xsl:value-of select="@name"/>_Connection(from);
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			H_AUTO(<xsl:value-of select="@name"/>__cbDisconnection);
			<xsl:value-of select="@name"/>Itf *_this = reinterpret_cast&lt;<xsl:value-of select="@name"/>Itf *&gt;(arg);

			_this->on_<xsl:value-of select="@name"/>_Disconnection(from);
		}


	public:
		/** Constructor, if you specify a replacement adaptor, then the object
		 *	become owner of the adaptor (and it will be released with the
		 *	interface).
		 */
<xsl:if test="@extend">
<xsl:text>		</xsl:text><xsl:value-of select="@name"/>Itf(ICallbackServerAdaptor *replacementAdaptor = NULL)
			:	<xsl:value-of select="@extend"/>Itf(replacementAdaptor)
		{}
</xsl:if>
<xsl:if test="not(@extend)">
<xsl:text>		</xsl:text><xsl:value-of select="@name"/>Itf(ICallbackServerAdaptor *replacementAdaptor = NULL)
		{
			if (replacementAdaptor == NULL)
			{
				// use default callback server
				_CallbackServer = std::auto_ptr&lt;ICallbackServerAdaptor&gt;(new CNelCallbackServerAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackServer = std::auto_ptr&lt;ICallbackServerAdaptor&gt;(replacementAdaptor);
			}
		}
</xsl:if>
		virtual ~<xsl:value-of select="@name"/>Itf()
		{
		}

		/// Open the interface socket in the specified port
		void openItf(uint16 port)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

<xsl:if test="@extend">
			// add callback array of the base interface class
			<xsl:value-of select="@extend"/>Itf::getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);
</xsl:if>

			getCallbakArray(arrayPtr, arraySize);
			_CallbackServer->addCallbackArray(arrayPtr, arraySize);

			_CallbackServer->setConnectionCallback (_cbConnection, this);
			_CallbackServer->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackServer->init(port);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch method invokation.
		 */
		void update()
		{
			H_AUTO(<xsl:value-of select="@name"/>_update);

			try
			{
				_CallbackServer->update();
			}
			catch (...)
			{
				nlwarning("<xsl:value-of select="@name"/> : Exception launch in callback server update");
			}
		}

<xsl:for-each select="return">
<xsl:call-template name="makeMethodDoc"/>
		void <xsl:value-of select="@name"/>(NLNET::TSockId dest<xsl:call-template name="makeParamList"/>)
		{
			H_AUTO(<xsl:value-of select="@name"/>_<xsl:value-of select="@name"/>);
#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>::<xsl:value-of select="@name"/> called");
#endif
			NLNET::CMessage message("<xsl:value-of select="@msg"/>");
<xsl:for-each select="param">
<xsl:call-template name="serialWrite"/>
</xsl:for-each>
			_CallbackServer->send(message, dest);
		}
</xsl:for-each>

<xsl:for-each select="invoke">
		static void cb_<xsl:value-of select="@name"/> (NLNET::CMessage &amp;message, NLNET::TSockId from, NLNET::CCallbackNetBase &amp;netbase)
		{
			H_AUTO(<xsl:value-of select="@name"/>_on_<xsl:value-of select="@name"/>);
#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>::cb_<xsl:value-of select="@name"/> received from class '%s'", typeid(netbase).name());
#endif
			ICallbackServerAdaptor *adaptor = static_cast&lt; ICallbackServerAdaptor *&gt;(netbase.getUserData());

			<xsl:value-of select="../@name"/>Itf *callback = (<xsl:value-of select="../@name"/>Itf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
<xsl:for-each select="param">
<xsl:if test="not(@array)">
<xsl:text>			</xsl:text><xsl:value-of select="@type"/><xsl:text>	</xsl:text><xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
</xsl:if>
<xsl:if test="@array = 'true'">
<xsl:text>			</xsl:text>std::vector&lt;<xsl:value-of select="@type"/>&gt;<xsl:text>	</xsl:text><xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
</xsl:if>
</xsl:for-each>
<xsl:for-each select="param">
<xsl:call-template name="serialRead"/>
</xsl:for-each>

#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>::cb_<xsl:value-of select="@name"/> : calling on_<xsl:value-of select="@name"/>");
#endif

<xsl:if test="not(return)">
			callback->on_<xsl:value-of select="@name"/>(from<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);
</xsl:if>
<xsl:if test="return">
<xsl:if test="not(return/@array)">
<xsl:text>			</xsl:text><xsl:value-of select="return/@type"/> retValue;
</xsl:if>
<xsl:if test="return/@array">
<xsl:text>			std::vector&lt;</xsl:text><xsl:value-of select="return/@type"/>&gt; retValue;
</xsl:if>
			retValue = callback->on_<xsl:value-of select="@name"/>(from<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);

			NLNET::CMessage retMsg("R_<xsl:value-of select="@msg"/>");
<xsl:if test="not(return/@array)">
			nlWrite(retMsg, serial<xsl:value-of select="return/@serial"/>, retValue);
</xsl:if>
<xsl:if test="return/@array">
			nlWrite(retMsg, serialCont, retValue);
</xsl:if>

			callback->_CallbackServer->send(retMsg, from);
</xsl:if>
		}
</xsl:for-each>

		/// Connection callback : a new interface client connect
		virtual void on_<xsl:value-of select="@name"/>_Connection(NLNET::TSockId from) =0;
		/// Disconnection callback : one of the interface client disconnect
		virtual void on_<xsl:value-of select="@name"/>_Disconnection(NLNET::TSockId from) =0;

<xsl:for-each select="invoke"><xsl:text>
</xsl:text><xsl:call-template name="makeMethodDoc"/>
<xsl:if test="not(return)">
<xsl:text>		virtual void on_</xsl:text><xsl:value-of select="@name"/>(NLNET::TSockId from<xsl:call-template name="makeParamList"/>) =0;
</xsl:if>
<xsl:if test="return">
<xsl:if test="not(return/@array)">
<xsl:text>		virtual </xsl:text><xsl:value-of select="return/@type"/> on_<xsl:value-of select="@name"/>(NLNET::TSockId from<xsl:call-template name="makeParamList"/>) =0;
</xsl:if>
<xsl:if test="return/@array">
<xsl:text>		virtual std::vector&lt;</xsl:text><xsl:value-of select="return/@type"/>&gt; on_<xsl:value-of select="@name"/>(NLNET::TSockId from<xsl:call-template name="makeParamList"/>) =0;
</xsl:if>
</xsl:if>
</xsl:for-each>
	};

	<!-- Callback interface client class -->
<xsl:call-template name="makeMasterDoc"/>
	/** This is the client side of the interface
	 *	Derive from this class to invoke method on the callback server
	 */
<xsl:choose>
<xsl:when test="@extend">
	class <xsl:value-of select="@name"/>ClientItf : public <xsl:value-of select="@extend"/>ClientItf
</xsl:when>
<xsl:otherwise>
	class <xsl:value-of select="@name"/>ClientItf <!--: public NLNET::CCallbackClient-->
</xsl:otherwise>
</xsl:choose>
	{
	protected:
<xsl:if test="not(@extend)">
		/// the callback client adaptor
		std::auto_ptr &lt; ICallbackClientAdaptor &gt;	_CallbackClient;
</xsl:if>

		void getCallbakArray(NLNET::TCallbackItem *&amp;arrayPtr, uint32 &amp;arraySize)
		{
<xsl:if test="not(return)">
			arrayPtr = NULL;
			arraySize = 0;
</xsl:if>

<xsl:if test="return">
			static NLNET::TCallbackItem callbackArray[] =
			{
<xsl:for-each select="return">				{	"<xsl:value-of select="@msg"/>",	<xsl:value-of select="../@name"/>ClientItf::cb_<xsl:value-of select="@name"/>	},
</xsl:for-each>
			};

			arrayPtr = callbackArray;
			arraySize = sizeofarray(callbackArray);
</xsl:if>
		}

		static void _cbDisconnection(NLNET::TSockId from, void *arg)
		{
			<xsl:value-of select="@name"/>ClientItf *_this = reinterpret_cast&lt;<xsl:value-of select="@name"/>ClientItf *&gt;(arg);

			_this->on_<xsl:value-of select="@name"/>Client_Disconnection(from);
		}


	public:
		/// Retreive the message name for a given callback name
		static const std::string &amp;getMessageName(const std::string &amp;methodName)
		{
			static std::map&lt;std::string, std::string&gt; messageNames;
			static bool initialized = false;
			if (!initialized)
			{
<xsl:for-each select="return">			messageNames.insert(std::make_pair(std::string("on_<xsl:value-of select="@name"/>"), std::string("<xsl:value-of select="@msg"/>")));
</xsl:for-each>
				initialized = true;
			}

			std::map &lt; std::string, std::string&gt;::const_iterator it(messageNames.find(methodName));
			if (it != messageNames.end())
				return it->second;

<xsl:if test="@extend">
			// try with the base class
			return <xsl:value-of select="@extend"/>ClientItf::getMessageName(methodName);
</xsl:if>
<xsl:if test="not(@extend)">
			static std::string emptyString;

			return emptyString;
</xsl:if>
		}

<xsl:if test="@extend">
<xsl:text>		</xsl:text><xsl:value-of select="@name"/>ClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
			:	<xsl:value-of select="@extend"/>ClientItf(adaptorReplacement)
		{}
</xsl:if>
<xsl:if test="not(@extend)">
<xsl:text>		</xsl:text><xsl:value-of select="@name"/>ClientItf(ICallbackClientAdaptor *adaptorReplacement = NULL)
		{
			if (adaptorReplacement == NULL)
			{
				// use the default Nel adaptor
				_CallbackClient = std::auto_ptr &lt; ICallbackClientAdaptor &gt;(new CNelCallbackClientAdaptor(this));
			}
			else
			{
				// use the replacement one
				_CallbackClient = std::auto_ptr &lt; ICallbackClientAdaptor &gt;(adaptorReplacement);
			}
		}
</xsl:if>
		/// Connect the interface client to the callback server at the specified address and port
		virtual void connectItf(NLNET::CInetAddress address)
		{
			NLNET::TCallbackItem *arrayPtr;
			uint32 arraySize;

			static bool callbackAdded = false;
			if (!callbackAdded)
			{
<xsl:if test="@extend">
				// add callback array of the base interface class
				<xsl:value-of select="@extend"/>ClientItf::getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
				callbackAdded = true;
				// add callback array of this interface
</xsl:if>
				getCallbakArray(arrayPtr, arraySize);
				_CallbackClient->addCallbackArray(arrayPtr, arraySize);
			}

			_CallbackClient->setDisconnectionCallback (_cbDisconnection, this);

			_CallbackClient->connect(address);
		}

		/** Must be called evenly, update the network subclass to receive message
		 *	and dispatch invokation returns.
		 */
		virtual void update()
		{
			H_AUTO(<xsl:value-of select="@name"/>_update);

			try
			{
				_CallbackClient->update();
			}
			catch (...)
			{
				nlwarning("<xsl:value-of select="@name"/> : Exception launch in callback client update");
			}
		}

<xsl:for-each select="invoke">
<xsl:call-template name="makeMethodDoc"/>
		void <xsl:value-of select="@name"/>(<xsl:call-template name="makeParamListNoStartingComma"/>)
		{
#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>Client::<xsl:value-of select="@name"/> called");
#endif
			NLNET::CMessage message("<xsl:value-of select="@msg"/>");
<xsl:for-each select="param">
<xsl:call-template name="serialWrite"/>
</xsl:for-each>
			_CallbackClient->send(message);
		}
</xsl:for-each>

<xsl:for-each select="return">
		static void cb_<xsl:value-of select="@name"/> (NLNET::CMessage &amp;message, NLNET::TSockId from, NLNET::CCallbackNetBase &amp;netbase)
		{
#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>Client::cb_<xsl:value-of select="@name"/> received from class '%s'", typeid(netbase).name());
#endif
			ICallbackClientAdaptor *adaptor = static_cast&lt; ICallbackClientAdaptor *&gt;(netbase.getUserData());

			<xsl:value-of select="../@name"/>ClientItf *callback = (<xsl:value-of select="../@name"/>ClientItf *)adaptor->getContainerClass();

			if (callback  == NULL)
				return;
<xsl:for-each select="param">

<xsl:if test="not(@array)">
<xsl:text>			</xsl:text><xsl:value-of select="@type"/><xsl:text>	</xsl:text><xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
</xsl:if>

<xsl:if test="@array = 'true'">
<xsl:text>			</xsl:text>std::vector&lt;<xsl:value-of select="@type"/>&gt;	<xsl:value-of select="@name"/>;<xsl:text>
</xsl:text>
</xsl:if>

</xsl:for-each>
<xsl:for-each select="param">
<xsl:call-template name="serialRead"/>
</xsl:for-each>

#ifdef NL_DEBUG
			nldebug("<xsl:value-of select="../@name"/>Client::cb_<xsl:value-of select="@name"/> : calling on_<xsl:value-of select="@name"/>");
#endif

			callback->on_<xsl:value-of select="@name"/>(from<xsl:for-each select="param">, <xsl:value-of select="@name"/></xsl:for-each>);
		}
</xsl:for-each>

		/// Disconnection callback : the connection to the server is lost
		virtual void on_<xsl:value-of select="@name"/>Client_Disconnection(NLNET::TSockId from) =0;

<xsl:for-each select="return"><xsl:text>
</xsl:text><xsl:call-template name="makeMethodDoc"/>
<xsl:text>		virtual void on_</xsl:text><xsl:value-of select="@name"/>(NLNET::TSockId from<xsl:call-template name="makeParamList"/>) =0;
</xsl:for-each>
	};
</xsl:template>


	<!-- ######################################################### -->
	<!-- #####         PHP Layer 3 interface generation         ### -->
	<!-- ######################################################### -->
	<!-- #####  php file #### -->
	<xsl:template match="callback_interface[@caller = 'php']" mode="php">
<xsl:if test=".//param[@type != 'uint32'
				and @type != 'uint8'
				and @type != 'std::string'
				and @enum != 'smart']">
	<xsl:message terminate="yes">
		ERROR : PHP interface only support uint8, uint32, enum and std::string parameter in callback interface '<xsl:value-of select="@name"/>.<xsl:value-of select=".//param[@type != 'uint32' and @type != 'uint8' and @type != 'std::string']/../@name"/>'
	</xsl:message>
</xsl:if>
<xsl:text>&lt;?php
	/////////////////////////////////////////////////////////////////
	// WARNING : this is a generated file, don't change it !
	/////////////////////////////////////////////////////////////////

	require_once('../tools/nel_message.php');

	class </xsl:text><xsl:value-of select="@name"/> extends CCallbackClient
	{
<xsl:for-each select="invoke">
		function <xsl:value-of select="@name"/>(<xsl:call-template name="makePhpArgListNoFollow"/>)
		{
			$msg = new CMessage;
			$msg->setName("<xsl:value-of select="@msg"/>");


<xsl:for-each select="param">

<xsl:choose>
<xsl:when test="@php_serial">
	<xsl:call-template name="makeWriteSerial">
		<xsl:with-param name="msgName" select="'$msg'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="concat('serial', @php_serial)"/>
	</xsl:call-template>
</xsl:when>
<xsl:when test="@enum">
	<xsl:call-template name="makeWriteSerial">
		<xsl:with-param name="msgName" select="'$msg'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="'serialEnum'"/>
	</xsl:call-template>
</xsl:when>
<xsl:otherwise>
	<xsl:call-template name="makeWriteSerial">
		<xsl:with-param name="msgName" select="'$msg'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="'serialUint32'"/>
	</xsl:call-template>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>

<xsl:if test="not(return)">
			return parent::sendMessage($msg);
</xsl:if>
<xsl:if test="return">
			<!-- this is a two way call -->
			$ret = "";
			$ret = parent::sendMessage($msg);
			if ($ret == false)
			{
				// error during send
				$this->invokeError("<xsl:value-of select="@name"/>", "Error in 'sendMessage'");
				return false;
			}

			$retMsg = parent::waitMessage();
			if ($ret == false)
			{
				// error during send
				$this->invokeError("<xsl:value-of select="@name"/>", "Error in 'waitMessage'");
				return false;
			}
			if (!($retMsg->MsgName === "R_<xsl:value-of select="@msg"/>"))
			{
				// error during send
				$this->invokeError("<xsl:value-of select="@name"/>", "Invalid response, awaited 'R_<xsl:value-of select="@msg"/>', received '".$retMsg->MsgName."'");
				return false;
			}

			// serial the return value
			<xsl:choose>
			<xsl:when test="return/@php_serial">
				<xsl:call-template name="makeReadSerial">
					<xsl:with-param name="paramNode" select="return"/>
					<xsl:with-param name="msgName" select="'$retMsg'"/>
					<xsl:with-param name="varName" select="'retValue'"/>
					<xsl:with-param name="serialName" select="concat('serial', return/@php_serial)"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="return/@enum">
			$<xsl:value-of select="@varName"/> = new <xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@type"/>;
				<xsl:call-template name="makeReadSerial">
					<xsl:with-param name="paramNode" select="return"/>
					<xsl:with-param name="return"/>
					<xsl:with-param name="msgName" select="'$retMsg'"/>
					<xsl:with-param name="varName" select="'retValue'"/>
					<xsl:with-param name="serialName" select="'serialEnum'"/>
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
				<xsl:call-template name="makeReadSerial">
					<xsl:with-param name="paramNode" select="return"/>
					<xsl:with-param name="return"/>
					<xsl:with-param name="msgName" select="'$retMsg'"/>
					<xsl:with-param name="varName" select="'retValue'"/>
					<xsl:with-param name="serialName" select="'serialUint32'"/>
				</xsl:call-template>
			</xsl:otherwise>
			</xsl:choose>

			// return the return value
			return $retValue;
</xsl:if>

		}
</xsl:for-each>

		function waitCallback()
		{
			$message = parent::waitMessage();

			if ($message == false)
				return false;

			switch($message->MsgName)
			{
<xsl:for-each select="return">			case "<xsl:value-of select="@msg"/>":
				$this-><xsl:value-of select="@name"/>_skel($message);
				break;
</xsl:for-each>			default:
				return false;
			}

			return true;
		}

<xsl:for-each select="return">
		function <xsl:value-of select="@name"/>_skel(&amp;$message)
		{
<xsl:for-each select="param">
<xsl:text>			</xsl:text>
<xsl:choose>
<xsl:when test="@php_serial">
	<xsl:call-template name="makeReadSerial">
		<xsl:with-param name="paramNode" select="."/>
		<xsl:with-param name="msgName" select="		'$message'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="concat('serial', @php_serial)"/>
	</xsl:call-template>
</xsl:when>
<xsl:when test="@enum">
			$<xsl:value-of select="@name"/> = new <xsl:value-of select="//namespace/@name"/>_<xsl:value-of select="@type"/>;
	<xsl:call-template name="makeReadSerial">
		<xsl:with-param name="paramNode" select="."/>
		<xsl:with-param name="msgName" select="		'$message'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="'serialEnum'"/>
	</xsl:call-template>
</xsl:when>
<xsl:otherwise>
	<xsl:call-template name="makeReadSerial">
		<xsl:with-param name="paramNode" select="."/>
		<xsl:with-param name="msgName" select="		'$message'"/>
		<xsl:with-param name="varName" select="@name"/>
		<xsl:with-param name="serialName" select="'serialUint32'"/>
	</xsl:call-template>
</xsl:otherwise>
</xsl:choose>
</xsl:for-each>
<xsl:text>
			$this-&gt;</xsl:text><xsl:value-of select="@name"/>(<xsl:call-template name="makePhpArgListNoFollow"/>);
		}
</xsl:for-each>

		/////////////////////////////////////////////////////////////////
		// Copy paste this part of code in your derived class
		//	and implement code to ract to incoming message
		/////////////////////////////////////////////////////////////////
<xsl:for-each select="return">
<xsl:call-template name="makeMethodDoc"/>
		function <xsl:value-of select="@name"/>(<xsl:call-template name="makePhpArgListNoFollow"/>)
		{
		}

</xsl:for-each>	}
<xsl:text>?&gt;
</xsl:text>
	</xsl:template>


	<!-- ######################################### -->
	<!-- #####  PHP Read serialisation code   ###### -->
	<!-- ######################################### -->

<xsl:template name="makeReadSerial">
	<xsl:param name="paramNode"/>
	<xsl:param name="msgName"/>
	<xsl:param name="varName"/>
	<xsl:param name="serialName"/>


	<xsl:choose>
	<xsl:when test="$paramNode/@array">			$nbElem = 0;
			<xsl:value-of select="$msgName"/>->serialUInt32($nbElem);
			$<xsl:value-of select="$varName"/> = array();
			for ($i=0; $i&lt;$nbElem;$i++)
			{
				<xsl:value-of select="$msgName"/>-><xsl:value-of select="$serialName"/>($item);
				$<xsl:value-of select="$varName"/>[] = $item;
			}
	</xsl:when>
	<xsl:otherwise>			<xsl:value-of select="$msgName"/>-><xsl:value-of select="$serialName"/>($<xsl:value-of select="$varName"/>);
	</xsl:otherwise>
	</xsl:choose>

</xsl:template>

	<!-- ######################################### -->
	<!-- #####  PHP Write serialisation code   ###### -->
	<!-- ######################################### -->

<xsl:template name="makeWriteSerial">
	<xsl:param name="msgName"/>
	<xsl:param name="varName"/>
	<xsl:param name="serialName"/>

	<xsl:choose>
	<xsl:when test="@array"><xsl:text>			</xsl:text><xsl:value-of select="$msgName"/>->serialUint32(count($<xsl:value-of select="@name"/>));
				foreach($<xsl:value-of select="$varName"/> as $key=>$value)
					<xsl:value-of select="$msgName"/>-><xsl:value-of select="$serialName"/>($value);
	</xsl:when>
	<xsl:otherwise><xsl:text>			</xsl:text><xsl:value-of select="$msgName"/>-><xsl:value-of select="$serialName"/>($<xsl:value-of select="$varName"/>);
	</xsl:otherwise>
	</xsl:choose>
</xsl:template>


	<!-- ######################################### -->
	<!-- #####  Make a php parameter list   ###### -->
	<!-- ######################################### -->
<xsl:template name="makePhpArgList">
<xsl:for-each select="param">$<xsl:value-of select="@name"/>, </xsl:for-each>
</xsl:template>

<xsl:template name="makePhpArgListNoFollow">
<xsl:for-each select="param">$<xsl:value-of select="@name"/><xsl:if test="position() != last()">, </xsl:if></xsl:for-each>
</xsl:template>

	<!-- ################################################### -->
	<!-- #####  Generate a cpp non template smart ptr ###### -->
	<!-- ################################################### -->

<xsl:template name="makePersistentPtrHeader">
	<xsl:param name="className"/>

	<xsl:variable name="ptrName" select="concat($className, 'Ptr')"/>


	class <xsl:value-of select="$ptrName"/>
	{
		friend class <xsl:value-of select="$className"/>;

		const	char	*_FileName;
		uint32			_LineNum;

		// linked list of smart ptr
		<xsl:value-of select="$ptrName"/>	*_NextPtr;
		<xsl:value-of select="$ptrName"/>	*_PrevPtr;

		<xsl:value-of select="$className"/>	*_Ptr;

		void linkPtr();

		void unlinkPtr();

	public:
		<xsl:value-of select="$ptrName"/>()
			: _FileName(NULL),
			_LineNum(0),
			_Ptr(NULL),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
		}

		<xsl:value-of select="$ptrName"/>(const <xsl:value-of select="$ptrName"/> &amp;other, const char *filename, uint32 lineNum)
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

		<xsl:value-of select="$ptrName"/>(const <xsl:value-of select="$ptrName"/> &amp;other)
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

		<xsl:value-of select="$ptrName"/>(<xsl:value-of select="$className"/> *objectPtr, const char *filename, uint32 lineNum)
			: _FileName(filename),
			_LineNum(lineNum),
			_NextPtr(NULL),
			_PrevPtr(NULL)
		{
			_Ptr = objectPtr;

			linkPtr();
		}

		<xsl:value-of select="$ptrName"/> &amp;assign(const <xsl:value-of select="$ptrName"/> &amp;other, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = other._Ptr;
			linkPtr();

			return *this;
		}

		~<xsl:value-of select="$ptrName"/>()
		{
			unlinkPtr();
		}

		<xsl:value-of select="$ptrName"/> &amp;assign(<xsl:value-of select="$className"/> *objectPtr, const char *filename, uint32 lineNum)
		{
			_FileName = filename;
			_LineNum = lineNum;

			unlinkPtr();
			_Ptr = objectPtr;
			linkPtr();

			return *this;
		}

		<xsl:value-of select="$ptrName"/> &amp;operator =(const <xsl:value-of select="$ptrName"/> &amp;other)
		{
			return assign(other, __FILE__, __LINE__);
		}

		<xsl:value-of select="$className"/> *operator ->()
		{
			return _Ptr;
		}
		const <xsl:value-of select="$className"/> *operator ->() const
		{
			return _Ptr;
		}

		bool operator == (const <xsl:value-of select="$ptrName"/> &amp;other) const
		{
			return _Ptr == other._Ptr;
		}
		bool operator != (const <xsl:value-of select="$ptrName"/> &amp;other) const
		{
			return !operator ==(other);
		}

		bool operator == (const <xsl:value-of select="$className"/> *object) const
		{
			return _Ptr == object;
		}
		bool operator != (const <xsl:value-of select="$className"/> *object) const
		{
			return !operator ==(object);
		}

		/// Less then comparator : comparison on pointer object address
		bool operator &lt; (const <xsl:value-of select="$ptrName"/> &amp;other) const
		{
			return _Ptr &lt; other._Ptr;
		}

		/// Used to walk thrue the linked list of pointer
		<xsl:value-of select="$ptrName"/> *getNextPtr()
		{
			return _NextPtr;
		}
	};

</xsl:template>

<xsl:template name="makePersistentPtrCpp">
	<xsl:param name="className"/>
	<xsl:variable name="ptrName" select="concat($className, 'Ptr')"/>
	void <xsl:value-of select="$ptrName"/>::linkPtr()
	{
		nlassert(_NextPtr == NULL);
		nlassert(_PrevPtr == NULL);
		if (_Ptr != NULL)
		{
			_NextPtr = _Ptr->getFirstPtr();
			if (_NextPtr != NULL)
			{
				_PrevPtr = _NextPtr->_PrevPtr;
				_PrevPtr->_NextPtr = this;
				_NextPtr->_PrevPtr = this;
			}
			else
			{
				_NextPtr = this;
				_PrevPtr = this;
				_Ptr->setFirstPtr(this);
			}
		}
	}

	void <xsl:value-of select="$ptrName"/>::unlinkPtr()
	{
		if (_NextPtr == NULL)
		{
			nlassert(_PrevPtr == NULL);
			return;
		}

		if (_Ptr != NULL)
		{
			if (_NextPtr == this)
			{
				nlassert(_PrevPtr == this);
				// last pointer !
				_Ptr->setFirstPtr(NULL);
			}
			else
			{
				if (_Ptr->getFirstPtr() == this)
				{
					// the first ptr is the current one, we need to switch to next one
					_Ptr->setFirstPtr(_NextPtr);
				}
			}

		}
		if (_NextPtr != this)
		{
			nlassert(_PrevPtr != this);

			_NextPtr->_PrevPtr = _PrevPtr;
			_PrevPtr->_NextPtr = _NextPtr;
		}
		_NextPtr = NULL;
		_PrevPtr = NULL;
	}
</xsl:template>
</xsl:stylesheet>

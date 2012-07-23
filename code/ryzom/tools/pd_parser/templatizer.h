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

#ifndef NL_TEMPLATIZER_H
#define NL_TEMPLATIZER_H

#include <nel/misc/types_nl.h>
#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/eval_num_expr.h>

#include <vector>
#include <string>
#include <map>

class ITemplatizerBloc;

const char	EnvSeparator = '/';








/**
 * A Templatizer Env
 */
class CTemplatizerEnv : public NLMISC::CEvalNumExpr
{
public:

	/// Constructor
	CTemplatizerEnv(CTemplatizerEnv* parent) : Parent(parent), CurrentArrayNode(0)	{ }

	/// Destructor
	virtual ~CTemplatizerEnv();

	/// Clear Env
	virtual void		clear();

	/// Get value
	virtual std::string	get(const std::string& name);

	/// Set value
	template<typename T>
	void		set(const std::string& name, const T& value)
	{
		std::string::size_type dotpos = name.find(EnvSeparator);
		std::string		child = name.substr(0, dotpos);

		if (dotpos == std::string::npos)
		{
			setAsRawText(name, NLMISC::toString(value));
		}
		else
		{
			getEnv(child)->set(name.substr(dotpos+1), value);
		}
	}

	/// Set a Define
	void				define(const std::string& name)
	{
		set(name, 1);
	}

	/// Set a Conditional Define
	void				define(bool isdef, const std::string& name)
	{
		if (isdef)
			set(name, 1);
	}

	/// Does Variable exist?
	virtual bool		exists(const std::string& name) const
	{
		TValueMap::const_iterator	it = Values.find(name);
		if (it == Values.end())
			return (Parent == NULL ? false : Parent->exists(name));
		return true;
	}

	/// Does Sub Environment exist?
	virtual bool		envExists(const std::string& name) const
	{
		std::string::size_type dotpos = name.find(EnvSeparator);
		std::string		child = name.substr(0, dotpos);

		if (child.empty())
			return true;

		TEnvMap::const_iterator	it = Envs.find(child);
		if (it == Envs.end())
			return false;

		return (dotpos == std::string::npos) ? true : (*it).second->envExists(name.substr(dotpos+1));
	}

	/// Enter Sub Env, like getEnv() but it doesn't look in parent, and always goes in current env
	virtual CTemplatizerEnv*	getSubEnv(const std::string& name)
	{
		std::string::size_type dotpos = name.find(EnvSeparator);
		std::string		child = name.substr(0, dotpos);

		if (child.empty())
			return this;

		CTemplatizerEnv*	env = NULL;

		if (child == ".")
		{
			env = this;
		}
		else if (child == "..")
		{
			env = (Parent != NULL ? Parent : this);
		}
		else if (child == "...")
		{
			env = getRootEnv();
		}
		else
		{
			TEnvMap::iterator	it = Envs.find(child);
			if (it != Envs.end())
			{
				env = (*it).second;
			}
			else
			{
				env = new CTemplatizerEnv(this);
				Envs[child] = env;
			}

		}

		return (dotpos == std::string::npos) ? env : env->getSubEnv(name.substr(dotpos+1));
	}

	/// Get Sub Env
	virtual CTemplatizerEnv*	getEnv(const std::string& name)
	{
		std::string::size_type dotpos = name.find(EnvSeparator);
		std::string		child = name.substr(0, dotpos);

		if (child.empty())
			return this;

		if (child == ".")
		{
			return (dotpos == std::string::npos) ? this : this->getSubEnv(name.substr(dotpos+1));
		}
		else if (child == "..")
		{
			CTemplatizerEnv*	env = (Parent != NULL ? Parent : this);
			return (dotpos == std::string::npos) ? env : env->getSubEnv(name.substr(dotpos+1));
		}
		else if (child == "...")
		{
			CTemplatizerEnv*	env = getRootEnv();
			return (dotpos == std::string::npos) ? env : env->getSubEnv(name.substr(dotpos+1));
		}
		else
		{
			TEnvMap::iterator	it = Envs.find(child);
			if (it != Envs.end())
			{
				return (dotpos == std::string::npos) ? (*it).second : (*it).second->getSubEnv(name.substr(dotpos+1));
			}
			else
			{
				return Parent != NULL ? Parent->getEnv(name) : getSubEnv(name);
			}
		}
	}

	/// Get Sub Env
	CTemplatizerEnv*	getEnv(uint node)
	{
		return getEnv(NLMISC::toString("%08X", node));
	}

	/// Evaluate string (string replacement)
	virtual std::string		eval(const std::string& text);

	/// Get Next Array Node
	CTemplatizerEnv*	nextArrayNode(const std::string& array)
	{
		CTemplatizerEnv*	aenv = getSubEnv(array);
		uint				node = (aenv->CurrentArrayNode)++;
		return aenv->getSubEnv(NLMISC::toString("%08X", node));
	}

	/// Set Sub Env
	virtual void		setSubEnv(const std::string& name, CTemplatizerEnv* subenv)
	{
		Envs[name] = subenv;
	}

	/// Get Parent Env
	virtual CTemplatizerEnv*	getParent()
	{
		return Parent;
	}

public:

	/// Parent Env
	CTemplatizerEnv*	Parent;

	typedef std::map<std::string, ITemplatizerBloc*>	TValueMap;
	typedef std::map<std::string, CTemplatizerEnv*>		TEnvMap;

	/// Contained Values
	TValueMap			Values;

	/// Sub Env
	TEnvMap				Envs;

	/// Get Root Env
	virtual CTemplatizerEnv*	getRootEnv()
	{
		CTemplatizerEnv*	root = this;
		while (root->getParent() != NULL)
			root = root->getParent();

		return root;
	}

	/// Current Array Node
	uint				CurrentArrayNode;

	/// Set As Raw Text
	virtual void		setAsRawText(const std::string& name, const std::string& text);

	/// Set Value Node
	virtual void		setValueNode(const std::string& name, ITemplatizerBloc* bloc)
	{
		Values[name] = bloc;
	}

	/// Get Value Node
	virtual ITemplatizerBloc*	getValueNode(const std::string& name)
	{
		ITemplatizerBloc*	node = NULL;
		CTemplatizerEnv*	env = NULL;
		return getValueNodeAndEnv(name, node, env) ? node : NULL;
	}

	/// Get Value Node
	virtual bool		getValueNodeAndEnv(const std::string& name, ITemplatizerBloc*& node, CTemplatizerEnv*& env)
	{
		std::string::size_type pos = name.find_last_of(EnvSeparator);
		if (pos == std::string::npos)
		{
			node = getNode(name);
			env = this;
			while (node == NULL && env != NULL)
			{
				env = env->getParent();
				if (env != NULL)
					node = env->getNode(name);
			}
		}
		else
		{
			env = getEnv(name.substr(0, pos));
			if (env != NULL)
				node = env->getNode(name.substr(pos+1));
		}

		return node != NULL && env != NULL;
	}

	virtual ITemplatizerBloc*	getNode(const std::string& name)
	{
		TValueMap::iterator	it = Values.find(name);
		return it == Values.end() ? NULL : (*it).second;
	}

	virtual NLMISC::CEvalNumExpr::TReturnState	evalValue (const char *value, double &result, uint32 userData);
};


/**
 * A Templatizer Env
 */
class CTemplatizerRefEnv : public CTemplatizerEnv
{
public:

	/// Constructor
	CTemplatizerRefEnv(CTemplatizerEnv* ref) : CTemplatizerEnv(NULL), Reference(ref)	{ }

	/// Clear Env
	virtual void		clear()
	{
		Reference = NULL;
	}

	/// Get value
	virtual std::string	get(const std::string& name)
	{
		return Reference->get(name);
	}

	/// Does Variable exist?
	virtual bool		exists(const std::string& name) const
	{
		return Reference->exists(name);
	}

	/// Does Sub Environment exist?
	virtual bool		envExists(const std::string& name) const
	{
		return Reference->envExists(name);
	}

	/// Get Sub Env
	virtual CTemplatizerEnv*	getEnv(const std::string& name)
	{
		return Reference->getEnv(name);
	}

	/// Evaluate string (string replacement)
	virtual std::string			eval(const std::string& text)
	{
		return Reference->eval(text);
	}

	/// Enter Sub Env, like getEnv() but it doesn't look in parent, and always goes in current env
	virtual CTemplatizerEnv*	getSubEnv(const std::string& name)
	{
		return Reference->getSubEnv(name);
	}

	/// Get Parent Env
	virtual CTemplatizerEnv*	getParent()
	{
		return Reference->getParent();
	}
public:

	CTemplatizerEnv*	Reference;

	/// Get Root Env
	virtual CTemplatizerEnv*	getRootEnv()
	{
		return Reference->getRootEnv();
	}

	/// Set As Raw Text
	virtual void		setAsRawText(const std::string& name, const std::string& text)
	{
		Reference->setAsRawText(name, text);
	}

	/// Set Value Node
	virtual void		setValueNode(const std::string& name, ITemplatizerBloc* bloc)
	{
		Reference->setValueNode(name, bloc);
	}

	/// Get Value Node
	virtual ITemplatizerBloc*	getValueNode(const std::string& name)
	{
		return Reference->getValueNode(name);
	}

	/// Get Value Node
	virtual bool	getValueNodeAndEnv(const std::string& name, ITemplatizerBloc*& node, CTemplatizerEnv*& env)
	{
		return Reference->getValueNodeAndEnv(name, node, env);
	}

	virtual ITemplatizerBloc*	getNode(const std::string& name)
	{
		return Reference->getNode(name);
	}
};


/**
 * <Class description>
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2003
 */
class CTemplatizer
{
public:

	/// Constructor
	CTemplatizer();

	/// Destructor
	~CTemplatizer();



	/**
	 * Build templatizer from text
	 */
	bool		build(const char* text);


	/**
	 * Evaluate template and render to string
	 */
	std::string	eval();


	/**
	 * Set Value in env
	 */
	template<typename T>
	void		set(const std::string& var, const T& value)
	{
		if (RootEnv == NULL)
			return;

		std::string::size_type pos = var.find_last_of(EnvSeparator);

		if (pos == std::string::npos)
		{
			RootEnv->set(var, value);
		}
		else
		{
			RootEnv->getEnv(var.substr(0, pos))->set(var.substr(pos+1), value);
		}
	}

public:

	ITemplatizerBloc*		RootBloc;

	CTemplatizerEnv*		RootEnv;

};



class CTemplatizerParser
{
public:

	CTemplatizerParser() : _Buffer(NULL), _Line(0), _Valid(false)	{ }
	CTemplatizerParser(const CTemplatizerParser& ptr) : _Buffer(ptr._Buffer), _Line(ptr._Line), _Valid(ptr._Valid)	{ }
	CTemplatizerParser(const char*	buffer, uint linestart = 1) : _Buffer(buffer), _Line(linestart), _Valid(_Buffer != NULL)	{ }

	char	operator * () const			{ return *_Buffer; }
	char	operator [] (int i) const	{ return _Buffer[i]; }

	CTemplatizerParser&	operator = (const CTemplatizerParser& ptr)
	{
		_Buffer = ptr._Buffer;
		_Line = ptr._Line;
		_Valid = ptr._Valid;
		return *this;
	}

	CTemplatizerParser&	operator ++ ()
	{
		if (*_Buffer == '\0')
			return *this;

		if (*_Buffer == '\n')
			++_Line;

		++_Buffer;
		return *this;
	}

	CTemplatizerParser	operator ++ (int)
	{
		CTemplatizerParser	ret(*this);
		++(*this);
		return ret;
	}

	void	invalidate()	{ _Valid = false; }
	bool	isValid() const	{ return _Valid; }

	uint	getLine() const	{ return _Line; }

private:

	const char*	_Buffer;

	uint	_Line;

	bool	_Valid;

};




/**
 * A Templatizer node
 */
class ITemplatizerBloc
{
public:

	/// Constructor
	ITemplatizerBloc();

	/// Destructor
	virtual ~ITemplatizerBloc();



	/// Evaluate node
	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	res;
		uint	i;
		for (i=0; i<Blocs.size(); ++i)
			res += Blocs[i]->eval(env);
		return res;
	}

	/// Get Text (assuming this is a raw text bloc)
	virtual std::string	getText(CTemplatizerEnv* env)
	{
		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		return NULL;
	}

	/// Get Actual Bloc (not a reference)
	virtual ITemplatizerBloc*	getActualBloc()
	{
		return this;
	}

public:

	std::string	evalParam(const std::string& param, CTemplatizerEnv* env)
	{
		TParamMap::iterator	it = Params.find(param);
		if (it == Params.end())
			return "";
		return (*it).second->eval(env);
	}

public:

	/// Bloc types
	enum TType
	{
		Text,
	};

	/// Bloc type
	TType				Type;

	typedef std::vector<ITemplatizerBloc*>				TBlocList;
	typedef std::map<std::string, ITemplatizerBloc*>	TParamMap;

	/// Params
	TParamMap			Params;

	/// Sub blocs
	TBlocList			Blocs;


	/// Parse bloc
	static ITemplatizerBloc*	parseBloc(CTemplatizerParser& ptr);

	/// Parse bloc header
	virtual CTemplatizerParser	parseHeader(CTemplatizerParser ptr);

	/// Parse bloc internal data
	virtual CTemplatizerParser	parseInternal(CTemplatizerParser ptr);

	/// Has A Internal Bloc of data
	virtual bool		hasInternal() const	{ return true; }
};





/// Root Templatizer
class CTemplatizerRootBloc : public ITemplatizerBloc
{
public:

};

/// Reference Node
class CTemplatizerReferenceBloc : public ITemplatizerBloc
{
public:

	ITemplatizerBloc*	Reference;

	/// Constructor
	CTemplatizerReferenceBloc(ITemplatizerBloc* ref = NULL) : Reference(ref)	{}

	/// Destructor
	virtual ~CTemplatizerReferenceBloc()
	{
		Reference = NULL;
	}

	/// Evaluate node
	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			name = evalParam("name", env);
		std::string			ref = evalParam("ref", env);
		ITemplatizerBloc*	refnode = env->getValueNode(ref);
		if (refnode != NULL)
			env->setValueNode(name, new CTemplatizerReferenceBloc(refnode));
		else
			nlwarning("Failed to create reference on '%s', not found", name.c_str());
		return "";
	}

	/// Get Text (assuming this is a raw text bloc)
	virtual std::string	getText(CTemplatizerEnv* env)
	{
		return Reference->getText(env);
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name", "ref" };
		return (const char**)args;
	}

	/// Get Actual Bloc (not a reference)
	virtual ITemplatizerBloc*	getActualBloc()
	{
		return Reference->getActualBloc();
	}

	/// Has A Internal Bloc of data
	virtual bool		hasInternal() const	{ return false; }
};


/// RefEnv Bloc
class CTemplatizerRefEnvBloc : public ITemplatizerBloc
{
public:

	/// Evaluate node
	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			name = evalParam("name", env);
		std::string			ref = evalParam("ref", env);

		CTemplatizerEnv*	refenv = env->getEnv(ref);
		if (refenv != NULL)
			env->setSubEnv(name, new CTemplatizerRefEnv(refenv));
		else
			nlwarning("Failed to create reference on env '%s', not found", name.c_str());
		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name", "ref" };
		return (const char**)args;
	}

	/// Has A Internal Bloc of data
	virtual bool		hasInternal() const	{ return false; }
};





/**
 * Comment Bloc
 */
class CTemplatizerCommentBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		return "";
	}

	/// Parse bloc internal data
	virtual CTemplatizerParser	parseInternal(CTemplatizerParser ptr);

};


/**
 * Raw Text Bloc
 */
class CTemplatizerRawTextBloc : public ITemplatizerBloc
{
public:

	std::string		Text;

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		return Text;
	}

	virtual std::string	getText(CTemplatizerEnv* env)
	{
		return Text;
	}
};

// Set As Raw Text
inline void	CTemplatizerEnv::setAsRawText(const std::string& name, const std::string& text)
{
	CTemplatizerRawTextBloc*	bloc = new CTemplatizerRawTextBloc();
	bloc->Text = text;
	setValueNode(name, bloc);
}

// Get value
inline std::string	CTemplatizerEnv::get(const std::string& name)
{
	ITemplatizerBloc*	bloc = getValueNode(name);

	return (bloc == NULL) ? std::string("") : bloc->getText(this);
}

// eval num expr override
inline NLMISC::CEvalNumExpr::TReturnState	CTemplatizerEnv::evalValue (const char *value, double &result, uint32 userData)
{
	if (*value != '$')
	{
		return NLMISC::CEvalNumExpr::ValueError;
	}

	std::string	strvalue = get(value+1);

	if (sscanf(strvalue.c_str(), "%lf", &result) != 1)
	{
		result = (strvalue.empty() ? 0.0 : 1.0);
	}

	return NLMISC::CEvalNumExpr::NoError;
}



/**
 * Text Bloc
 */
class CTemplatizerTextBloc : public ITemplatizerBloc
{
public:

	std::string		Text;

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		return env->eval(Text);
	}

	virtual std::string	getText(CTemplatizerEnv* env)
	{
		return env->eval(Text);
	}

	/// Parse bloc internal data
	virtual CTemplatizerParser	parseInternal(CTemplatizerParser ptr);
};




/**
 * Sub bloc
 */
class CTemplatizerSubBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			subname = evalParam("name", env);
		CTemplatizerEnv*	subenv = env->getSubEnv(subname);

		return ITemplatizerBloc::eval(subenv);
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


/**
 * Loop bloc
 */
class CTemplatizerLoopBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			subname = evalParam("name", env);
		CTemplatizerEnv*	subenv = env->getSubEnv(subname);

		std::string			res;

		CTemplatizerEnv::TEnvMap::iterator	it;
		for (it=subenv->Envs.begin(); it!=subenv->Envs.end(); ++it)
			res += ITemplatizerBloc::eval((*it).second);

		return res;
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


/**
 * IfDefEnv bloc
 */
class CTemplatizerIfDefEnvBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			subname = evalParam("name", env);
		std::string			evalinsub = evalParam("evalinsub", env);

		if (env->envExists(subname))
		{
			sint eval;
			NLMISC::fromString(evalinsub, eval);
			CTemplatizerEnv*	subenv = (eval ? env->getEnv(subname) : env);
			return ITemplatizerBloc::eval(subenv);
		}

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name", "evalinsub" };
		return (const char**)args;
	}
};


/**
 * IfDef bloc
 */
class CTemplatizerIfDefBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			varname = evalParam("name", env);

		if (env->exists(varname))
		{
			return ITemplatizerBloc::eval(env);
		}

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


/**
 * IfDefEnv bloc
 */
class CTemplatizerIfNotDefEnvBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			subname = evalParam("name", env);

		if (!env->envExists(subname))
		{
			return ITemplatizerBloc::eval(env);
		}

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


/**
 * IfDef bloc
 */
class CTemplatizerIfNotDefBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			varname = evalParam("name", env);

		if (!env->exists(varname))
		{
			return ITemplatizerBloc::eval(env);
		}

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


/**
 * Switch bloc
 */
class CTemplatizerSwitchBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string			switchvalue = evalParam("value", env);

		uint	i;
		for (i=0; i<Blocs.size(); ++i)
			if (Blocs[i]->evalParam("case", env) == switchvalue)
				return Blocs[i]->eval(env);

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "value" };
		return (const char**)args;
	}
};

/**
 * File bloc
 */
class CTemplatizerFileBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	clearfile = evalParam("clear", env);
		std::string	filename = evalParam("name", env);

		std::string	result = ITemplatizerBloc::eval(env);

		FILE*	f;
		f = fopen(filename.c_str(), (clearfile == "true" ? "w" : "a"));
		if (f != NULL)
		{
			fwrite(result.c_str(), 1, result.size(), f);
			fclose(f);
		}

		return result;
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};

/**
 * Set bloc
 */
class CTemplatizerSetBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	var = evalParam("name", env);
		std::string	result = ITemplatizerBloc::eval(env);

		env->set(var, result);

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};

/**
 * Append bloc
 */
class CTemplatizerAppendBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	var = evalParam("name", env);
		std::string	result = ITemplatizerBloc::eval(env);

		ITemplatizerBloc*	bloc = env->getValueNode(var);
		if (bloc == NULL)
			return "";

		CTemplatizerRawTextBloc*	text = dynamic_cast<CTemplatizerRawTextBloc*>(bloc->getActualBloc());
		if (text == NULL)
			return "";

		text->Text += result;

		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};

/**
 * Define Bloc
 */
class CTemplatizerDefineBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	name = evalParam("name", env);
		env->setValueNode(name, new CTemplatizerReferenceBloc(this));
		return "";
	}

	virtual std::string	getText(CTemplatizerEnv* env)
	{
		return ITemplatizerBloc::eval(env);
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};

/**
 * If Bloc
 */
class CTemplatizerIfBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	value = evalParam("cond", env);
		double		result;
		NLMISC::CEvalNumExpr::TReturnState	res = env->evalExpression(value.c_str(), result, NULL);

		if (res == NLMISC::CEvalNumExpr::NoError && result != 0.0)
		{
			return ITemplatizerBloc::eval(env);
		}
		else
		{
			return "";
		}
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "cond" };
		return (const char**)args;
	}
};

/**
 * If Not Bloc
 */
class CTemplatizerIfNotBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	value = evalParam("cond", env);

		if (value.empty())
		{
			return ITemplatizerBloc::eval(env);
		}
		else
		{
			return "";
		}
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "cond" };
		return (const char**)args;
	}
};

/**
 * Join bloc
 */
class CTemplatizerJoinBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	sep = evalParam("separator", env);

		std::string	res;
		uint	i;
		for (i=0; i<Blocs.size(); ++i)
		{
			std::string	token = Blocs[i]->eval(env);

			if (token.empty())
				continue;

			if (!res.empty())
				res += sep;

			res += token;
		}
		return res;
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "separator" };
		return (const char**)args;
	}
};

/**
 * User Defined function call
 */
class CTemplatizerUserFunctionBloc : public ITemplatizerBloc
{
public:

	CTemplatizerUserFunctionBloc(const std::string& name) : Name(name)	{}

	std::string		Name;

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		ITemplatizerBloc*	func = NULL;
		CTemplatizerEnv*	fenv = NULL;
		if (!env->getValueNodeAndEnv(Name, func, fenv))
		{
			nlwarning("Unknown user function '%s'", Name.c_str());
			return "";
		}

		// subenv is child of object env, not of current env
		CTemplatizerEnv*	subenv = new CTemplatizerEnv(fenv);

		// deport params in subenv
		// \todo : eval param in current env
		TParamMap::iterator	it;
		for (it=Params.begin(); it!=Params.end(); ++it)
			subenv->setAsRawText((*it).first, (*it).second->getText(env));
			//subenv->setValueNode((*it).first, new CTemplatizerReferenceBloc((*it).second));

		std::string	res = func->getText(subenv);

		delete subenv;

		return res;
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		return NULL;
	}

	/// Has A Internal Bloc of data
	virtual bool		hasInternal() const
	{
		return false;
	}
};

/**
 * Class
 */
class CTemplatizerClassBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	name = evalParam("name", env);
		env->setValueNode(name, new CTemplatizerReferenceBloc(this));
		return "";
	}

	virtual std::string	instantiate(CTemplatizerEnv* env)
	{
		return ITemplatizerBloc::eval(env);
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};

/**
 * Class
 */
class CTemplatizerObjectBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	classname = evalParam("class", env);
		std::string	name = evalParam("name", env);

		ITemplatizerBloc*	bloc = env->getValueNode(classname);
		if (bloc == NULL)
		{
			nlwarning("Unknown class '%s'", classname.c_str());
			return "";
		}

		CTemplatizerClassBloc*	classbloc = dynamic_cast<CTemplatizerClassBloc*>(bloc->getActualBloc());
		if (classbloc == NULL)
		{
			nlwarning("object '%s' is not a class", classname.c_str());
			return "";
		}

		CTemplatizerEnv* objectenv = env->getSubEnv(name);
		objectenv->clear();

		return classbloc->instantiate(objectenv);
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "class", "name" };
		return (const char**)args;
	}

	/// Has A Internal Bloc of data
	virtual bool		hasInternal() const	{ return false;	}
};


/**
 * Breakpoint
 */
class CTemplatizerBreakpointBloc : public ITemplatizerBloc
{
public:

	virtual std::string	eval(CTemplatizerEnv* env)
	{
		std::string	value = evalParam("name", env);
		return "";
	}

	/// Get Param list
	virtual const char**	getDefParamList()
	{
		static const char*	args[] = { "name" };
		return (const char**)args;
	}
};


#endif // NL_TEMPLATIZER_H

/* End of templatizer.h */

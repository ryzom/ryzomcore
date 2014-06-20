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

#ifndef NL_HIERARCHICAL_TIMER_H
#define NL_HIERARCHICAL_TIMER_H

#include <string>
#include <vector>
#include <algorithm>

#include "types_nl.h"
#include "time_nl.h"
#include "debug.h"

#ifndef NL_NO_DEBUG
#	define ALLOW_TIMING_MEASURES
#endif // NL_NO_DEBUG


#ifdef ALLOW_TIMING_MEASURES

// You should need only this macro, bench the local scope
#	define H_AUTO(__name)		static NLMISC::CHTimer	__name##_timer(#__name); NLMISC::CAutoTimer	__name##_auto(&__name##_timer);

// Same as H_AUTO but you don't have to give a name, it uses the function/line
#	define H_AUTO2				static std::string __str_##__LINE__(NLMISC::toString("%s:%d", __FUNCTION__, __LINE__)); static NLMISC::CHTimer __timer_##__LINE__(__str_##__LINE__.c_str()); NLMISC::CAutoTimer __auto_##__LINE__(&__timer_##__LINE__);

// If you want to bench a specific part of the code
#	define H_BEFORE(__name)		static NLMISC::CHTimer	__name##_timer(#__name); __name##_timer.before();
#	define H_AFTER(__name)		__name##_timer.after();

// Display the timer info after each loop call
#	define H_AUTO_INST(__name)	static NLMISC::CHTimer	__name##_timer(#__name); NLMISC::CAutoTimerInst	__name##_auto(&__name##_timer);

// H_AUTO split in 2. The declaration of the static timer, and a CAutoTimer instance.
// Useful to group same timer bench in different functions for example
#	define H_AUTO_DECL(__name)	static NLMISC::CHTimer	__name##_timer(#__name);
#	define H_AUTO_USE(__name)	NLMISC::CAutoTimer	__name##_auto(&__name##_timer);

//
#	define H_TIME(__name,__inst) \
	{ \
	static NLMISC::CHTimer	nl_h_timer(#__name); \
	nl_h_timer.before(); \
	__inst \
	nl_h_timer.after(); \
	}

#else
// void macros
#	define H_TIME(__name,__inst) __inst
#	define H_BEFORE(__name)
#	define H_AFTER(__name)
#	define H_AUTO(__name)
#	define H_AUTO2
#	define H_AUTO_INST(__name)
#	define H_AUTO_DECL(__name)
#	define H_AUTO_USE(__name)
#endif

namespace NLMISC
{

#ifdef NL_COMP_VC
// Visual C++ warning : ebp maybe modified
#	pragma warning(disable:4731)
#endif


/**  A simple clock to measure ticks.
  *  \warning On Intel platform, processor cycles are counted, on other platforms, CTime::getPerformanceTime is used instead.
  *
  * \sa CStopWatch
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSimpleClock
{
public:
	CSimpleClock() : _NumTicks(0)
	{
#ifdef NL_DEBUG
			_Started = false;
#endif
	}
	// start measure
	void start()
	{
#ifdef NL_DEBUG
			nlassert(!_Started);
			_Started = true;
#endif
#ifdef NL_CPU_INTEL
		_StartTick = rdtsc();
#else
		_StartTick = CTime::getPerformanceTime();
#endif
	}
	// end measure
	void stop()
	{
#ifdef  NL_DEBUG
			nlassert(_Started);
			_Started = false;
#endif
#ifdef NL_CPU_INTEL
		_NumTicks = rdtsc() - _StartTick;
#else
		_NumTicks = CTime::getPerformanceTime() - _StartTick;
#endif
	}
	// get measure
	uint64	getNumTicks() const
	{
#ifdef NL_DEBUG
		nlassert(!_Started);
#endif
		nlassert(_NumTicks != 0);
		return _NumTicks;
	}
	// This compute the duration of start and stop (in cycles).
	static void init();
	/** Get the number of ticks needed to perform start().
	  * Should have called init() before calling this.
	  */
	static uint64 getStartStopNumTicks()
	{
		return _StartStopNumTicks;
	}
private:
	uint64  _StartTick;
	uint64	_NumTicks;
#ifdef  NL_DEBUG
	bool	_Started;
#endif
	static bool		_InitDone;
	static uint64	_StartStopNumTicks;
};


/**
 * Hierarchical timing system. Allows to accurately measure performance of routines, and displays results hierarchically.
 * To time a piece of code, just declare a static CHTimer object and encapsulate code between calls to before() and after() methods.
 * ex:
 *\code
 void myFunction()
 {
	static CHTimer	myTimer("myFunction");
	myTimer.before();
	// some code here
	myTimer.after();
 }
 *\endcode
 * Don't forget to call after() to avoid timing wrongness or assertion crashes !
 *
 * \warning Supports only single-threaded applications.
 * \warning Supports only Intel processors.
 *
 * \author Benjamin Legros
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001, 2002
 */
class CHTimer
{
public:
	// this enum is used to sort displayed results
	enum TSortCriterion { NoSort,
						  TotalTime,
						  TotalTimeWithoutSons,
						  MeanTime,
						  NumVisits,
						  MaxTime,
						  MinTime,
						  MaxSession,
						  SortCriterionsLast
						};
public:
	/// ctor
	CHTimer() : _Name(NULL), _Parent(NULL), _IsRoot(false) {}
	CHTimer(const char *name, bool isRoot = false) : _Name(name), _Parent(NULL), _IsRoot(isRoot) {}
	/// Starts a measuring session
	void			before()
	{
		if (_Benching)
			doBefore();
	}
	// Ends a measuring session
	void			after()
	{
		if (_Benching)
			doAfter(false);
	}
	void			after(bool displayAfter)
	{
		if (_Benching)
			doAfter(displayAfter);
	}
	// Get this node name
	const char		*getName() const { return _Name; }
	void			setName(const char *name) { _Name = name; }
	/** Starts a bench session
	  * \param wantStandardDeviation When true, benchmarks will report the standard deviation of values. This require more memory, however, because each samples must be kept.
	  * \param quick if true, quick compute the frequency of the processor
	  */
	static void		startBench(bool wantStandardDeviation = false, bool quick = false, bool reset = true);
	/** For backward compatibility
	  */
	static void		bench() { startBench(); }
	/** For backward compatibility
	  */
	static void		adjust() {}
	/// Ends a bench session
	static void		endBench();

	static bool		benching () { return _Benching; }

	/** Display results
	  * \param displayEx true to display more detailed infos
	  */
	static void		display(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayInline = true, bool displayEx = true);
	/** Display results by execution paths
	  * \param displayInline true to display each result on a single line.
	  * \param alignPaths    true to display all execution paths aligned.
	  * \param displayEx	 true to display more detailed infos.
	  */
	static void		displayByExecutionPath(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayInline = true, bool alignPaths = true, bool displayEx = true);

	/** Hierarchical display, no sorting is done
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar
	  */
	static void		displayHierarchical(CLog *log= InfoLog, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/** Hierarchical display, no sorting is done
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar
	  */
	static void		displayHierarchicalByExecutionPath(CLog *log= InfoLog, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/** Hierarchical display, sorting is done in branches
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar
	  */
	static void		displayHierarchicalByExecutionPathSorted(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/** Hierarchical display, sorting is done in branches
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar
	  */
	static void		displaySummary(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2, uint maxDepth = 3);

	/// Clears stats, and re initializes all timer structure
	static void		clear();

	/// Clears SessionMax current stats (only current value)
	static void		clearSessionCurrent();

	/// Clears all SessionMax stats (max and current values)
	static void		clearSessionStats();

	/// Update session stats
	static void		updateSessionStats();

//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	struct CNode;
	typedef std::vector<CNode *>   TNodeVect;
	typedef std::vector<CHTimer *> TTimerVect;
	//
	/// a node in an execution path
	struct CNode
	{
		typedef std::vector<double> TTimeVect;
		//
		CNode					*Parent;
		TNodeVect				Sons;
		CHTimer					*Owner;	   // the hierarchical timer this node is associated with
		uint64					TotalTime; // the total time spent in that node, including sons
		uint64					LastSonsTotalTime;
		uint64					SonsTotalTime; // maybe different from LastSonsTotalTime while benching the sons and if the display is called in a benched node
		TTimeVect				Measures;  // All time measures. Used only when standard deviation is wanted
		uint64					MinTime;   // the minimum time spent in that node
		uint64					MaxTime;   // the maximum time spent in that node
		uint64					NumVisits; // the number of time the execution has gone through this node
		// session max measure
		uint64					SessionCurrent;
		uint64					SessionMax;
		//
		uint64					SonsPreambule; // preamble time for the sons
		CSimpleClock			Clock;         // a clock to do the measures at this node
		// ctor
	  CNode(CHTimer	*owner = NULL, CNode	*parent = NULL) : Parent(parent), Owner(owner)
		{
			reset();
		}
		// dtor
		~CNode();
		// Get the number of nodes in the tree starting at this node
		uint  getNumNodes() const;
		// release the sons, should not be benching when calling this
		void	releaseSons();
		// reset this node measures
		void	reset()
		{
			SonsTotalTime		 = 0;
			TotalTime			 = 0;
			MaxTime				 = 0;
			MinTime				 = (uint64) -1;
			NumVisits			 = 0;
			SonsPreambule	     = 0;
			LastSonsTotalTime    = 0;
			SessionCurrent       = 0;
			SessionMax           = 0;
			NLMISC::contReset(Measures);
		}
		//
		// Display this node path
		void	displayPath(CLog *log) const;
		// Get this node path
		void    getPath(std::string &dest) const;

		// reset session current
		void	resetSessionCurrent()
		{
			SessionCurrent = 0;
			for (uint i=0; i<Sons.size(); ++i)
				Sons[i]->resetSessionCurrent();
		}
		// reset all session stats
		void	resetSessionStats()
		{
			SessionCurrent = 0;
			SessionMax = 0;
			for (uint i=0; i<Sons.size(); ++i)
				Sons[i]->resetSessionStats();
		}
		// spread session value through the while node tree
		void	spreadSession()
		{
			SessionMax = SessionCurrent;
			for (uint i=0; i<Sons.size(); ++i)
				Sons[i]->spreadSession();
		}
	};

	/** Some statistics
	  * They can be build from a set of nodes
	  */
	struct CStats
	{
		double  TimeStandardDeviation;
		double	TotalTime;
		double	TotalTimeWithoutSons;
		double	MeanTime;
		uint64	NumVisits;
		double	MinTime;
		double	MaxTime;
		double	SessionMaxTime;

		// build stats from a single node
		void buildFromNode(CNode *node, double msPerTick);

		// build stats from a vector of nodes
		void buildFromNodes(CNode **firstNode, uint numNodes, double msPerTick);

		// display stats
		void display(CLog *log, bool displayEx = false, bool wantStandardDeviation = false);

		/** Get a string for stats (all stats on the same line)
		  * \param statEx display extended stats
		  */
		void getStats(std::string &dest, bool statEx, double rootTotalTime, bool wantStandardDeviation = false);
	};
	// Stats and the associated timer
	struct CTimerStat : public CStats
	{
		CHTimer *Timer;
	};
	// Stats and the associated node
	struct CNodeStat : public CStats
	{
		CNode *Node;
	};

	/** A statistics sorter, based on some criterion.
	  * It works on pointers on CStats objects
	  */
	struct CStatSorter
	{
		CStatSorter(TSortCriterion criterion = TotalTime) : Criterion(criterion)
		{}
		TSortCriterion Criterion;
		// Less operator
		bool operator()(const CStats *lhs, const CStats *rhs);
	};


	/** For Hierarchical + sorted display. displayHierarchicalByExecutionPath()
	 *
	 */
	struct	CExamStackEntry
	{
		// The node.
		CNode				*Node;
		// The current child to process.
		uint				CurrentChild;
		// The childes, sorted by specific criterion.
		std::vector<CNode*>	Children;
		// The depth of the entry
		uint				Depth;

		explicit	CExamStackEntry(CNode *node)
		{
			Node= node;
			CurrentChild= 0;
			Depth = 0;
		}

		explicit	CExamStackEntry(CNode *node, uint depth)
		{
			Node= node;
			CurrentChild= 0;
			Depth = depth;
		}
	};

	// Real Job.
	void			doBefore();
	void			doAfter(bool displayAfter = false);

	static void		estimateAfterStopTime();

private:
	// walk the tree to current execution node, creating it if necessary
	void			walkTreeToCurrent();
private:
	// node name
	const  char						*_Name;
	// the parent timer
	CHTimer							*_Parent;
	// the sons timers
	TTimerVect						_Sons;
	// Tells if this is a root node
	bool							_IsRoot;
private:
	// root node of the hierarchy
	static CNode					_RootNode;
	// the current node of the execution
	static CNode					*_CurrNode;
	// the root timer
	static CHTimer					 _RootTimer;
	/** This clock is used to measure the preamble of methods such as CHTimer::before()
	  * This is static, but the Hierarchical Timer doesn't support multi threading anyway..
      */
	static CSimpleClock				_PreambuleClock;
	//
	static double					_MsPerTick;
	//
	static bool						_Benching;
	//
	static bool						_BenchStartedOnce;
	//
	static bool						_WantStandardDeviation;
	//
	static CHTimer				   *_CurrTimer;
	//
	static sint64					_AfterStopEstimateTime;
	static bool						_AfterStopEstimateTimeDone;
};

/**
 * An automatic measuring timer. Encapsulates calls to CHTimer, and avoids missuses of before() and after().
 * ex:
 *\code
 void myFunction()
 {
	static CHTimer	myTimer("myFunction");
	CAutoTimer		myAuto(myTimer);
	// some code here
 }
 *\endcode
 * Don't forget to call after() to avoid timing wrongness or assertion crashes !
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CAutoTimer
{
private:
	CHTimer *_HTimer;
public:
	CAutoTimer(CHTimer *timer) : _HTimer(timer) { _HTimer->before(); }
	~CAutoTimer() { _HTimer->after(); }
};


/**
 *	Same but display result at end.
 */
class CAutoTimerInst
{
private:
	CHTimer *_HTimer;
public:
	CAutoTimerInst(CHTimer *timer) : _HTimer(timer) { _HTimer->before(); }
	~CAutoTimerInst() { _HTimer->after(true); }
};


} // NLMISC

#endif // NL_HIERARCHICAL_TIMER_H

/* End of hierarchical_timer.h */

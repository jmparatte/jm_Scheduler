/*
	jm_Scheduler
	============

	jm_Scheduler.h and jm_Scheduler.cpp - Implementation of a general
	scheduler named "jm_Scheduler" to use in various environment
	like Arduino, Energia, MDEB, etc...

	Copyright (c) 2017,2016,2015 Jean-Marc Paratte

	jm_Scheduler is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    jm_Scheduler is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with jm_Scheduler.  If not, see <http://www.gnu.org/licenses/>.

    Last revised: 2017-04-26,2016-07-07,2016-04-27,2015-06-29
*/

#ifndef jm_Scheduler_h
#define jm_Scheduler_h

//------------------------------------------------------------------------------

#include <Arduino.h>

//----------------------------------------------------------------------

#ifndef voidfuncptr_t
#define voidfuncptr_t voidfuncptr_t
typedef void (*voidfuncptr_t)(void); // void function pointer typedef
#endif

#ifndef vfp_vpu32b_t
#define vfp_vpu32b_t vfp_vpu32b_t
typedef void (*vfp_vpu32b_t)(void*, uint32_t, bool);
#endif

//----------------------------------------------------------------------

#ifndef timestamp_t
#define timestamp_t timestamp_t
typedef uint32_t timestamp_t; // mbed/ticker_api.h(21): typedef uint32_t timestamp_t;
#endif

#ifndef timestamp_read
#define timestamp_read() timestamp_read()
inline timestamp_t timestamp_read() { return (timestamp_t)micros(); } // mbed/us_ticker_api.h(54): uint32_t us_ticker_read(void);
#endif

//------------------------------------------------------------------------------

#ifdef assert
#else
#	ifdef NDEBUG
#		define assert(e) ((void)0)
#	else
#		define assert(e) ((e) ? (void)0 : abort())
#	endif
#endif

//------------------------------------------------------------------------------

//#define TIMESTAMP_TMAX ((timestamp_t)(1UL<<(sizeof(timestamp_t)*8-1))-1) // 2147483647=0x7FFFFFFF
// 4'294'967'296 = 1'0000'0000 = [2^32us]
// 3'600'000'000 =   D693'A400 = [1h]
//   694'967'296 =   296C'5C00 = [2^32us - 1h = 694.967296s = 11m + 34.967296s]
// 3'947'483'648 =   EB49'D200
//   268'435'456 =   1000'0000 = [268.435456s = 4m + 28.435456s]
//    65'536'000 =    3E8'0000 = [65.536000s = 1m + 5.536000s]
//     1'000'000 =      F'4240 = [1s]
//    30'000'000 =    1C9'C380 = [30s]
// 4'264'951'808 =   FE36'0000 = 1'0000'0000 - 1CA'0000 = [1h + 11m + 4s + 951ms + 808us]
//    30'015'488 =    1CA'0000 = [30s + 15ms + 488us]

#define TIMESTAMP_DEAD (0x01CA0000) // dead time [30s + 15ms + 488us]
#define TIMESTAMP_TMAX (0xFE35FFFF) // [1h + 11m + 4s + 951ms + 808us - 1]

#define TIMESTAMP_1US	(1UL)					// [1us]
#define TIMESTAMP_1MS	(1000*TIMESTAMP_1US)	// [1ms]
#define TIMESTAMP_1SEC	(1000*TIMESTAMP_1MS)	// [1s]
#define TIMESTAMP_1MIN	(60*TIMESTAMP_1SEC)		// [1 minute]
#define TIMESTAMP_1HOUR	(60*TIMESTAMP_1MIN)		// [1 hour]

//------------------------------------------------------------------------------

#define jm_Scheduler_time_read() timestamp_read()
#define jm_Scheduler_tref_read() (jm_Scheduler::tref)
#define jm_Scheduler_tref_ival(ival) (jm_Scheduler::tref + ival)

#define jm_Scheduler_time_ge_time(tref, time) ((timestamp_t)(tref - time) < TIMESTAMP_DEAD)
#define jm_Scheduler_tref_ge_time(time) ((timestamp_t)(jm_Scheduler::tref - time) < TIMESTAMP_DEAD)
#define jm_Scheduler_tref_ival_ge_time(ival, time) ((timestamp_t)(jm_Scheduler::tref + ival - time) < TIMESTAMP_DEAD)

class jm_Scheduler
{
public:

	static timestamp_t tref;			// current scheduler time
	static jm_Scheduler *first;			// first scheduled routine chain
	static jm_Scheduler *crnt;			// current running routine

	static jm_Scheduler *wakeup_first;	// first wakeup routine chain

public:

	voidfuncptr_t func;					// address of routine function
	timestamp_t time;					// time of scheduled execution
	timestamp_t ival;					// interval of cyclic execution

	jm_Scheduler *next;					// next in routine chain

	timestamp_t wakeup_time;			// time of first wakeup (may be repeated)
	jm_Scheduler *wakeup_next;			// next routine in wakeup routine chain
	int wakeup_count;					// count of repeated wakeup

	void chain_insert();
	void chain_remove();

	void wakeup_chain_append();
	void wakeup_chain_remove();

	bool started;
	bool stopping;
	bool yielded;

	jm_Scheduler();
	~jm_Scheduler();

	operator bool();

	void display(int line);

	static void time_cycle();
	static void cycle();
	static void yield();
	static void sleep(timestamp_t ival);

	// start routine immediately
	void start(voidfuncptr_t func);

	// start routine immediately and repeat it at fixed interval
	void start(voidfuncptr_t func, timestamp_t ival);

	// start routine on time and repeat it at fixed interval
	void start(voidfuncptr_t func, timestamp_t time, timestamp_t ival);

	// stop routine, current or scheduled, remove it from chain
	void stop();

	// rearm routine
	void rearm();

	// rearm routine asynchronously
	void rearm_async();

	// rearm routine and set interval
	void rearm(timestamp_t ival);

	// rearm routine asynchronously and set interval
	void rearm_async(timestamp_t ival);

//	// rearm routine, set time and set next interval
//	void rearm(timestamp_t time, timestamp_t ival);

	// rearm routine, change routine function and set interval
	void rearm(voidfuncptr_t func, timestamp_t ival);

	// rearm routine asynchronously, change routine function and set interval
	void rearm_async(voidfuncptr_t func, timestamp_t ival);

//	// rearm routine, change routine function, set time and set interval
//	void rearm(voidfuncptr_t func, timestamp_t time, timestamp_t ival);

	// wakeup a scheduled routine (maybe repeated)
	void wakeup();

	// wakeup a scheduled routine (maybe repeated but only 1st wakeup_time is recorded)
	void wakeup(uint32_t wakeup_time);

	// read wakeup count, reset it and remove routine from wakeup chain
	int wakeup_read();
};

//------------------------------------------------------------------------------

extern void yield(void);

//------------------------------------------------------------------------------

#endif

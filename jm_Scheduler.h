/*
	jm_Scheduler
	============

	jm_Scheduler.h and jm_Scheduler.cpp - Implementation of a general
	scheduler named "jm_Scheduler" to use in various environment
	like Arduino, Energia, MDEB, etc...

	Copyright (c) 2016,2015 Jean-Marc Paratte

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

    Last revised: 2016-07-07,2016-04-27,2015-06-29
*/

#ifndef jm_Scheduler_h
#define jm_Scheduler_h

//------------------------------------------------------------------------------

#ifdef ARDUINO
#include <Arduino.h>
typedef uint32_t timestamp_t; // mbed/ticker_api.h(21): typedef uint32_t timestamp_t;
#define us_ticker_read() ((timestamp_t)micros()) // mbed/us_ticker_api.h(54): uint32_t us_ticker_read(void);
#endif
//typedef int32_t stimestamp_t; // signed timestamp_t
#define timestamp_read() us_ticker_read()
//#define TIMESTAMP_TMAX ((timestamp_t)(1UL<<(sizeof(timestamp_t)*8-1))-1) // 2147483647=0x7FFFFFFF
// 4'294'967'296 = 1'0000'0000 = [2^32us]
// 3'600'000'000 =   D693'A400 = [1h]
//   694'967'296 =   296C'5C00 = [2^32us - 1h = 694.967296s = 11m + 34.967296s]
// 3'947'483'648 =   EB49'D200
//   268'435'456 =   1000'0000 = [268.435456s = 4m + 28.435456s]
//     1'000'000 =      F'4240 = [1s]
//    30'000'000 =    1C9'C380 = [30s]
// 4'264'951'808 =   FE36'0000 = 1'0000'0000 - 1CA'0000 = [1h + 11m + 4s + 951ms + 808us]
//    30'015'488 =    1CA'0000 = [30s + 15ms + 488us]
#define TIMESTAMP_TMAX (0xFE35FFFF) // [1h + 11m + 4s + 951ms + 808us - 1]

#define TIMESTAMP_1US	(1UL)					// [1us]
#define TIMESTAMP_1MS	(1000*TIMESTAMP_1US)	// [1ms]
#define TIMESTAMP_1SEC	(1000*TIMESTAMP_1MS)	// [1s]
#define TIMESTAMP_1MIN	(60*TIMESTAMP_1SEC)		// [1 minute]
#define TIMESTAMP_1HOUR	(60*TIMESTAMP_1MIN)		// [1 hour]

#ifndef assert
#define assert(v) while(!v){} // default assert function
#endif

#ifndef func_p_t
typedef void (*func_p_t)(void); // function pointer typedef
#endif

//------------------------------------------------------------------------------

#define jm_Scheduler_time_read() us_ticker_read()
#define jm_Scheduler_tref_read() (jm_Scheduler::tref)
#define jm_Scheduler_tref_ival(ival) (jm_Scheduler::tref + ival)

#define jm_Scheduler_time_ge_time(tref, time) ((timestamp_t)(tref - time) <= TIMESTAMP_TMAX)
#define jm_Scheduler_tref_ge_time(time) ((timestamp_t)(jm_Scheduler::tref - time) <= TIMESTAMP_TMAX)
#define jm_Scheduler_tref_ival_ge_time(ival, time) ((timestamp_t)(jm_Scheduler::tref + ival - time) <= TIMESTAMP_TMAX)

class jm_Scheduler
{
public:

	static timestamp_t tref;		// current scheduler time
	static jm_Scheduler *first;		// first scheduled coroutine chain
	static jm_Scheduler *crnt;		// current running coroutine

	static jm_Scheduler *wakeup_first; // first wakeup coroutine chain

public:

	func_p_t func;					// address of coroutine function
	timestamp_t time;				// time of scheduled execution
	timestamp_t ival;				// interval of cyclic execution

	jm_Scheduler *next;				// next in coroutine chain

	timestamp_t wakeup_time;		// time of first wakeup (may be repeated)
	jm_Scheduler *wakeup_next;		// next coroutine in wakeup coroutine chain
	int wakeup_count;				// count of repeated wakeup

	void chain_insert();
	void chain_remove();

	void wakeup_chain_append();
	void wakeup_chain_remove();

	bool started;
	bool stopping;

	jm_Scheduler();
	~jm_Scheduler();

	operator bool();

	void display(int line);

	static void time_cycle();
	static void cycle();

	// start coroutine immediately
	void start(func_p_t func);

	// start coroutine immediately and repeat it at fixed intervals
	void start(func_p_t func, timestamp_t ival);

	// start coroutine on time and repeat it at fixed intervals
	void start(func_p_t func, timestamp_t time, timestamp_t ival);

	// stop coroutine, current or scheduled, remove from chain
	void stop();

	// rearm current coroutine and set or reset interval
	void rearm(timestamp_t ival);

	// rearm current coroutine, change coroutine function and set or reset interval
	void rearm(void (*func)(), timestamp_t ival);

	// wakeup a scheduled coroutine (maybe repeated)
	void wakeup();

	// read wakeup count, reset it and remove coroutine from wakeup chain
	int wakeup_read();
};

//------------------------------------------------------------------------------

#endif

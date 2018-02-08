/*
	jm_Scheduler
	============

	jm_Scheduler.h and jm_Scheduler.cpp - Implementation of a general
	scheduler named "jm_Scheduler" to use in various environment
	like Arduino, Energia, MBED, etc...

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

    Last revised: 2017-11-10,2017-07-20,2017-04-26,2016-07-07,2016-04-27,2015-06-29
*/

#include <jm_Scheduler.h>

//------------------------------------------------------------------------------

timestamp_t jm_Scheduler::tref = timestamp_read();	// current scheduler time
jm_Scheduler *jm_Scheduler::first = 0;				// first scheduled routine chain
jm_Scheduler *jm_Scheduler::crnt = 0;				// current running routine

jm_Scheduler *jm_Scheduler::wakeup_first = 0;		// first wakeup routine chain

void jm_Scheduler::chain_insert()
{
	jm_Scheduler *prev1 = 0;
	jm_Scheduler *next1 = jm_Scheduler::first;

	while (next1 && jm_Scheduler_time_ge_time(this->time, next1->time))
	{
		prev1 = next1;
		next1 = next1->next;
	}

	if (prev1) prev1->next = this; else jm_Scheduler::first = this;

	this->next = next1;
}

void jm_Scheduler::chain_remove()
{
	jm_Scheduler *prev1 = 0;
	jm_Scheduler *next1 = jm_Scheduler::first;

	while (next1)
	{
		if (next1 == this) break;

		prev1 = next1;
		next1 = next1->next;
	}

	if (next1)
	{
		if (prev1)
			prev1->next = this->next;
		else
			jm_Scheduler::first = this->next;
	}

	this->next = 0;
}

void jm_Scheduler::wakeup_chain_append()
{
	jm_Scheduler *wakeup_prev1 = 0;
	jm_Scheduler *wakeup_next1 = jm_Scheduler::wakeup_first;

	while (wakeup_next1) // loop wakeup_chain
	{
		if (wakeup_next1 == this) return; // already inserted in wakeup_chain ? exit function

		wakeup_prev1 = wakeup_next1;
		wakeup_next1 = wakeup_next1->wakeup_next;
	}
	// end of wakeup_cain reached, this not found in it

	// insert this at end of wakeup_chain
	if (wakeup_prev1)
		wakeup_prev1->wakeup_next = this;
	else
		jm_Scheduler::wakeup_first = this;

//	this->wakeup_next = 0;
}

void jm_Scheduler::wakeup_chain_remove()
{
	jm_Scheduler *wakeup_prev1 = 0;
	jm_Scheduler *wakeup_next1 = jm_Scheduler::wakeup_first;

	while (wakeup_next1) // loop wakeup_chain
	{
		if (wakeup_next1 == this) // this found in wakeup_chain ?
		{
			// remove from wakeup_chain
			if (wakeup_prev1)
				wakeup_prev1->wakeup_next = this->wakeup_next;
			else
				jm_Scheduler::wakeup_first = this->wakeup_next;
			this->wakeup_next = 0;

			return; // now removed from wakeup_chain, exit function
		}

		wakeup_prev1 = wakeup_next1;
		wakeup_next1 = wakeup_next1->wakeup_next;
	}
}

//------------------------------------------------------------------------------

jm_Scheduler::jm_Scheduler() :
	func(NULL),
	time(0),
	ival(0),

	next(NULL),

	wakeup_time(0),			// time of first wakeup routine chain
	wakeup_next(NULL),			// next in wakeup routine chain
	wakeup_count(0),		// count of repeated interrupt routine

	started(false),
	stopping(false),
	yielded(false)
{
}

jm_Scheduler::~jm_Scheduler()
{
	this->stop();
}

jm_Scheduler::operator bool()
{
	return (this->started);
}

//------------------------------------------------------------------------------

void jm_Scheduler::display(int line)
{
	Serial.print(line);
	Serial.print(':');
	Serial.print(jm_Scheduler::tref);

//	Serial.print(' ');
//	Serial.print("first");
//	Serial.print('=');
	Serial.print(F(" first="));
	Serial.print((long)jm_Scheduler::first);

//	Serial.print(' ');
//	Serial.print("crnt");
//	Serial.print('=');
	Serial.print(F(" crnt="));
	Serial.print((long)jm_Scheduler::crnt);

//	Serial.print(' ');
//	Serial.print("this");
//	Serial.print('=');
	Serial.print(F(" this="));
	Serial.print((long)this);

//	Serial.print(' ');
//	Serial.print("time");
//	Serial.print('=');
	Serial.print(F(" time="));
	Serial.print(this->time);

//	Serial.print(' ');
//	Serial.print("ival");
//	Serial.print('=');
	Serial.print(F(" ival="));
	Serial.print(this->ival);

//	Serial.print(' ');
//	Serial.print("next");
//	Serial.print('=');
	Serial.print(F(" next="));
	Serial.print((long)this->next);

	Serial.println();
	Serial.flush();
}

//------------------------------------------------------------------------------

void jm_Scheduler::time_cycle()
{
	jm_Scheduler::tref = timestamp_read();
}

void jm_Scheduler::cycle()
{
	for (;;)
{
	jm_Scheduler::time_cycle();

	noInterrupts();

	if (jm_Scheduler::wakeup_first)
	{
		jm_Scheduler *wakeup_first0 = jm_Scheduler::wakeup_first;

		// remove wakeup_first
		jm_Scheduler::wakeup_first = wakeup_first0->wakeup_next;
		wakeup_first0->wakeup_next = 0;

		// remove wakeuped routine from routine chain
//		jm_Scheduler::wakeup_first->chain_remove();
		wakeup_first0->chain_remove();

		// insert wakeuped routine at first routine
		wakeup_first0->next = jm_Scheduler::first;
		jm_Scheduler::first = wakeup_first0;

		interrupts();
	}
	else
	{
		interrupts();

		if (jm_Scheduler::first == 0) break;

		if (!jm_Scheduler_tref_ge_time(jm_Scheduler::first->time)) break;
	}

	// set crnt with first
	jm_Scheduler::crnt = jm_Scheduler::first;

	// remove first from chain routine
	jm_Scheduler::first = jm_Scheduler::crnt->next;
	jm_Scheduler::crnt->next = 0;

	jm_Scheduler::crnt->func(); // call routine function

	if (jm_Scheduler::crnt->stopping || jm_Scheduler::crnt->ival == 0) // stopping or not rearmed ?
	{
		jm_Scheduler::crnt->started = false;
		jm_Scheduler::crnt->stopping = false;

		jm_Scheduler::crnt->wakeup_chain_remove();
	}
	else // rearmed/intervalled calls
	{
		jm_Scheduler::crnt->time += jm_Scheduler::crnt->ival; // synchonous

		jm_Scheduler::crnt->chain_insert();
	}

	jm_Scheduler::crnt = 0;
}
}

void jm_Scheduler::yield()
{
	if (jm_Scheduler::crnt) // called from a running routine ?
	{
		// backup routine states
//		timestamp_t tref0 = jm_Scheduler::tref;
		jm_Scheduler *crnt0 = jm_Scheduler::crnt;

		// set routine yielded state
		crnt0->yielded = true;

		jm_Scheduler::crnt = 0; // free scheduler from current routine
		jm_Scheduler::cycle(); // yield current routine

		// clr routine yielded state
		crnt0->yielded = false;

		// restore routine states
		jm_Scheduler::crnt = crnt0;
//		jm_Scheduler::tref = tref0;
	}
	else // called from setup() or loop().
	{
		jm_Scheduler::cycle();
	}
}

void jm_Scheduler::sleep(timestamp_t ival)
{
	timestamp_t time1 = jm_Scheduler_time_read() + ival;
	while (!jm_Scheduler_time_ge_time(jm_Scheduler_time_read(), time1)) jm_Scheduler::yield();
}

//------------------------------------------------------------------------------

// start routine immediately
void jm_Scheduler::start(voidfuncptr_t func)
{
	this->func = func;
	this->time = jm_Scheduler_time_read();
	this->ival = 0;

	this->chain_insert();

	this->wakeup_time = 0;		// time of first wakeup (may be repeated)
	this->wakeup_next = 0;		// next routine in wakeup routine chain
	this->wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
	this->yielded = false;
}

// start routine immediately and repeat it at fixed intervals
void jm_Scheduler::start(voidfuncptr_t func, timestamp_t ival)
{
	this->func = func;
	this->time = jm_Scheduler_time_read();
	this->ival = ival;

	this->chain_insert();

	this->wakeup_time = 0;		// time of first wakeup (may be repeated)
	this->wakeup_next = 0;		// next routine in wakeup routine chain
	this->wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
	this->yielded = false;
}

// start routine on time and repeat it at fixed intervals
void jm_Scheduler::start(voidfuncptr_t func, timestamp_t time, timestamp_t ival)
{
	this->func = func;
	this->time = time;
	this->ival = ival;

	this->chain_insert();

	this->wakeup_time = 0;		// time of first wakeup (may be repeated)
	this->wakeup_next = 0;		// next routine in wakeup routine chain
	this->wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
	this->yielded = false;
}

// stop routine, current or scheduled, remove from chain
void jm_Scheduler::stop()
{
	if (!this->started || this->stopping) return;

	if (this == jm_Scheduler::crnt) // routine running ?
	{
		this->stopping = true;
	}
	else
	{
		this->started = false;
		this->stopping = false;
		this->yielded = false;

		this->chain_remove();
	}
}

//------------------------------------------------------------------------------

// rearm routine
void jm_Scheduler::rearm()
{
}

// rearm routine asynchronously
void jm_Scheduler::rearm_async()
{
	this->time = jm_Scheduler_time_read();
}

// rearm routine and set next interval
void jm_Scheduler::rearm(timestamp_t ival)
{
	this->ival = ival;
}

// rearm routine asynchronously and set next interval
void jm_Scheduler::rearm_async(timestamp_t ival)
{
	this->time = jm_Scheduler_time_read();
	this->ival = ival;
}

//// rearm routine, set time and set next interval
//void jm_Scheduler::rearm(timestamp_t time, timestamp_t ival)
//{
//	this->time = time;
//	this->ival = ival;
//}

// rearm routine, change routine function and set next interval
void jm_Scheduler::rearm(voidfuncptr_t func, timestamp_t ival)
{
	this->func = func;
	this->ival = ival;
}

// rearm routine asynchronously, change routine function and set next interval
void jm_Scheduler::rearm_async(voidfuncptr_t func, timestamp_t ival)
{
	this->time = jm_Scheduler_time_read();
	this->func = func;
	this->ival = ival;
}

//// rearm routine, change routine function, set time and set next interval
//void jm_Scheduler::rearm(voidfuncptr_t func, timestamp_t time, timestamp_t ival)
//{
//	this->func = func;
//	this->time = time;
//	this->ival = ival;
//}

//------------------------------------------------------------------------------

// wakeup a scheduled routine (maybe repeated)
void jm_Scheduler::wakeup()
{
	if (!this->started) return;

//	if (this == jm_Scheduler::crnt) return;

	if (this->wakeup_count == 0) // set wakeup_time if never set before
	{
		this->wakeup_time = timestamp_read();

		this->wakeup_chain_append();
	}

	this->wakeup_count++; // inc wakeup_count
}

// wakeup a scheduled routine (maybe repeated but only 1st wakeup_time is recorded)
void jm_Scheduler::wakeup(uint32_t wakeup_time)
{
	if (!this->started) return;

//	if (this == jm_Scheduler::crnt) return;

	if (this->wakeup_count == 0) // set wakeup_time if never set before
	{
		this->wakeup_time = wakeup_time;

		this->wakeup_chain_append();
	}

	this->wakeup_count++; // inc wakeup_count
}

// read wake_count, reset it and remove routine from wakeup chain
int jm_Scheduler::wakeup_read()
{
	int count = 0;

	noInterrupts();

	if (this->wakeup_count)
	{
		count = this->wakeup_count;
		this->wakeup_count = 0;

		this->wakeup_chain_remove();
	}

	interrupts();

	return count;
}

//------------------------------------------------------------------------------

void yield(void)
{
	jm_Scheduler::cycle();
}

//------------------------------------------------------------------------------

// END.
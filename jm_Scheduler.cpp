/*
	jm_Scheduler
	===========

	jm_Scheduler.h and jm_Scheduler.cpp - Implementation of a general
	scheduler named "jm_Scheduler" to use in various environment
	like Arduino, Energia, MBED, etc...

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

#include <jm_Scheduler.h>

//------------------------------------------------------------------------------

timestamp_t jm_Scheduler::tref = timestamp_read();	// current scheduler time
jm_Scheduler *jm_Scheduler::first = 0;				// first scheduled coroutine chain
jm_Scheduler *jm_Scheduler::crnt = 0;				// current running coroutine

jm_Scheduler *jm_Scheduler::wakeup_first = 0;		// first wakeup coroutine chain

void jm_Scheduler::chain_insert()
{
	jm_Scheduler *prev1 = 0;
	jm_Scheduler *next1 = jm_Scheduler::first;

//	while (next1 && (long)(this->time - next1->time) >= 0)
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
		if (prev1)
			prev1->next = this->next;
		else
			jm_Scheduler::first = this->next;

	this->next = 0;
}

void jm_Scheduler::wakeup_chain_append()
{
	jm_Scheduler *wakeup_prev1 = 0;
	jm_Scheduler *wakeup_next1 = jm_Scheduler::wakeup_first;

	while (wakeup_next1)
	{
		if (wakeup_next1 == this) return;

		wakeup_prev1 = wakeup_next1;
		wakeup_next1 = wakeup_next1->wakeup_next;
	}

	if (wakeup_prev1)
		wakeup_prev1->wakeup_next = this;
	else
		jm_Scheduler::wakeup_first = this;

	this->wakeup_next = 0;
}

void jm_Scheduler::wakeup_chain_remove()
{
	jm_Scheduler *wakeup_prev1 = 0;
	jm_Scheduler *wakeup_next1 = jm_Scheduler::wakeup_first;

	while (wakeup_next1)
	{
		if (wakeup_next1 == this) break;

		wakeup_prev1 = wakeup_next1;
		wakeup_next1 = wakeup_next1->wakeup_next;
	}

	if (wakeup_next1)
		if (wakeup_prev1)
			wakeup_prev1->wakeup_next = this->wakeup_next;
		else
			jm_Scheduler::wakeup_first = this->wakeup_next;

	this->wakeup_next = 0;
}

jm_Scheduler::jm_Scheduler()
{
	this->func = 0;
	this->time = 0;
	this->ival = 0;

	this->next = 0;

	this->wakeup_time = 0;	// time of first wakeup coroutine chain
	this->wakeup_next = 0;	// next in wakeup coroutine chain
	this->wakeup_count = 0;	// count of repeated interrupt coroutine

	this->started = false;
	this->stopping = false;
}

jm_Scheduler::~jm_Scheduler()
{
	this->stop();
}

jm_Scheduler::operator bool()
{
	return (this->started);
}

void jm_Scheduler::display(int line)
{
	Serial.print(line);
	Serial.print(':');
	Serial.print(jm_Scheduler::tref);

	Serial.print(' ');
	Serial.print("first");
	Serial.print('=');
	Serial.print((long)jm_Scheduler::first);

	Serial.print(' ');
	Serial.print("crnt");
	Serial.print('=');
	Serial.print((long)jm_Scheduler::crnt);

	Serial.print(' ');
	Serial.print("this");
	Serial.print('=');
	Serial.print((long)this);

	Serial.print(' ');
	Serial.print("time");
	Serial.print('=');
	Serial.print(this->time);

	Serial.print(' ');
	Serial.print("ival");
	Serial.print('=');
	Serial.print(this->ival);

	Serial.print(' ');
	Serial.print("next");
	Serial.print('=');
	Serial.print((long)this->next);

	Serial.println();
	Serial.flush();
}

void jm_Scheduler::time_cycle()
{
	jm_Scheduler::tref = timestamp_read();
}

void jm_Scheduler::cycle()
{
	for (;;)
{
	jm_Scheduler::time_cycle();

	cli();

	if (jm_Scheduler::wakeup_first)
	{
		jm_Scheduler *wakeup_first = jm_Scheduler::wakeup_first;

		// remove wakeup_first
		jm_Scheduler::wakeup_first = wakeup_first->wakeup_next;
		wakeup_first->wakeup_next = 0;

		// remove wakeuped coroutine from coroutine chain
		jm_Scheduler::wakeup_first->chain_remove();

		// insert wakeuped coroutine at first coroutine
		wakeup_first->next = jm_Scheduler::first;
		jm_Scheduler::first = wakeup_first;

		sei();
	}
	else
	{
		sei();

		if (jm_Scheduler::first == 0) break;

		if (!jm_Scheduler_tref_ge_time(jm_Scheduler::first->time)) break;
	}

//	jm_Scheduler::first->display(__LINE__);

	// set crnt with first
	jm_Scheduler::crnt = jm_Scheduler::first;

	// remove first from chain coroutine
	jm_Scheduler::first = jm_Scheduler::crnt->next;
	jm_Scheduler::crnt->next = 0;

	jm_Scheduler::crnt->func(); // call coroutine function

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

// start coroutine immediately
void jm_Scheduler::start(func_p_t func)
{
	this->func = func;
	this->time = jm_Scheduler_time_read();
	this->ival = 0;

	this->chain_insert();

	wakeup_time = 0;		// time of first wakeup (may be repeated)
	wakeup_next = 0;		// next coroutine in wakeup coroutine chain
	wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
}

// start coroutine immediately and repeat it at fixed intervals
void jm_Scheduler::start(func_p_t func, timestamp_t ival)
{
	this->func = func;
	this->time = jm_Scheduler_time_read();
	this->ival = ival;

	this->chain_insert();

	wakeup_time = 0;		// time of first wakeup (may be repeated)
	wakeup_next = 0;		// next coroutine in wakeup coroutine chain
	wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
}

// start coroutine on time and repeat it at fixed intervals
void jm_Scheduler::start(func_p_t func, timestamp_t time, timestamp_t ival)
{
	this->func = func;
	this->time = time;
	this->ival = ival;

	this->chain_insert();

	wakeup_time = 0;		// time of first wakeup (may be repeated)
	wakeup_next = 0;		// next coroutine in wakeup coroutine chain
	wakeup_count = 0;		// count of repeated wakeup

	this->started = true;
	this->stopping = false;
}

// stop coroutine, current or scheduled, remove from chain
void jm_Scheduler::stop()
{
	if (!this->started || this->stopping) return;

	if (this == this->crnt) // coroutine running ?
	{
//		this->started = false;
		this->stopping = true;
	}
	else
	{
		this->started = false;
		this->stopping = false;

		this->chain_remove();
	}
}

// rearm current coroutine and set or reset interval
void jm_Scheduler::rearm(timestamp_t ival)
{
	this->ival = ival;
}

// rearm current coroutine, change coroutine function and set or reset interval
void jm_Scheduler::rearm(void (*func)(), timestamp_t ival)
{
	this->func = func;
	this->ival = ival;
}

// wakeup a scheduled coroutine (maybe repeated)
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

// read wake_count, reset it and remove coroutine from wakeup chain
int jm_Scheduler::wakeup_read()
{
	int count = 0;

	cli();

	if (this->wakeup_count)
	{
		count = this->wakeup_count;
		this->wakeup_count = 0;

		this->wakeup_chain_remove();
	}

	sei();

	return count;
}

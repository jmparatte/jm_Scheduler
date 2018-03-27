
#define __PROG__ "Wakeup1"

/*
	Wakeup1.ino demonstrate the possible interaction of interrupt with scheduled coroutine.

	Wakeup1.ino implements an 10s timeout to receive an interrupt.
	The interrupt is activated when the Arduino UNO pin 2 is shortcut to GND.
	The elapsed time between Arduino RESET and pin INT is printed.
	If no interrupt is provided during 10s, then the message "timeout" is printed.

	Copyright (c) 2016,2015 Jean-Marc Paratte

	Wakeup1.ino is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wakeup1.ino is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wakeup1.ino.  If not, see <http://www.gnu.org/licenses/>.

    Last revised: 2016-07-08,2015-06-29
*/

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

/*
	https://www.arduino.cc/en/Reference/AttachInterrupt.html

	Board								Digital Pins Usable For Interrupts
	----------------------------------------------------------------------
	Uno, Nano, Mini, other 328-based	2, 3
	Mega, Mega2560, MegaADK				2, 3, 18, 19, 20, 21
	Micro, Leonardo, other 32u4-based	0, 1, 2, 3, 7
	Zero								all digital pins, except 4
	MKR1000 Rev.1						0, 1, 4, 5, 6, 7, 8, 9, A1, A2
	Due									all digital pins
*/

#define WAKEUP_PIN (2) // All models same digital pin usable for interrupts except MKR1000 Rev.1

//------------------------------------------------------------------------------

const timestamp_t WAKEUP_TIMEOUT = 5*TIMESTAMP_1SEC; // 5s

jm_Scheduler wakeup_scheduler;

void wakeup_coroutine_start()
{
	Serial.print(F("          "));
	Serial.println( F("start... ") );

	led_on(); // LED ON

	wakeup_scheduler.rearm( wakeup_coroutine_stop, WAKEUP_TIMEOUT ); // set timeout

	// flush possible pending FALLING interrupt
	attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), dummy_interrupt, FALLING); // attach int.0 (Arduino UNO pin 2).
//	detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN)); // detach int.0 (Arduino UNO pin 2).

	attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeup_interrupt, FALLING); // attach int.0 (Arduino UNO pin 2).
}

void wakeup_coroutine_stop()
{
	detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN)); // detach int.0 (Arduino UNO pin 2).

	led_off(); // LED OFF

	timestamp_t wakeup_time = wakeup_scheduler.wakeup_time;
	timestamp_t wakeup_count = wakeup_scheduler.wakeup_read();

	if (wakeup_count == 0) // timeout ?
	{
		Serial.print(F("          "));
		Serial.println( F("timeout") );
	}
	else
	{
		Serial.print(F("          "));
		Serial.print( (wakeup_time - (wakeup_scheduler.time - WAKEUP_TIMEOUT))/1E6, 6 ); // print elapsed time in seconds.
		Serial.println( F("s") );
		Serial.print(F("          "));
		Serial.print( wakeup_count );
		Serial.println( F(" wakeup(s)") ); // print count of wakeups
	}

	wakeup_scheduler.stop(); // stop coroutine
}

void wakeup_interrupt()
{
	wakeup_scheduler.wakeup();
}

void dummy_interrupt()
{
	// does nothing!
}

//------------------------------------------------------------------------------

uint8_t clock_sec = 0; // 0..59 seconds => 1 minute
uint8_t clock_min = 0; // 0..59 minutes => 1 hour
uint8_t clock_h24 = 0; // 0..23 hours => 1 day

void clock_inc()
{
	clock_sec++;
	if (clock_sec == 60)
	{
		clock_sec = 0;
		clock_min++;
		if (clock_min == 60)
		{
			clock_min = 0;
			clock_h24++;
			if (clock_h24 == 24) clock_h24 = 0;
		}
	}
}

void clock_display()
{
	Serial.print( clock_h24/10 );
	Serial.print( clock_h24%10 );
	Serial.print( ':' );
	Serial.print( clock_min/10 );
	Serial.print( clock_min%10 );
	Serial.print( ':' );
	Serial.print( clock_sec/10 );
	Serial.print( clock_sec%10 );
	Serial.println();
}

//------------------------------------------------------------------------------

jm_Scheduler clock_scheduler;

void clock_coroutine()
{
	static bool coroutine_first_start = true;

	if (!coroutine_first_start) clock_inc();

	clock_display();

	if (!wakeup_scheduler && digitalRead(WAKEUP_PIN)) wakeup_scheduler.start(wakeup_coroutine_start);

	coroutine_first_start = false;
}

//------------------------------------------------------------------------------

void setup()
{
	Serial.begin(115200);
	while (!Serial && millis()<3000); // timeout 3s for USB Serial ready
	Serial.print(F(__PROG__));
	Serial.print(F("..."));
	Serial.println();

	led_init();

	clock_scheduler.start(clock_coroutine, TIMESTAMP_1SEC); // Start coroutine immediately and repeat it every 1s.

	pinMode(WAKEUP_PIN, INPUT_PULLUP); // Arduino UNO pin 2 is int.0
//	while (!digitalRead(WAKEUP_PIN)); // wait for pullup
}

void loop()
{
	yield();
}

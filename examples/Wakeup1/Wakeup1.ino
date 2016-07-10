
#define __PROG__ "Wakeup1"

/*
	Wakeup1.ino demonstrate the possible interaction of interrupt with scheduled routine.

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

void led_init()
{
	pinMode(LED_BUILTIN, OUTPUT);
}

void led_on()
{
	digitalWrite(LED_BUILTIN, HIGH);
}

void led_off()
{
	digitalWrite(LED_BUILTIN, LOW);
}

void led_write(bool state)
{
	if (state) led_on(); else led_off();
}

bool led_state()
{
	return (digitalRead(LED_BUILTIN) == HIGH);
}

void led_toggle()
{
	led_write(!led_state());
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
	Serial.flush();
}

//------------------------------------------------------------------------------

jm_Scheduler clock_scheduler;

void clock_routine()
{
	clock_display();

	clock_inc();
}

//------------------------------------------------------------------------------

const timestamp_t wakeup_timeout = 10*TIMESTAMP_1SEC; // 10s

jm_Scheduler wakeup_scheduler;

void wakeup_routine_start()
{
	Serial.println( F("wakeup... ") );

	led_on(); // LED ON

	wakeup_scheduler.rearm( wakeup_routine_stop, wakeup_timeout ); // set timeout

	attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), dummy_interrupt, FALLING); // attach int.0 (Arduino UNO pin 2).
	detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN)); // detach int.0 (Arduino UNO pin 2).

	attachInterrupt(digitalPinToInterrupt(WAKEUP_PIN), wakeup_interrupt, FALLING); // attach int.0 (Arduino UNO pin 2).
}

void wakeup_routine_stop()
{
	detachInterrupt(digitalPinToInterrupt(WAKEUP_PIN)); // detach int.0 (Arduino UNO pin 2).

	led_off(); // LED OFF

	timestamp_t wakeup_time = wakeup_scheduler.wakeup_time;
	timestamp_t wakeup_count = wakeup_scheduler.wakeup_read();

	if (wakeup_count == 0) // timeout ?
	{
		Serial.println( F("timeout") );
	}
	else
	{
		Serial.print( (wakeup_time - (wakeup_scheduler.time - wakeup_timeout))/1E6, 6 ); // print elapsed time in seconds.
		Serial.println( F("s") );
		Serial.print( wakeup_count );
		Serial.println( F(" wakeup(s)") ); // print count of wakeups
	}

	wakeup_scheduler.stop(); // stop routine
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

void setup()
{
	Serial.begin(115200);
	while (!Serial && millis()<3000); // timeout 3s for USB Serial ready
	Serial.print(F(__PROG__));
	Serial.print(F("..."));
	Serial.println();

	led_init();

	clock_scheduler.start(clock_routine, TIMESTAMP_1SEC); // Start routine immediately and repeat it every 1s.

	pinMode(WAKEUP_PIN, INPUT_PULLUP); // Arduino UNO pin 2 is int.0
	while (!digitalRead(WAKEUP_PIN)); // wait for pullup
}

void loop()
{
	jm_Scheduler::cycle();

//	if (!wakeup_scheduler) wakeup_scheduler.start(wakeup_routine_start);
	if (!wakeup_scheduler) wakeup_scheduler.start(wakeup_routine_start, jm_Scheduler_tref_ival(TIMESTAMP_1SEC), 0);
}

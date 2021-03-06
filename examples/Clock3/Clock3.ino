
#define __PROG__ "Clock3"

#include <jm_Scheduler.h>

#include "led.h"

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

	if (!led_state())
	{
		if (!coroutine_first_start) clock_inc();

		led_on(); // LED ON, pulse LED every second

		clock_display();

		clock_scheduler.rearm( 20*TIMESTAMP_1MS ); // 20ms
	}
	else
	{
		led_off(); // LED OFF

		clock_scheduler.rearm( TIMESTAMP_1SEC - 20*TIMESTAMP_1MS ); // 1s - 20ms
	}

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

	clock_scheduler.start(clock_coroutine); // Start coroutine immediately, interval will be set later.
}

void loop()
{
	yield();
}

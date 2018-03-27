
#define __PROG__ "Clock2"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

uint8_t clock_t50 = 0; // 0..49 ticks(20ms/50hz) => 1 second

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
	if (clock_t50 == 0) led_on(); // LED ON, pulse 20ms every 1s
	if (clock_t50 == 1) led_off(); // LED OFF.

	if (clock_t50 == 0) // display clock every second
	{
		clock_display();
	}

	clock_t50++;

	if (clock_t50 == 50)
	{
		clock_t50 = 0;

		clock_inc();
	}
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

	clock_scheduler.start(clock_coroutine, 20*TIMESTAMP_1MS); // Start coroutine immediately and repeat it every 20ms => 50hz.
}

void loop()
{
	yield();
}

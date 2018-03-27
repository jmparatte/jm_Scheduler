
#define __PROG__ "Beat3"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

jm_Scheduler beat_scheduler;

bool beat_state = false;
timestamp_t beat_ival = 0;

void beat_coroutine()
{
	if (!beat_state)
	{
		beat_state = true;

		led_toggle();

		beat_ival += TIMESTAMP_1MS/10; // +0.1ms

		beat_scheduler.rearm(beat_ival);
	}
	else
	{
		beat_state = false;

		if (beat_ival < 10*TIMESTAMP_1MS)
			led_toggle();
		else
			beat_ival = 0;

		beat_scheduler.rearm(10*TIMESTAMP_1MS - beat_ival); // 10ms - beat_ival
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

	beat_scheduler.start(beat_coroutine, 10*TIMESTAMP_1MS); // 10ms
}

void loop()
{
	yield();
}
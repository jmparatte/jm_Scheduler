
#define __PROG__ "Blink4"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

jm_Scheduler blink_scheduler;

void blink_coroutine()
{
	if (!led_state())
	{
		led_on();

		blink_scheduler.rearm(100*TIMESTAMP_1MS);
	}
	else
	{
		led_off();

		blink_scheduler.rearm(900*TIMESTAMP_1MS);
	}
}

void setup()
{
	Serial.begin(115200);
	while (!Serial && millis()<3000); // timeout 3s for USB Serial ready
	Serial.print(F(__PROG__));
	Serial.print(F("..."));
	Serial.println();

	led_init();

	blink_scheduler.start(blink_coroutine);
}

void loop()
{
	jm_Scheduler::yield();
}
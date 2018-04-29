
#define __PROG__ "Blink2"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

jm_Scheduler blink_scheduler;

void blink_coroutine()
{
	for(;;)
	{
		led_on();

		jm_Scheduler::delay(100);

		led_off();

		jm_Scheduler::delay(900);
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
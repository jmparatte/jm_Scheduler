
#define __PROG__ "Blink3"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

jm_Scheduler blink_scheduler;

void blink_coroutine()
{
	led_on();

	jm_Scheduler::delay(100);

	led_off();

	blink_scheduler.rearm(TIMESTAMP_1SEC);
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
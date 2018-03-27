
#define __PROG__ "Count1"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

long count = 0;

jm_Scheduler count1_scheduler;

void count1_coroutine()
{
	led_toggle();
	count++;
	count1_scheduler.rearm_async(1);
}

jm_Scheduler count_scheduler;

void count_coroutine()
{
	Serial.println(count);
	count = 0;
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

	count1_scheduler.start(count1_coroutine); // Start coroutine immediately.
	count_scheduler.start(count_coroutine, TIMESTAMP_1SEC); // Start coroutine immediately and repeat it every 1s.
}

void loop()
{
	yield();
}

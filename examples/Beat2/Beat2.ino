
#define __PROG__ "Beat2"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

jm_Scheduler beat_scheduler_led_toggle1;
jm_Scheduler beat_scheduler_led_toggle2;

void beat_coroutine_led_toggle()
{
	led_toggle();
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

	beat_scheduler_led_toggle1.start(beat_coroutine_led_toggle, 10000L); // 10ms
	beat_scheduler_led_toggle2.start(beat_coroutine_led_toggle, 10100L); // 10.1ms
}

void loop()
{
	yield();
}
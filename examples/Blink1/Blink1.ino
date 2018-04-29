
#define __PROG__ "Blink1"

#include <jm_Scheduler.h>

#include "led.h"

//------------------------------------------------------------------------------

void setup()
{
	Serial.begin(115200);
	while (!Serial && millis()<3000); // timeout 3s for USB Serial ready
	Serial.print(F(__PROG__));
	Serial.print(F("..."));
	Serial.println();

	led_init();
}

void loop()
{
	led_on();

	jm_Scheduler::delay(100);

	led_off();

	jm_Scheduler::delay(900);
}
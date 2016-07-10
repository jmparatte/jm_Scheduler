
#define __PROG__ "Beat3"

#include <jm_Scheduler.h>

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

jm_Scheduler beat_scheduler;

bool beat_state = false;
timestamp_t beat_ival = 0;

void beat_routine()
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

	beat_scheduler.start(beat_routine, 10*TIMESTAMP_1MS); // 10ms
}

void loop()
{
	jm_Scheduler::cycle();
}
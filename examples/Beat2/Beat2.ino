
#define __PROG__ "Beat2"

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

jm_Scheduler beat_scheduler_led_toggle1;
jm_Scheduler beat_scheduler_led_toggle2;

void beat_routine_led_toggle()
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

	beat_scheduler_led_toggle1.start(beat_routine_led_toggle, 10000L); // 10ms
	beat_scheduler_led_toggle2.start(beat_routine_led_toggle, 10100L); // 10.1ms
}

void loop()
{
	jm_Scheduler::cycle();
}
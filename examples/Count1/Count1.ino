
#define __PROG__ "Count1"

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

long count = 0;

jm_Scheduler count1_scheduler;

void count1_routine()
{
	led_toggle();
	count++;
	count1_scheduler.rearm_async(1);
}

jm_Scheduler count_scheduler;

void count_routine()
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

	count1_scheduler.start(count1_routine); // Start routine immediately.
	count_scheduler.start(count_routine, TIMESTAMP_1SEC); // Start routine immediately and repeat it every 1s.
}

void loop()
{
	jm_Scheduler::cycle();
}

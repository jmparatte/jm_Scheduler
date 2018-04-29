
#ifdef ARDUINO_ARCH_ESP32
#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#warning LED_BUILTIN has been defined hopefully on digital pin 2 in this file.
#endif
#define LED_ON LOW
#define LED_OFF HIGH
#else
#define LED_ON HIGH
#define LED_OFF LOW
#endif

#ifndef LED_BUILTIN
#error LED_BUILTIN is undefined! Please define one in this file.
#endif

// C:\Arduino15\hardware\arduino\avr\cores\arduino

void led_init()
{
	digitalWrite(LED_BUILTIN, LED_OFF);
	pinMode(LED_BUILTIN, OUTPUT);
}

bool led_state()
{
	return (digitalRead(LED_BUILTIN) == LED_ON);
}

void led_on()
{
	digitalWrite(LED_BUILTIN, LED_ON);
}

void led_off()
{
	digitalWrite(LED_BUILTIN, LED_OFF);
}

void led_write(bool state)
{
	if (state) led_on(); else led_off();
}

void led_toggle()
{
	led_write(!led_state());
}

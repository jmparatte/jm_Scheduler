
// C:\Arduino15\hardware\arduino\avr\cores\arduino

void led_init()
{
	digitalWrite(LED_BUILTIN, LOW);
	pinMode(LED_BUILTIN, OUTPUT);
}

bool led_state()
{
	return (digitalRead(LED_BUILTIN) == HIGH);
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

void led_toggle()
{
	led_write(!led_state());
}

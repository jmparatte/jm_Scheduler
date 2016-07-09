# Scheduler Arduino Library

2016-07-08: Initial commit

### Example

	// This example start a coroutine that print a char every second
	
	#include <jm_Scheduler.h>
  
	jm_Scheduler scheduler;
	
	void coroutine()
	{
		Serial.print('.');
	}
  
	void setup(void)
	{
		Serial.begin(9600);
		
		scheduler.start(coroutine, TIMESTAMP_1SEC); // Start immediatly coroutine() and repeat it every second
	}
  
	void loop(void)
	{
		jm_Scheduler::cycle();
	}


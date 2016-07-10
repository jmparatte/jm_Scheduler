# Scheduler Arduino Library

2016-07-08: Initial commit

### Example

	// This example schedules a routine every second
	
	#include <jm_Scheduler.h>
  
	jm_Scheduler scheduler;
	
	void routine()
	{
		Serial.print('.');
	}
  
	void setup(void)
	{
		Serial.begin(9600);
		
		scheduler.start(routine, TIMESTAMP_1SEC); // Start immediatly routine() and repeat it every second
	}
  
	void loop(void)
	{
		jm_Scheduler::cycle();
	}


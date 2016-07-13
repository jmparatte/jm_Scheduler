<!--

2016-07-13: In progress...
2016-07-08: Initial commit.

Licence: LGPL v3

Author: Jean-Marc Paratte
Email: jean-marc@paratte.ch

-->

<style>

.markdown-body pre,
.markdown-body code {
	background-color: #e0e0e0;
}

</style>

# jm_Scheduler - A Scheduler Library for Arduino

2016-07-08: Initial commit

### Concepts

**jm_Scheduler** schedules repeated and intervaled routines like the JavaScript **setInterval()** function does, but with some differences:

- By default, **jm_Scheduler** starts immediately the routine and repeats it periodically.
- The 1st execution can be differed.
- The repeated executions can be voided.
- The interval between executions can be dynamically modified.
- The execution can be stopped and later restarted.
- The executed routine can be dynamically changed.

### Basic Example

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
		
		scheduler.start(routine, TIMESTAMP_1SEC); // Start immediately routine() and repeat it every second
	}
  
	void loop(void)
	{
		jm_Scheduler::cycle();
	}

### Study Plan

- Begin with example **Clock1.ino**. This example demonstrates the advantages to start immediately a time display routine and periodically repeat it.
- Follow with examples **Clock2.ino** and **Clock3.ino** which present other timing ways.
- **Clock4.ino** example presents a usefully **jm_Scheduler** technic: changing dynamically the routine to execute.
- **Beat1.ino** and **Beat2.ino** examples present interaction between 2 scheduling routines.
- **Wakeup1.ino** example demonstrates the possible interaction between an interrupt and a scheduled routine, implementing a timeout.

### Timestamp

The _timestamp_ is read from the Arduino function **micros()**.
By design, the **micros()** function of Arduino UNO and Leonardo running at 16MHz returns a _[us]_ _timestamp_ with a resolution of _[4us]_.

**micros()** declaration is:

```C
unsigned long micros()
```

Look at https://www.arduino.cc/en/Reference/Micros for details.

### More about Timestamp

_timestamp_ is a 32bit _[us]_ counter and it overflows about every 70 minutes (precisely 1h+11m+34s+967ms+296us).

The periodicity of 70 minutes is sometimes not enough to control slow processes.
Look next section for answers and tricks.

### Changing of Timestamp

Here are some hacks that can be implemented by modifying the file **jm_Scheduler.h**.

- Another choice for the _timestamp_ resolution could be the _[ms]_ read from the Arduino function **millis()**. 
- Gain speed during _timestamp_ comparison by shortening the size to 16bit.
- Obtain very long periodicity by implementing a 64bit _timestamp_.

### Timestamp declaration and constants

```C
typedef uint32_t timestamp_t;

#define timestamp_read() ((timestamp_t)micros())

#define TIMESTAMP_DEAD (0x01CA0000) // dead time [30s + 15ms + 488us]
#define TIMESTAMP_TMAX (0xFE35FFFF) // [1h + 11m + 4s + 951ms + 808us - 1]

#define TIMESTAMP_1US	(1UL)					// [1us]
#define TIMESTAMP_1MS	(1000*TIMESTAMP_1US)	// [1ms]
#define TIMESTAMP_1SEC	(1000*TIMESTAMP_1MS)	// [1s]
#define TIMESTAMP_1MIN	(60*TIMESTAMP_1SEC)		// [1 minute]
#define TIMESTAMP_1HOUR	(60*TIMESTAMP_1MIN)		// [1 hour]
```

All _timestamp_ variables are of type `timestamp_t`.

The _timestamp_ can be read with function `timestamp_read()`.

<img src="http://jean-marc.paratte.ch/wp-content/uploads/2013/01/diduino1_960x96.jpg" class="header-image" alt="jmP" height="96" width="960">

# jm_Scheduler - A Cooperative Scheduler Library for Arduino

```
2018-04-29: v1.0.9 - jm_Scheduler is now compatible with EPS32. 5 new Blink examples added.
2018-04-19: v1.0.8 - yield() function corrected.
2018-03-27: v1.0.7 - A Cooperative Scheduler Library for Arduino.
2018-02-08: v1.0.6 - Minor adjustments.
2017-10-17: v1.0.5 - Minor adjustments.
2017-05-08: v1.0.4 - Minor adjustments.
2017-05-08: v1.0.4 - Minor adjustments.
2017-05-05: v1.0.3 - Adding yield(),sleep(),rearm_async(). Removing void rearm(timestamp_t time, timestamp_t ival);
2017-04-26: v1.0.2 - Adding void rearm(timestamp_t time, timestamp_t ival);
2017-03-29: v1.0.1 - Minor adjustments.
2016-07-08: v1.0.0 - Initial commit.
```

### Concept

**jm_Scheduler** schedules repeated and intervaled coroutines like the JavaScript `setInterval()` function does,
but with some improvements:

- By default, **jm_Scheduler** starts immediately the coroutine and repeats it periodically.
- The first execution can be differed.
- The repeated executions can be voided.
- The interval between executions can be dynamically changed.
- The scheduled coroutine function can be dynamically changed.
- The scheduled coroutine can be stopped and later restarted.

**jm_Scheduler** doesn't schedule like the official [**Scheduler** Library for Arduino DUE and ZERO](https://www.arduino.cc/en/Reference/Scheduler) does,
`yield()` function which suspends a task is implemented, but 
`startLoop()` function which allocates a stack to the new task is not implemented.

**jm_Scheduler** schedules tasks sequentially on the stack processor.
The rules to _yield_ and _resume_ are:

- _yield_ comes out when coroutine leaves at end of function or by an explicit `return` instruction.
- _resume_ to a next state can be done with a variable and a `switch` instruction. Or:
- _resume_ to a next state can be done by switching to another function.
- Persistent variables must be implemented _global_ or _local_ with the pragma `static`.


### Basic Example

	// This example schedules a coroutine every second
	
	#include <jm_Scheduler.h>
  
	jm_Scheduler scheduler;
	
	void coroutine()
	{
		Serial.print('.');
	}
  
	void setup(void)
	{
		Serial.begin(9600);
		
		scheduler.start(coroutine, TIMESTAMP_1SEC); // Start immediately coroutine() and repeat it every second
	}
  
	void loop(void)
	{
		yield();
	}


### Study Plan

2018-04-29: NEW - Begin with **Blink*.ino** examples. Theses examples demonstrate how to blink a led by replacing delay() Arduino function with rearm() jm_Scheduler method.
- Begin with example **Clock1.ino**. This example demonstrates the advantage to start immediately a time display coroutine and periodically repeat it.
- Follow with examples **Clock2.ino** and **Clock3.ino** which present other timing ways.
- **Clock4.ino** example presents a usefully **jm_Scheduler** technical: changing dynamically the function to execute.
- **Beat1.ino** and **Beat2.ino** examples present interaction between 2 scheduled coroutines.
- **Wakeup1.ino** example demonstrates the possible interaction between an interrupt and a scheduled coroutine, implementing a timeout.


### Timestamp

The _timestamp_ is read from the Arduino function `micros()`.
By design, the `micros()` function of Arduino UNO and Leonardo running at 16MHz returns a [us] _timestamp_ with a resolution of [4us].

`micros()` declaration is:

```C
unsigned long micros();
```

Look at https://www.arduino.cc/en/Reference/Micros for details.

<!--
### More about Timestamp
-->

_timestamp_ is a 32bit [us] counter and overflows about every 70 minutes (precisely 1h+11m+34s+967ms+296us).

<!--
The periodicity of 70 minutes is sometimes not enough to control slow processes.
Look next section for answers and tricks.
-->


### Timestamp declaration and constants

```C
typedef uint32_t timestamp_t;

#define timestamp_read() ((timestamp_t)micros())

#define TIMESTAMP_DEAD (0x01CA0000) // coroutine dead time [30s + 15ms + 488us]
#define TIMESTAMP_TMAX (0xFE35FFFF) // [1h + 11m + 4s + 951ms + 808us - 1]

#define TIMESTAMP_1US	(1UL)					// [1us]
#define TIMESTAMP_1MS	(1000*TIMESTAMP_1US)	// [1ms]
#define TIMESTAMP_1SEC	(1000*TIMESTAMP_1MS)	// [1s]
#define TIMESTAMP_1MIN	(60*TIMESTAMP_1SEC)		// [1 minute]
#define TIMESTAMP_1HOUR	(60*TIMESTAMP_1MIN)		// [1 hour]
```

> `timestamp_t` defines the type of all _timestamp_ values.

> `timestamp_read()` returns the instantaneous _timestamp_.
This function can also be used by interrupt coroutines to _timestamp_ they data.

> `TIMESTAMP_DEAD` is the maximum allowed execution time of a coroutine to guarantee right scheduling.
If the coroutine doesn't end before, the scheduler could miss very long scheduling (see next).

> `TIMESTAMP_TMAX` is the maximum allowed scheduling time of a coroutine.
In practice, don't use _timestamp_ values greater than 1 hour.

### jm_Scheduler methods

```C
// start coroutine immediately
void start(voidfuncptr_t func);

// start coroutine immediately and repeat it at fixed interval
void start(voidfuncptr_t func, timestamp_t ival);

// start coroutine on time and repeat it at fixed interval
void start(voidfuncptr_t func, timestamp_t time, timestamp_t ival);

// stop coroutine, current or scheduled, remove it from chain
void stop();

// rearm current coroutine and set or reset interval
void rearm(timestamp_t ival);

// rearm current coroutine, change coroutine function and set or reset interval
void rearm(voidfuncptr_t func, timestamp_t ival);

> `start()` initiates a scheduler variable, starts a coroutine function, immediately or on time, with or without repetitions.
`start()` is invoked once. Next `rearm()` allows changing scheduler values.

> `stop()` cancels further execution of a scheduled coroutine. 
`stop()` can be invoked from inside coroutine or elsewhere.
If invoked from inside _coroutine_, `stop()` doesn't exit the function, just cancels further execution.

> `rearm()` changes values of a scheduler variable.
The new values are evaluated on exit coroutine function.
The main usage is to change _interval_ or _function_ or both or else cancel further execution.
```


### jm_Scheduler loop

```C
static void cycle();
```

> `cycle()` is the cornerstone of the scheduler and must be invoked as often as possible. 
Note that `cycle()` is a _static_ _method_. 
The right place is in Arduino `loop()` function.
Example:

```C
void loop(void)
{
	yield();
}
```

> `cycle()` can also be invoked in Arduino `setup()` function. Example:

```C
void setup(void)
{
	// here, some jm_Scheduler variables initialized...
	
	Serial.begin(9600);
	while (!Serial)
	{
		// wait for USB Serial ready...
		
		yield();
	}
	
	// split long setup()...

	yield();
	
	// continue setup()...
}
```

> `cycle()` can't be invoked from inside a coroutine function.


### Good scheduling practices

- To guarantee a good scheduling of all tasks,
the execution time of each function must be as short as possible.

- Avoid Arduino `delay()` function, use **jm_Scheduler** `rearm()` method with appropriate arguments to split the coroutine in some serialized functions.

- Use same technical for long calculations.


### Changing of Timestamp

Here are some hacks that can be implemented by modifying the file **jm_Scheduler.h**.

- Another source for the _timestamp_ could be the [ms] read from the Arduino function `millis()`. 
- Gain speed during _timestamp_ comparison by shortening the size to 16bit.
- Obtain very long periodicity by implementing a 64bit _timestamp_.

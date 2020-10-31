# community version of nxrtos

This project is created to allow people to experience and practice a new RTOS -- nxRTOS. nxRTOS has brought some extraordinary extension to conventional RTOS, make it a great choice to firmware development on IoT device.

On conventional RTOSes, all Tasks has to be designed in a form of forever loop and never return, with each Task a chunk of RAM been assigned along for Task's stack, exclusively belong to the Task. This is kind of easy and simple arrangement, but result inefficiency on RAM usage. It causes a great difficult to deploy RTOS on RAM constrained platform.

With nxRTOS brought a new concept of ShortLive_Thread, which run to termination rather than forever loop. With ShortLive_Thread the stack space is only assigned when the ShortLive_Thread is running and higher priority of ShortLive_Thread can preempt lower priority of ShortLive_Thread, allocate its stack space on top the stack of preempted lower priority of ShortLive_Thread, aka stack on top of stack. This scheme makes a great improvement on efficiency of RAM usage, and so to fit many embedded platform very well, especially when suffering from low count of RAM. 

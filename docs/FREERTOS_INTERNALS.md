# How FreeRTOS Works Internally

## Kernel model

FreeRTOS divides an application into tasks. Each task owns a stack and is represented
by a Task Control Block (TCB). The TCB stores the saved stack pointer, priority, list
items, task name and optional bookkeeping information. Only one task executes at an
instant on a single-core STM32.

## Scheduler and task states

The scheduler selects the highest-priority task that is Ready. Tasks move among:

- **Running**: currently executing
- **Ready**: able to execute
- **Blocked**: waiting for time or an event
- **Suspended**: explicitly excluded from scheduling

FreeRTOS keeps ready lists by priority. Delayed tasks are held in delayed lists, and
tasks waiting for queues, semaphores or events are linked to object-specific event lists.
A blocked task consumes no CPU time.

## Tick and context switch

A periodic tick interrupt increments kernel time and makes expired delayed tasks Ready.
On Cortex-M, a typical port uses SysTick for timekeeping, PendSV for the context switch,
and SVC when starting the scheduler. During a switch, the current CPU context is saved
on its task stack, the TCB records the stack pointer, the scheduler selects another TCB,
and the new context is restored.

## Preemption

With preemption enabled, a newly Ready higher-priority task can replace the running task.
Equal-priority Ready tasks may time-slice. Priority must follow deadline and criticality:
safety and motor control should outrank telemetry, but interrupt handlers must stay short.

## Communication primitives

- **Queue**: copies fixed-size items into kernel-managed storage and can block producers or consumers.
- **Binary semaphore**: signals an event, often from ISR to task.
- **Mutex**: protects shared state and supports priority inheritance.
- **Task notification**: an efficient event/count/value attached directly to a task TCB.
- **Event group**: represents multiple Boolean system conditions.

This project uses a direct task notification for IMU data-ready, queues for measurements
and velocity commands, and event bits for sensor/link health.

## Interrupt interaction

An ISR should acknowledge hardware, capture minimal data, notify a task and exit. Only
`FromISR` APIs may be used from interrupts. The `higher_priority_task_woken` flag tells
the port whether an unblocked task should run immediately after ISR completion. Cortex-M
interrupt priorities must obey the port's configured API-call threshold.

## Deterministic timing

Periodic tasks use `vTaskDelayUntil()` instead of repeated `vTaskDelay()`. The former
anchors execution to an absolute schedule and limits accumulated drift. A 100 Hz control
task therefore uses a 10 ms period. Measure worst-case execution time and stack high-water
marks under realistic interrupt and communication load.

## Memory

This starter uses static task and queue allocation. Static allocation makes memory use
visible and prevents runtime heap exhaustion. Stack sizes are initial estimates and must
be measured. Enable stack-overflow hooks and assertions during development.

## Rover priority plan

```text
Priority 5  Safety task
Priority 4  Motor-control task
Priority 3  IMU and encoder tasks
Priority 2  micro-ROS communication
Priority 1  Telemetry
Priority 0  FreeRTOS idle task
```

## Internal execution example

```text
IMU interrupt occurs
  -> ISR clears hardware flag
  -> vTaskNotifyGiveFromISR wakes IMU task
  -> PendSV performs context switch if IMU outranks current task
  -> IMU task reads sample and writes queue
  -> communication task later publishes sample through micro-ROS
  -> IMU task blocks again without wasting CPU
```

## Core design rules

1. Block tasks instead of polling.
2. Keep ISRs short and use `FromISR` APIs.
3. Transfer ownership explicitly and avoid unprotected shared data.
4. Put deadlines and safe-state behavior before telemetry throughput.
5. Use static allocation where practical.
6. Measure execution time, latency, jitter and stack usage.
7. Use command timeouts and stop motors on invalid system state.
8. Test safety logic independently from hardware.

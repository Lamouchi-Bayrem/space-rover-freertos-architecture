/* Compile this file only in the STM32/FreeRTOS target. */
#include "rover_tasks.h"
#include "rover_config.h"
#include "rover_safety.h"
#include "rover_state.h"
#include "rover_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

#define EVENT_LINK_OK   (1u << 0)
#define EVENT_SENSOR_OK (1u << 1)

static StaticTask_t safety_tcb, control_tcb, imu_tcb, encoder_tcb, comms_tcb, telemetry_tcb;
static StackType_t safety_stack[384], control_stack[512], imu_stack[384], encoder_stack[384], comms_stack[1024], telemetry_stack[512];
static StaticQueue_t imu_queue_cb, encoder_queue_cb, command_queue_cb;
static uint8_t imu_queue_storage[ROVER_QUEUE_LENGTH * sizeof(rover_imu_t)];
static uint8_t encoder_queue_storage[ROVER_QUEUE_LENGTH * sizeof(rover_encoder_t)];
static uint8_t command_queue_storage[ROVER_QUEUE_LENGTH * sizeof(rover_command_t)];
static QueueHandle_t imu_queue, encoder_queue, command_queue;
static StaticEventGroup_t system_events_cb;
static EventGroupHandle_t system_events;
static TaskHandle_t imu_task_handle;

static void safety_task(void *arg) {
  (void)arg; TickType_t wake=xTaskGetTickCount();
  for (;;) {
    rover_status_t s=rover_state_get_status(); rover_command_t c=rover_state_get_command();
    const EventBits_t e=xEventGroupGetBits(system_events);
    s.battery_v=rover_hal_battery_voltage(); s.board_temp_c=rover_hal_board_temperature();
    s.fault_flags=rover_safety_evaluate(rover_hal_millis(), &c, s.battery_v, s.board_temp_c,
      (e&EVENT_SENSOR_OK)!=0u, (e&EVENT_LINK_OK)!=0u);
    if (rover_safety_requires_stop(s.fault_flags)) { s.mode=ROVER_MODE_SAFE; rover_hal_motor_stop(); }
    s.heartbeat++; rover_state_set_status(&s);
    vTaskDelayUntil(&wake, pdMS_TO_TICKS(ROVER_SAFETY_PERIOD_MS));
  }
}
static void control_task(void *arg) {
  (void)arg; TickType_t wake=xTaskGetTickCount(); rover_command_t c={0};
  for (;;) {
    while (xQueueReceive(command_queue,&c,0)==pdPASS) rover_state_set_command(&c);
    rover_status_t s=rover_state_get_status();
    if (s.mode==ROVER_MODE_ACTIVE && !rover_safety_requires_stop(s.fault_flags)) rover_hal_motor_command(c.linear_mps,c.angular_rps);
    else rover_hal_motor_stop();
    vTaskDelayUntil(&wake,pdMS_TO_TICKS(ROVER_CONTROL_PERIOD_MS));
  }
}
static void imu_task(void *arg) {
  (void)arg; rover_imu_t sample;
  for (;;) {
    ulTaskNotifyTake(pdTRUE,portMAX_DELAY);
    if (rover_hal_imu_read(&sample)) { xQueueOverwrite(imu_queue,&sample); xEventGroupSetBits(system_events,EVENT_SENSOR_OK); }
    else xEventGroupClearBits(system_events,EVENT_SENSOR_OK);
  }
}
static void encoder_task(void *arg) {
  (void)arg; TickType_t wake=xTaskGetTickCount(); rover_encoder_t sample;
  for (;;) {
    if (rover_hal_encoder_read(&sample)) xQueueOverwrite(encoder_queue,&sample);
    vTaskDelayUntil(&wake,pdMS_TO_TICKS(ROVER_CONTROL_PERIOD_MS));
  }
}
static void communications_task(void *arg) {
  (void)arg; rover_encoder_t encoder_sample;
  for (;;) {
    while (xQueueReceive(encoder_queue, &encoder_sample, 0) == pdPASS) {
      (void)encoder_sample;
      /* TODO: publish encoder samples through micro-ROS. */
    }
    /* TODO: execute micro-ROS executor, publish IMU queue and enqueue received commands. */
    xEventGroupSetBits(system_events,EVENT_LINK_OK);
    vTaskDelay(pdMS_TO_TICKS(5));
  }
}
static void telemetry_task(void *arg) {
  (void)arg; TickType_t wake=xTaskGetTickCount();
  for (;;) { rover_status_t s=rover_state_get_status(); rover_hal_debug_status(&s); vTaskDelayUntil(&wake,pdMS_TO_TICKS(ROVER_TELEMETRY_PERIOD_MS)); }
}
void rover_app_init(void) {
  rover_state_init(); rover_hal_init();
  imu_queue=xQueueCreateStatic(1,sizeof(rover_imu_t),imu_queue_storage,&imu_queue_cb);
  encoder_queue=xQueueCreateStatic(1,sizeof(rover_encoder_t),encoder_queue_storage,&encoder_queue_cb);
  command_queue=xQueueCreateStatic(ROVER_QUEUE_LENGTH,sizeof(rover_command_t),command_queue_storage,&command_queue_cb);
  system_events=xEventGroupCreateStatic(&system_events_cb);
}
void rover_create_tasks(void) {
  xTaskCreateStatic(safety_task,"Safety",384,0,5,safety_stack,&safety_tcb);
  xTaskCreateStatic(control_task,"Control",512,0,4,control_stack,&control_tcb);
  imu_task_handle=xTaskCreateStatic(imu_task,"IMU",384,0,3,imu_stack,&imu_tcb);
  xTaskCreateStatic(encoder_task,"Encoder",384,0,3,encoder_stack,&encoder_tcb);
  xTaskCreateStatic(communications_task,"microROS",1024,0,2,comms_stack,&comms_tcb);
  xTaskCreateStatic(telemetry_task,"Telemetry",512,0,1,telemetry_stack,&telemetry_tcb);
}
void rover_imu_data_ready_isr(void) {
  BaseType_t wake=pdFALSE; vTaskNotifyGiveFromISR(imu_task_handle,&wake); portYIELD_FROM_ISR(wake);
}

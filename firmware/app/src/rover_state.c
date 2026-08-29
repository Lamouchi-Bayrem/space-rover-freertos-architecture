#include "rover_state.h"

#include "FreeRTOS.h"
#include "semphr.h"


/*
 *==========================================================================*
 *                         PRIVATE STATE                                    *
 *==========================================================================*
 */

static rover_command_t command_state;
static rover_status_t status_state;


/*
 * Mutex protecting the shared rover state.
 *
 * The mutex prevents multiple tasks from accessing/modifying
 * shared state simultaneously.
 */
static StaticSemaphore_t state_mutex_cb;
static SemaphoreHandle_t state_mutex;


/*
 *==========================================================================*
 *                         PRIVATE HELPERS                                  *
 *==========================================================================*
 */

static bool rover_state_lock(void)
{
    return (xSemaphoreTake(
        state_mutex,
        pdMS_TO_TICKS(ROVER_STATE_MUTEX_TIMEOUT_MS)
    ) == pdTRUE);
}


static void rover_state_unlock(void)
{
    (void)xSemaphoreGive(state_mutex);
}


/*
 *==========================================================================*
 *                         INITIALIZATION                                   *
 *==========================================================================*
 */

void rover_state_init(void)
{
    /*
     * Create mutex before accessing shared state.
     */
    state_mutex = xSemaphoreCreateMutexStatic(
        &state_mutex_cb
    );

    /*
     * Initialize command state.
     */
    command_state.linear_mps = 0.0F;
    command_state.angular_rps = 0.0F;
    command_state.received_at_ms = 0U;


    /*
     * Initialize status state.
     */
    status_state.mode = ROVER_MODE_BOOT;
    status_state.battery_v = 0.0F;
    status_state.board_temp_c = 0.0F;
    status_state.fault_flags = 0UL;
    status_state.heartbeat = 0UL;
}


/*
 *==========================================================================*
 *                         COMMAND ACCESS                                   *
 *==========================================================================*
 */

void rover_state_set_command(
    const rover_command_t *command)
{
    if (command != NULL)
    {
        if (rover_state_lock())
        {
            command_state = *command;

            rover_state_unlock();
        }
    }
}


rover_command_t rover_state_get_command(void)
{
    rover_command_t copy;

    copy.linear_mps = 0.0F;
    copy.angular_rps = 0.0F;
    copy.received_at_ms = 0U;

    if (rover_state_lock())
    {
        copy = command_state;

        rover_state_unlock();
    }

    return copy;
}


/*
 *==========================================================================*
 *                          STATUS ACCESS                                   *
 *==========================================================================*
 */

void rover_state_set_status(
    const rover_status_t *status)
{
    if (status != NULL)
    {
        if (rover_state_lock())
        {
            status_state = *status;

            rover_state_unlock();
        }
    }
}


rover_status_t rover_state_get_status(void)
{
    rover_status_t copy;

    copy.mode = ROVER_MODE_BOOT;
    copy.battery_v = 0.0F;
    copy.board_temp_c = 0.0F;
    copy.fault_flags = 0UL;
    copy.heartbeat = 0UL;

    if (rover_state_lock())
    {
        copy = status_state;

        rover_state_unlock();
    }

    return copy;
}

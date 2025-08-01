/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "platform.h"

#include "fc/init.h"

#include "scheduler/scheduler.h"
#include "common/time.h"

void run(void);

int main(int argc, char * argv[])
{
#ifdef SIMULATOR_BUILD
    targetParseArgs(argc, argv);
#else
    UNUSED(argc);
    UNUSED(argv);
#endif
    init();

    run();

    return 0;
}

void FAST_CODE run(void)
{
    static int sched_counter = 0;
    while (true) {
        scheduler();
        sched_counter++;
        if (sched_counter % 10000 == 0) {
            printf("[DEBUG] Scheduler running - cycles: %d\n", sched_counter);
        }
        
        // SITL: Execute REALTIME tasks that are normally called from hardware interrupts
#ifdef SIMULATOR_BUILD
        static uint64_t lastRealtimeExecution = 0;
        uint64_t currentTime = micros64();
        // Use TASK_GYROPID_DESIRED_PERIOD for correct frequency
        if (currentTime - lastRealtimeExecution >= TASK_GYROPID_DESIRED_PERIOD) {
            extern void taskGyroSample(timeUs_t currentTimeUs);
            extern void taskFiltering(timeUs_t currentTimeUs);
            extern void taskMainPidLoop(timeUs_t currentTimeUs);
            
            taskGyroSample((timeUs_t)currentTime);
            taskFiltering((timeUs_t)currentTime);
            taskMainPidLoop((timeUs_t)currentTime);
            
            lastRealtimeExecution = currentTime;
        }
        
        delayMicroseconds_real(50); // max rate 20kHz
#endif
    }
}

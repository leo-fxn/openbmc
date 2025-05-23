/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This program file is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program in a file named COPYING; if not, write to the
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 */

#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <getopt.h>
#include <openbmc/log.h>
#include <openbmc/misc-utils.h>

// Polling mode, default is interval polling
// 0 : interval polling (default)
// 1 : event interrupt polling (support only GPIO)
static char gEventPoll = 0;

// Polling interval in milliseconds (use in interval polling mode)
static unsigned int gPollSleep = 500;

// Input Leakage value logic
// 0 : 0 alert, 1 normal
// 1 : 0 normal, 1 alert (default)
static char gInputAlertValue = 1;

// Output setting alert signal logic
// 0 : 0 alert, 1 normal
// 1 : 0 normal, 1 alert (default)
static char gOutputAlertValue = 1;

int last_state = -1;

/*
 * @brief Function to sleep for given milliseconds
 *
 * @param msecs Time in milliseconds to sleep.
 */
void msleep(unsigned int msecs) {
    if (msecs >= 1000)
      sleep(msecs / 1000);
    usleep((msecs % 1000) * 1000);
}

/*
 * @brief Function to action on leakage status change
 *
 * When detecting an alert, write the alert value to the `alert_output_path` file.
 * When the state changes, log the new state.
 *
 * @param current_state Current state of the leakage presence.
 * @param alert_output_path Path to the output alert file.
 */
char state_handler(int current_state, const char *alert_output_path) {
    char buf[8];
    char cmd[128];
    if (current_state == gInputAlertValue) {
        sprintf(buf, "%d", gOutputAlertValue);
        device_write_buff(alert_output_path, buf);
    }
    if (current_state != last_state) {
        OBMC_INFO("Leakage state changed to %s",
            current_state == gInputAlertValue ? "alert" : "normal");
        // use logger with crit level to keep the log to /mnt/data/logfile
        snprintf(cmd, 128, "/usr/bin/logger -t leakage-mond -p daemon.crit "
            "\"Leakage state changed to %s\"",
            current_state == gInputAlertValue ? "alert" : "normal");
        system(cmd);
        last_state = current_state;
    }
    return 0;
}

/*
 * @brief Function to poll the leakage status file and set the alert status file
 *
 * This function continuously monitors the leakage presence status file specified
 * by `leakage_presence_status_path`. When a change in the leakage presence state is
 * detected, write the alert value to `alert_output_path` until the platform power cycle.
 *
 * @param leakage_status_path Path to the leakage status file.
 * @param alert_output_path Path to the output alert file.
 */
void leakage_interval_poll(const char *leakage_status_path, const char *alert_output_path) {
    int rc = 0;
    int current_state;

    OBMC_INFO("Interval polling %d ms.", gPollSleep);
    while (1) {
        rc = device_read(leakage_status_path, &current_state);
        if (rc) {
            OBMC_ERROR(rc, "Error reading from leakage status file %s : %s\n",
                leakage_status_path , strerror(rc));
            return;
        }
        state_handler(current_state, alert_output_path);
        msleep(gPollSleep);
    }
}

/*
 * @brief Function to poll the leakage status file and set the alert status file
 *
 * This function continuously monitors the leakage presence status file specified
 * by `leakage_presence_status_path`. When a change in the leakage presence state is
 * detected, write the alert value to `alert_output_path` until the platform power cycle.
 *
 * @param leakage_status_path Path to the leakage status file.
 * @param alert_output_path Path to the output alert file.
 */
 void leakage_poll_event(const char *leakage_status_path, const char *alert_output_path) {
    int rc = 0;
    int current_state;
    char buf[8];

    OBMC_INFO("Polling for event");
    struct pollfd pfd;
    int fd = open(leakage_status_path, O_RDONLY);
    if (fd < 0) {
        OBMC_ERROR(errno, "Failed to open GPIO value file: %s", strerror(errno));
        return;
    }
    pfd.fd = fd;
    pfd.events = POLLPRI | POLLERR;

    // Initial read to clear any pending interrupts
    lseek(fd, 0, SEEK_SET);
    rc = read(fd, buf, sizeof(buf));
    if (rc < 0) {
        OBMC_ERROR(errno, "Failed to read GPIO value file: %s", strerror(errno));
        close(fd);
        return;
    }

    while (1) {
        rc = poll(&pfd, 1, -1);
        if (rc < 0) {
            OBMC_ERROR(errno, "Poll error: %s", strerror(errno));
            close(fd);
            return;
        }

        if (pfd.revents & POLLPRI) {
            rc = device_read(leakage_status_path, &current_state);
            if (rc) {
                OBMC_ERROR(rc, "Error reading from leakage status file %s : %s\n",
                    leakage_status_path , strerror(rc));
                return;
            }
            // Clear the interrupt
            lseek(fd, 0, SEEK_SET);

            state_handler(current_state, alert_output_path);
        }
    }
    close(fd);
    return;
}

void usage(char const* app_name) {
    printf("Usage: %s [-hio] [-T Time interval(ms)] -I <Input path> -O <Output path>\n", app_name);
    printf("   options: \n");
    printf("      -h: display usage\n");
    printf("      -T: Time polling interval (ms) (default: 500 ms)\n");
    printf("      -e: poll event instead of interval polling\n");
    printf("      -I: leakage status path file\n");
    printf("      -i: inverse status value logic (0 alert, 1 normal)\n");
    printf("      -O: alert output path file\n");
    printf("      -o: inverse output value logic (0 alert, 1 normal)\n");
}

int main(int argc, char *argv[]) {
    int opt, rc = 0;
    char *input_status_path_file = NULL;
    char *output_alert_path_file = NULL;
    char cmd[128];
    while ((opt = getopt(argc, argv, "heioI:O:T:")) != -1) {
        switch (opt) {
            case 'h':
                usage(argv[0]);
                return 0;
            case 'i':
                gInputAlertValue = 0;
                break;
            case 'o':
                gOutputAlertValue = 0;
                break;
            case 'I':
                input_status_path_file = optarg;
                break;
            case 'O':
                output_alert_path_file = optarg;
                break;
            case 'T':
                gPollSleep = strtoul(optarg, NULL, 10);
                if (gPollSleep == 0) {
                    OBMC_ERROR(LOG_ERR, "Invalid poll interval: %s", optarg);
                    return 1;
                }
                break;
            case 'e':
                gEventPoll = 1;
                break;
            default:
                OBMC_ERROR(LOG_ERR, " invalid options [%c]", opt);
                usage(argv[0]);
                return 1;
        }
    }

    openlog(argv[0], LOG_PID, LOG_DAEMON);
    OBMC_INFO("Starting Leakage-monitor");
    if (access(input_status_path_file, F_OK) == -1 ||
        access(output_alert_path_file, F_OK) == -1) {
        OBMC_ERROR(LOG_ERR, "Leakage status path file does not exist");
        return 1;
    }

    rc = device_read(input_status_path_file, &last_state);
    if (rc) {
        OBMC_ERROR(rc, "Error reading from leakage status file %s : %s\n",
            input_status_path_file , strerror(rc));
            return 1;
    }
    OBMC_INFO("Initial leakage state: %s",
        last_state == gInputAlertValue ? "alert" : "normal");
    // use logger with crit level to keep the log to /mnt/data/logfile
    snprintf(cmd, 128, "/usr/bin/logger -t leakage-mond -p daemon.crit "
        "\"Initial leakage state: %s\"",
        last_state == gInputAlertValue ? "alert" : "normal");
    system(cmd);
    state_handler(last_state, output_alert_path_file);

    if (gEventPoll) {
        leakage_poll_event(input_status_path_file, output_alert_path_file);
    } else {
        leakage_interval_poll(input_status_path_file, output_alert_path_file);
    }
    return 0;
}

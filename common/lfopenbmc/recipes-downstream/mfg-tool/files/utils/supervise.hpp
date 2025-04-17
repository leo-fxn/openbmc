#pragma once

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <phosphor-logging/lg2.hpp>

#include <functional>

namespace mfg_tool
{
PHOSPHOR_LOG2_USING;

static pid_t pid;
static int was_killed = 0;

void alarmHandler(int signum)
{
    was_killed = 1;
    kill(pid, SIGTERM);
    error("Alarm signal {SIGNUM} received. Child process timed out", "SIGNUM",
          signum);
}

int supervise(std::function<int()> fn, int timeout, int retries)
{
    int es = 0;
    for (int attempt = 0; attempt <= retries; attempt++)
    {
        was_killed = 0;
        pid = fork();
        if (pid == 0)
        {
            exit(fn());
        }
        else if (pid > 0)
        {
            signal(SIGALRM, alarmHandler);
            alarm(timeout);
            int status;
            wait(&status);
            es = WEXITSTATUS(status);
            alarm(0);
            if (was_killed)
            {
                continue;
            }
            if (es == 0)
            {
                return es;
            }
        }
    }
    // If the last attempt was killed by the timeout, then we
    // return 137 to mimic the returncode from the timeout binary
    if (was_killed)
    {
        exit(137);
    }
    return es;
}
} // namespace mfg_tool

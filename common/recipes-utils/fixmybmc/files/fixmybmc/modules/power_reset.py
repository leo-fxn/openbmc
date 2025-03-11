import os

from datetime import datetime, timedelta

from pathlib import Path

from fixmybmc.bmccheck import bmcCheck
from fixmybmc.status import Warning

logfile = Path("/mnt/data/logfile")
date_format = "%Y %b %d %H:%M:%S"


def get_recent_logs(file_path: Path, time_format: str, days_ago: int = 7) -> list[str]:
    cutoff_date = datetime.now() - timedelta(days=days_ago)
    time_str_len = len(datetime.now().strftime(time_format))

    recent_logs = []
    with open(file_path, "rb") as f:
        f.seek(0, os.SEEK_END)
        pos = f.tell()

        while pos > 0:
            pos -= 1
            f.seek(pos)
            char = f.read(1)

            if char == b"\n" or pos == 0:
                if pos > 0:
                    f.seek(pos + 1)
                line = f.readline().decode().strip()

                try:
                    line_date = datetime.strptime(line[:time_str_len], time_format)
                    if line_date >= cutoff_date:
                        recent_logs.append(line)
                except ValueError:
                    continue
    return recent_logs


@bmcCheck
def recent_power_reset():
    """
    Check if a power reset has been performed recently (in last 7 days)
    """
    recent_logs = get_recent_logs(logfile, date_format, days_ago=7)
    power_reset_logs = [
        log for log in recent_logs if "user.crit" in log and "Power" in log
    ]
    if len(power_reset_logs) == 0:
        return None
    return Warning(
        description=(
            "Power reset has been performed recently:\n" + "\n".join(power_reset_logs)
        )
    )

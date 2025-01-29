#!/usr/bin/env python3
#
# Copyright (c) Meta Platforms, Inc. and affiliates. (http://www.meta.com)
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA
#

from common.base_cpu_utilization_test import BaseCpuUtilizationTest


class CpuUtilizationTest(BaseCpuUtilizationTest):
    def init_cpu_variables(self):
        self.expected_cpu_utilization = 10

    def skip_cpu_utilization_processes(self):
        self.skip_processes = [
            "watchdog",
            "mTerm",
            "rsyslogd",
            "getty",
            "rest.py",
            "top",
            "awk",
        ]

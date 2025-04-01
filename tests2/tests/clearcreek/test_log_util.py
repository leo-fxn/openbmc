#!/usr/bin/env python3
#
# Copyright 2018-present Facebook. All Rights Reserved.
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
import unittest

from common.base_log_util_test import BaseLogUtilTest

FRU_IDS = {
    "mb": 1,
    "bsm": 2,
    "pdb": 3,
    "carrier1": 4,
    "carrier2": 5,
    "fio": 6,
    "nic0": 7,
    "nic1": 8,
    "nic2": 9,
    "nic3": 10,
    "nic4": 11,
    "nic5": 12,
    "nic6": 13,
    "nic7": 14,
}


class AllLogUtilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "all"

    def set_fru_id(self):
        """
        for spacific FRU need to override this function
        to define FRU_ID with fru id number by integer type
        """
        self.FRU_ID = FRU_IDS.get(self.FRU, None)


class MbLogUtilTest(AllLogUtilTest):
    FRU = "mb"


class Carrier1LogUtilTest(AllLogUtilTest):
    FRU = "carrier1"


class Carrier2LogUtilTest(AllLogUtilTest):
    FRU = "carrier2"


class FioLogUtilTest(AllLogUtilTest):
    FRU = "fio"


class Nic0LogUtilTest(AllLogUtilTest):
    FRU = "nic0"


class Nic1LogUtilTest(AllLogUtilTest):
    FRU = "nic1"


class Nic2LogUtilTest(AllLogUtilTest):
    FRU = "nic2"


class Nic3LogUtilTest(AllLogUtilTest):
    FRU = "nic3"


class Nic4LogUtilTest(AllLogUtilTest):
    FRU = "nic4"


class Nic5LogUtilTest(AllLogUtilTest):
    FRU = "nic5"


class Nic6LogUtilTest(AllLogUtilTest):
    FRU = "nic6"


class Nic7LogUtilTest(AllLogUtilTest):
    FRU = "nic7"


class BsmLogUtilTest(AllLogUtilTest):
    FRU = "bsm"


class PdbLogUtilTest(AllLogUtilTest):
    FRU = "pdb"


class SysLogUtilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "sys"

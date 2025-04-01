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
    "pdb": 2,
    "bsm": 3,
    "asic0": 4,
    "asic1": 5,
    "asic2": 6,
    "asic3": 7,
    "asic4": 8,
    "asic5": 9,
    "asic6": 10,
    "asic7": 11,
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


class Asic0LogUtilTest(AllLogUtilTest):
    FRU = "asic0"


class Asic1LogUtilTest(AllLogUtilTest):
    FRU = "asic1"


class Asic2LogUtilTest(AllLogUtilTest):
    FRU = "asic2"


class Asic3LogUtilTest(AllLogUtilTest):
    FRU = "asic3"


class Asic4LogUtilTest(AllLogUtilTest):
    FRU = "asic4"


class Asic5LogUtilTest(AllLogUtilTest):
    FRU = "asic5"


class Asic6LogUtilTest(AllLogUtilTest):
    FRU = "asic6"


class Asic7LogUtilTest(AllLogUtilTest):
    FRU = "asic7"


class BsmLogUtilTest(AllLogUtilTest):
    FRU = "bsm"


class PdbLogUtilTest(AllLogUtilTest):
    FRU = "pdb"


class SysLogUtilTest(BaseLogUtilTest, unittest.TestCase):
    FRU = "sys"

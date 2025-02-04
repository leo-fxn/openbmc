#!/usr/bin/env python3
# Copyright (c) Facebook, Inc. and its affiliates.

# This software may be used and distributed according to the terms of the
# GNU General Public License version 2.
#

import unittest

from common.base_interface_test import CommonInterfaceTest
from utils.cit_logger import Logger
from utils.test_utils import qemu_check


"""
Tests eth0 v4 interface
"""


class InterfaceTest(CommonInterfaceTest, unittest.TestCase):
    @unittest.skipIf(qemu_check(), "test env is QEMU, skipped")
    def test_usb0_v6_interface(self):
        """
        Tests usb0 v6 interface
        """
        self.set_ifname("usb0")
        Logger.log_testname(self._testMethodName)
        self.assertEqual(self.ping_v6(), 0, "Ping test for usb0 v6 failed")

    def test_eth0_4088_v6_interface(self):
        """
        Tests eth0 v6 interface
        """
        self.set_ifname("eth0.4088")
        Logger.log_testname(self._testMethodName)
        self.assertEqual(self.ping_v6(), 0, "Ping test for eth0.4088 v6 failed")

    def test_eth0_4088_v6_interface_link_local(self):
        """
        Tests eth0 v6 interface
        """
        self.set_ifname("eth0.4088")
        Logger.log_testname(self._testMethodName)
        self.assertEqual(
            self.ping_v6_link_local(),
            0,
            "Ping test for eth0.4088 v6 over link local failed",
        )

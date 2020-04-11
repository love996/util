#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from util import run_cmd


class TestDict(unittest.TestCase):

    def setUp(self):
        print("testUp ...")

    def tearDown(self):
        print("testDown ...")

    def test_cmd(self):
        self.assertIsNotNone(run_cmd(['ping', 'www.baidu.com', '-c', '3']))
        self.assertIsNotNone(run_cmd("ping www.baidu.com -c 3"))


if __name__ == '__main__':
    unittest.main()

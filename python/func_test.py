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
        ret = run_cmd(['ping', 'www.baidu.com', '-c', '3'])
        self.assertIsNotNone(ret)
        retcode = ret[-1]
        print("retcode:", retcode)
        ret = run_cmd("ping www.baidu.com1 -c 3")
        self.assertIsNotNone(ret)
        retcode = ret[-1]
        print("retcode:", retcode)
        # run_cmd("https://github.com/yixuehan/study.git", ["--depth=1")


if __name__ == '__main__':
    unittest.main()

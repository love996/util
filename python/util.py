#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json5
import subprocess


def readjson(filename):
    fp = open(filename)
    j = json5.load(fp)
    return j


def assert_unique(l, value):
    assert isinstance(l, list)
    if value in l:
        print("value [%s] exist" % value)
        assert False


def abs_path(path):
    if path[0] == '~':
        path = os.environ['HOME'] + path[1:]
    return os.path.abspath(path)


def assert_file(filename):
    if not os.path.exists(filename):
        print("file [%s] not exist" % (filename))
        assert False


def gen_upper_camel(name):
    if -1 == name.find('_'):
        name = name[0].upper() + name[1:]
        return name
    names = name.split('_')
    name = ""
    for n in names:
        name += n.title()
    return name


def gen_lower_camel(name):
    name = gen_upper_camel(name)
    return name[0].lower() + name[1:]


def gen_underline_name(name):
    res = name[0].lower()
    for c in name[1:]:
        if c.upper() == c and c.isalpha():
            res = res + '_' + c.lower()
        else:
            res = res + c
    return res


def get_base_type(value):
    if isinstance(value, list):
        return get_base_type(value[0])
    return type(value)


# 对subprocess.Popen的封装，返回执行结果
def run_cmd(cmd, show=True, code="utf8"):
    # print(cmd)
    shell = not isinstance(cmd, list)
    # import pdb
    # pdb.set_trace()
    print("shell:", shell)
    process = subprocess.Popen(cmd, shell=shell, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    lines = []
    while process.poll() is None:
        line = process.stdout.readline()
        # line = line.strip()
        if line:
            if show:
                print(line.decode(code, 'ignore'), end="")
            lines.append(line)
        else:
            break
    return lines

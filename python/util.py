#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import json5
import subprocess
import hashlib
import shutil


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
    if not os.path.isfile(filename):
        print("file [%s] not exist" % (filename))
        assert False


def assert_dir(dirname):
    if not os.path.isdir(dirname):
        print("dir [%s] not exist" % (dirname))
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
    return __run_cmd(cmd, show, code)


def __run_cmd(cmd, show=True, code="utf8"):
    # print("cmd:", cmd)
    shell = not isinstance(cmd, list)
    # import pdb
    # pdb.set_trace()
    # print("shell:", shell)
    process = subprocess.Popen(
            cmd,
            shell=shell,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE)
    lines = []
    retcode = 0
    while process.poll() is None:
        line = process.stdout.readline()
        if not line:
            line = process.stderr.readline()
            if line:
                e = line.decode(code, 'ignore')
                print("error:[%s]" % e)
                retcode = 1
        # line = line.strip()
        if line:
            line = line.decode(code, 'ignore')
            if show:
                print(line, end="")
            lines.append(line)
        else:
            # print("break")
            break
    # None
    # print("returncode:", process.returncode)
    lines.append(retcode)
    process.stdout.close()
    process.stderr.close()
    return lines


def unique(obj_list):
    r = []
    for obj in obj_list:
        if obj not in r:
            r.append(obj)
    return r


def md5(filename):
    if os.path.isfile(filename):
        md5obj = hashlib.md5()
        maxbuf = 8192
        f = open(filename, 'rb')
        while True:
            buf = f.read(maxbuf)
            if not buf:
                break
            md5obj.update(buf)
        f.close()
        h = md5obj.hexdigest()
        return str(h).upper()
    print("[%s] is not a file" % filename)
    assert False


def copy_file(src_file, dst_file):
    assert_file(src_file)
    if not os.path.exists(dst_file):
        dirname = os.path.dirname(dst_file)
        if not os.path.exists(dirname):
            os.makedirs(dirname)
        shutil.copyfile(src_file, dst_file)
        return
    else:
        assert_file(dst_file)
    m1 = md5(src_file)
    m2 = md5(dst_file)
    print("[%s][%s]:[%s][%s]" % (src_file, m1, dst_file, m2))
    assert m1 and m2
    if m1 != m2:
        shutil.copyfile(src_file, dst_file)


def copy_dir(src_dir, dst_dir):
    assert_dir(src_dir)
    if not os.path.exists(dst_dir):
        os.makedirs(dst_dir)
    for f in os.listdir(src_dir):
        src_file = os.path.join(src_dir, f)
        dst_file = os.path.join(dst_dir, f)
        if os.path.isfile(src_file):
            copy_file(src_file, dst_file)
        elif os.path.isdir(src_file):
            copy_dir(src_file, dst_file)
        else:
            print("error file [%s]" % (f))
            assert False

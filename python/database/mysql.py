#!/usr/bin/env python3
# -*- coding: utf-8 -*-


class Mysql(DataBase):
    def __init__(self):
        DataBase.__init__(self)
        self.__keywords = []
        self.__keywords += ""



keywords = [
        # 类型
        "char",
        "varchar",
        "int",
        "smallint",

        # 特殊关键字
        "primary key",
        "foreign key references",
        "unique key"
]


def is_keyword(word):
    # print("is_keyword:", word)
    return word.strip() in keywords


def contain_keyword(text):
    text = text.lower()
    words = text.split()
    assert len(words) > 1
    if is_keyword(words[0]):
        return True
    if is_keyword(' '.join(words[:2])):
        return True
    return False

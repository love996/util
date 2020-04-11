#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from util.python.database import mysql


class DBFactory:
    def __init__(self, database):
        if database == 'mysql':
            self.__obj = mysql.Mysql()

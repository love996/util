#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import select
import socket


packet_length = 1400
cmd_length = 10
session_id_length = 10


def extern_length(data, length):
    data = str(data).encode()
    data += bytes(length - len(data))
    return data


class SSHSession:
    def __init__(self, client_id, session_id, nat_address):
        self.__client_id = client_id
        self.__session_id = session_id
        self.__socket = None
        self.__nat_address = nat_address

    def __eq__(self, o):
        return self.__socket == o.__socket

    @property
    def id(self):
        return extern_length(self.__session_id, session_id_length)

    @property
    def socket(self):
        return self.__socket

    def send(self, data):
        self.__socket.send(data)

    def recv(self):
        pass

    def close(self):
        self.__socket.close()

    def connect(self, ip, port):
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__socket.connect(self.__nat_address)


class NatClient:
    def __init__(self, server_ip, server_port, nat_ip, nat_port):
        self.__server_ip = server_ip
        self.__server_port = server_port
        self.__nat_ip = nat_ip
        self.__nat_port = nat_port
        self.__epoll = select.epoll()
        self.__fd_sockets = {}
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__id_sessions = {}
        self.__socket_sessions = {}
        self.__buffer = bytes()
        self.__length = 0
        self.__left = 0

    def register(self, sock):
        self.__epoll.register(sock.fileno(), select.EPOLLIN)
        self.__fd_sockets[sock.fileno()] = sock

    def connect(self):
        address = (self.__server_ip, self.__server_port)
        self.__socket.connect(address)
        self.register(self.__socket)

    def read(self):
        if self.__buffer:
            data = self.__socket.recv(self.__left)
            self.__left -= len(data)
            self.__buffer += data
            if self.__left == 0:
                self.parse_msg()
        else:
            self.__length = int(self.__socket.recv(4))
            self.__left = self.__length

    def parse_msg(self):
        session_id = int(self.__buffer[0:session_id_length])
        next_pos = session_id_length

        cmd = str(self.__buffer[next_pos:next_pos + cmd_length])
        next_pos += cmd_length

        msg = self.__buffer[next_pos:]
        self.execute(session_id, cmd, msg)

    def execute(self, session_id, cmd, data):
        if cmd == 'connect':
            self.__socket.connect(self.__nat_address)
            session = SSHSession(self.__last_session_id + 1, self.__nat_address)
            session.connect()
            self.register(session.socket)
            self.__sock_sessions[session.socket] = session
            return

        if cmd == 'close':
            session.close()
        elif cmd == 'data':
            session.send(data)
        self.__buffer = bytes()
        self.__length = 0

    def loop(self):
        while True:
            events = self.__epoll.poll()
            if not events:
                print("error should not return")
                continue

            for fd, event in events:
                sock = self.__fd_to_socket[fd]
                if sock == self.__socket:
                    self.read(sock)
                else:
                    self.write(sock)

    def write(self, sock):
        session = self.__sock_sessions[sock]
        session_id = session.id
        cmd = extern_length('data', cmd_length)
        data = sock.read(1400)
        length = len(session_id) + len(cmd) + len(data)
        length = extern_length(length, 4)
        self.__socket.send(length + session_id + cmd + data)

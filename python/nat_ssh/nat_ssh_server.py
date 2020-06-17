#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import select
import socket


packet_length = 1400
cmd_length = 10
session_id_length = 10
client_id_length = 10


def extern_length(data, length):
    data = str(data).encode()
    data += bytes(length - len(data))
    return data


def parse_msg(data):
    session_id = int(data[0:session_id_length])
    next_pos = session_id_length

    cmd = str(data[next_pos:next_pos + cmd_length])
    next_pos += cmd_length

    msg = data[next_pos:]
    return session_id, cmd, msg


class NatSession:
    def __init__(self, nat_socket, request_socket, client_id, session_id):
        self.__nat_socket = nat_socket
        self.__request_socket = request_socket
        self.__client_id = client_id
        self.__session_id = session_id
        # self.__nat_address = nat_address

    @setter
    def nat_socket

    def set_id(self, client_id, session_id):
        self.__client_id = client_id
        self.__session_id = session_id

    def __eq__(self, o):
        return self.__socket == o.__socket

    def read_data(self):
        data = self.__socket.recv(packet_length)
        self.transfer_to_nat('data', data)

    def transfer_to_nat(self, cmd, data):
        print('[%s][%s][%s]' % (self.__session_id, cmd, data))
        session_id = extern_length(self.__session_id, session_id_length)
        cmd = extern_length(cmd, cmd_length)
        data = self.__nat_socket.read(1400)
        length = len(session_id) + len(cmd) + len(data)
        length = extern_length(length, 4)
        self.__nat_socket.send(length + session_id + cmd + data)

    @property
    def client_id(self):
        return extern_length(self.__client_id, client_id_length)

    @property
    def session_id(self):
        return extern_length(self.__session_id, session_id_length)

    @property
    def nat_socket(self):
        return self.__nat_socket

    @property
    def request_socket(self):
        return self.__request_socket

    def send(self, data):
        self.__socket.send(data)

    def recv(self):
        pass

    def close(self):
        self.__socket.close()

    def connect(self, ip, port):
        self.__socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__socket.connect(self.__nat_address)


class NatServer:
    def __init__(self, server_ip, server_port):
        self.__server_ip = server_ip
        self.__server_port = server_port
        self.__epoll = select.epoll()
        self.__fd_sockets = {}
        self.__listen_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.__id_sessions = {}
        self.__socket_sessions = {}
        self.__request_sessions = {}
        self.__buffer = bytes()
        self.__length = 0
        self.__left = 0

    def register(self, sock):
        self.__epoll.register(sock.fileno(), select.EPOLLIN)
        self.__fd_sockets[sock.fileno()] = sock

    def bind_listen(self):
        address = (self.__server_ip, self.__server_port)
        self.__listen_socket.bind(address)
        self.__listen_socket.listen(1024)
        self.register(self.__listen_socket)

    def request(self, sock):
        session = self.__request_sessions[sock]

    def parse_msg(self, client_socket):
        session_id = int(self.__buffer[0:session_id_length])
        next_pos = session_id_length

        cmd = str(self.__buffer[next_pos:next_pos + cmd_length])
        next_pos += cmd_length

        msg = self.__buffer[next_pos:]
        self.parse_client(client_socket, session_id, cmd, msg)

    def parse_client(self, client_socket, session_id, cmd, data):
        if cmd == 'login':
            session = NatSession(client_socket,  data, session_id)
            self.__sock_sessions[session.socket] = session
            return

        if cmd == 'close':
            pass
        elif cmd == 'data':
            session.send(data)
        self.__buffer = bytes()
        self.__length = 0

    def add_session(self, client_socket, client_addr):

    def loop(self):
        while True:
            events = self.__epoll.poll()
            if not events:
                print("error should not return")
                continue

            for fd, event in events:
                sock = self.__fd_to_socket[fd]
                # client connect
                if sock == self.__listen_socket:
                    session = NatSession(request_socket = sock)
                    self.__socket_sessions[client_socket] = session
                    client_socket, client_addr = sock.accept()
                    self.register(client_socket)
                # client msg
                else:
                    self.request(sock)

    def write(self, sock):
        session = self.__sock_sessions[sock]
        session_id = session.id
        cmd = extern_length('data', cmd_length)
        data = sock.read(1400)
        length = len(session_id) + len(cmd) + len(data)
        length = extern_length(length, 4)
        self.__socket.send(length + session_id + cmd + data)

#!/usr/bin/python
from server import Config, HTTPRequestHandler, WebSocketServer
from server.build import builder
import os

os.chdir('editor')
#HTTPRequestHandler.StartServer()
WebSocketServer.StartServer()

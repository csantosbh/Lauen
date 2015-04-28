#!/usr/bin/python
from server import Config, HTTPRequestHandler, WebSocketServer
import os

os.chdir('editor')
#HTTPRequestHandler.StartServer()
WebSocketServer.StartServer()

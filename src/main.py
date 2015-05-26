#!/usr/bin/python
from server import Config, HTTPRequestHandler, WebSocketServer
from server.build import builder
from server.io import IOEventHandler
from server.project import ProjectEventHandler
import os

os.chdir('editor')
#HTTPRequestHandler.StartServer()
WebSocketServer.StartServer()

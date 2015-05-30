#!/usr/bin/python
import os, sys

from server import Config, HTTPRequestHandler, WebSocketServer
from server.build import builder
from server.io import IOEventHandler
from server.project import ProjectEventHandler

os.chdir(os.path.dirname(os.path.realpath(sys.argv[0]))+'/editor')
#HTTPRequestHandler.StartServer()
WebSocketServer.StartServer()

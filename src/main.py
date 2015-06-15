#!/usr/bin/python
import os, sys

from server import Config, HTTPServer, WSServer
from server.build import BuildEventHandler
from server.io import IOEventHandler
from server.project import ProjectEventHandler
from server.components import DefaultComponentManager

os.chdir(os.path.dirname(os.path.realpath(sys.argv[0]))+'/editor')

WSServer.serve(blocking = False)
HTTPServer.serve()

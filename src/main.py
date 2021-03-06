#!/usr/bin/python
import os, sys

from server import Config, HTTPServer, WSServer
from server.build import BuildEventHandler
from server.io import IOEventHandler
from server.project import Project, ProjectEventHandler, AssetFolderWatcher
from server.components import DefaultComponentManager


try:
    os.chdir(os.path.dirname(os.path.realpath(sys.argv[0]))+'/editor')
    Project.initialize()
    WSServer.serve(blocking = False)
    HTTPServer.serve()
except KeyboardInterrupt:
    exit()
    pass

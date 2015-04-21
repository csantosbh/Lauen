#!/usr/bin/python
from server import Config, HTTPRequestHandler
import os

print Config.get('server', 'port')
os.chdir('editor')
HTTPRequestHandler.StartServer()

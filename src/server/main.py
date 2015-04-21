#!/usr/bin/python
import Config
import HTTPRequestHandler
import os

print Config.get('server', 'port')
os.chdir('editor')
HTTPRequestHandler.StartServer()

from gevent import monkey; monkey.patch_all()

import gevent
import gevent.pywsgi

from ws4py.server.geventserver import WebSocketWSGIApplication, WebSocketWSGIHandler, WSGIServer
from ws4py.websocket import EchoWebSocket

class ClientManager(object):
    def __init__(self):
        self.clients = []
clientManager = ClientManager()

def GetFilesFromFolder(mypath):
    from os import walk
    files = []
    for (dirpath, dirnames, filenames) in walk(mypath):
        files.extend(filenames)
        break
    return [dict(name=f) for f in files]

def GetCPPFilesFromFolder(mypath):
    from parser import CppParser
    files=GetFilesFromFolder(mypath)
    cppFiles = []
    for fileInfo in files:
        fname = fileInfo['name']
        if fname.endswith('.cpp') or fname.endswith('.cxx'):
            cppFileInfo=dict(name=fname, classes=CppParser.GetSimpleClasses(mypath + '/' + fname))
            cppFiles.append(cppFileInfo)

    return cppFiles

class BroadcastWebSocket(EchoWebSocket):
    def opened(self):
        global clientManager
        import time
        import json
        self.cm = clientManager
        self.cm.clients.append(self)
        #time.sleep(1)
        self.send(json.dumps(dict(event='assetlist', files=GetCPPFilesFromFolder('/home/csantos/workspace/LauEngine/bin/assets/'))))

    def received_message(self, m):
        # self.clients is set from within the server
        # and holds the list of all connected servers
        # we can dispatch to
        for client in self.cm.clients:
            client.send(m)

    def closed(self, code, msg):
        if self in self.cm.clients:
            self.cm.clients.remove(self)
            for client in self.cm.clients:
                try:
                    client.send('Mandando msg de close')
                except:
                    pass

def StartServer():
    server = WSGIServer(('localhost', 9001), WebSocketWSGIApplication(handler_cls=BroadcastWebSocket))
    server.serve_forever()

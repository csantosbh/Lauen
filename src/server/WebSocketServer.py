from gevent import monkey; monkey.patch_all()

import gevent
import gevent.pywsgi

from ws4py.server.geventserver import WebSocketWSGIApplication, WebSocketWSGIHandler, WSGIServer
from ws4py.websocket import EchoWebSocket

import Event

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
        clientManager.clients.append(self)
        #time.sleep(1)
        # TODO: Move this to somewhere else (Files?) and broadcast a connected event
        send('assetlist', dict(files=GetCPPFilesFromFolder('/home/csantos/workspace/LauEngine/bin/assets/')))
        pass

    def received_message(self, m):
        # self.clients is set from within the server
        # and holds the list of all connected servers
        # we can dispatch to
        import json
        msg = json.loads(str(m))
        Event.broadcast(msg['event'], msg)
        pass

    def closed(self, code, msg):
        global clientManager
        if self in clientManager.clients:
            clientManager.clients.remove(self)

def send(event, message):
    import json
    msg = dict(message)
    msg['event'] = event
    msg = json.dumps(msg)
    for client in clientManager.clients:
        client.send(msg)
        pass
    pass

def StartServer():
    server = WSGIServer(('localhost', 9001), WebSocketWSGIApplication(handler_cls=BroadcastWebSocket))
    server.serve_forever()
    pass

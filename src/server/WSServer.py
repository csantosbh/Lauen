from gevent import monkey; monkey.patch_all()

import gevent
import gevent.pywsgi

from ws4py.server.geventserver import WebSocketWSGIApplication, WebSocketWSGIHandler, WSGIServer
from ws4py.websocket import EchoWebSocket

###
import Config, Event
from project import Project
from io import Utils
###

class ClientManager(object):
    def __init__(self):
        self.clients = []
clientManager = ClientManager()

class _LAUWebSocketHandler(EchoWebSocket):
    def opened(self):
        global clientManager
        clientManager.clients.append(self)
        pass

    def received_message(self, m):
        # self.clients is set from within the server
        # and holds the list of all connected servers
        # we can dispatch to
        import json
        msg = json.loads(str(m))
        Event.broadcast(msg['event'], msg['msg'])
        pass

    def closed(self, code, msg):
        global clientManager
        if self in clientManager.clients:
            clientManager.clients.remove(self)

def send(event, message):
    import json
    msg = dict(event=event, msg=message)
    msg = json.dumps(msg)
    for client in clientManager.clients:
        client.send(msg)
        pass
    pass

def serve(blocking = False):
    ws_server = WSGIServer(('localhost', int(Config.get('server', 'ws_port'))), WebSocketWSGIApplication(handler_cls=_LAUWebSocketHandler))
    if blocking == True:
        ws_server.serve_forever() # Non-blocking serve
    else:
        ws_server.start() # Non-blocking serve
        pass
    pass

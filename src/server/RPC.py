from server import Event
from server import WebSocketServer

procedures={}
def listen(procedureName, callback):
    if procedures.has_key(procedureName):
        print 'ERROR: Procedure ' + procedureName + ' already registered!'
        pass
    procedures[procedureName] = callback
    pass

def _RPCHandler(event):
    if procedures.has_key(event['procedure']):
        WebSocketServer.send('RPCAnswer', dict(to=event['from'], result=procedures[event['procedure']](event['parameters'])))
        pass
    pass

Event.listen('RPCCall', _RPCHandler)

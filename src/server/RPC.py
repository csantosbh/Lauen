from server import Event
from server import WebSocketServer

procedures={}
def listen(callback):
    if procedures.has_key(callback.__name__):
        print 'ERROR: Procedure ' + callback.__name__ + ' already registered!'
        pass
    procedures[callback.__name__] = callback
    pass

def _RPCHandler(event):
    if procedures.has_key(event['procedure']):
        WebSocketServer.send('RPCAnswer', dict(to=event['from'], result=procedures[event['procedure']](event['parameters'])))
        pass
    pass

Event.listen('RPCCall', _RPCHandler)

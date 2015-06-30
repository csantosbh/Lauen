events={}
def listen(eventName, callback):
    if not events.has_key(eventName):
        events[eventName] = []
        pass

    events[eventName].append(callback)
    pass

def broadcast(eventName, eventData):
    if events.has_key(eventName):
        for listener in events[eventName]:
            listener(eventData)
        pass
    else:
        # TODO properly log this problem
        print '[warning] Event "' + eventName + '" reached no listeners!'
    pass

if __name__ == '__main__':
    def tstListener(data):
        from pprint import pprint
        pprint(data)
        pass
    broadcast('test', dict(afterMessage='This should trigger a warning'))
    listen('before', tstListener)
    broadcast('before', dict(beforeMessage='Congrats, you received the before after listening!'))
    listen('after', tstListener)
    broadcast('after', dict(afterMessage='Congrats, you received right on time!'))
    broadcast('after', dict(afterMessage='Congrats, you received right on time!!!'))
    broadcast('before', dict(beforeMessage='Congrats, you received the before after listening (2)!'))
    pass


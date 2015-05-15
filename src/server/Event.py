events={}
pendingBroadcasts={}
def listen(eventName, callback):
    if not events.has_key(eventName):
        events[eventName] = []
        pass

    events[eventName].append(callback)

    if pendingBroadcasts.has_key(eventName):
        for i in pendingBroadcasts[eventName]:
            callback(i)
            pass

        # We don't need to store this event anymore
        pendingBroadcasts.pop(eventName)
        pass
    pass

def broadcast(eventName, eventData):
    if events.has_key(eventName):
        for listener in events[eventName]:
            listener(eventData)
        pass
    else:
        # Store the event so the first subscriber will receive it
        # TODO: Discard too old entries
        if not pendingBroadcasts.has_key(eventName):
            pendingBroadcasts[eventName] = []
            pass
        pendingBroadcasts[eventName].append(eventData)
        pass
    pass

if __name__ == '__main__':
    def tstListener(data):
        from pprint import pprint
        pprint(data)
        pass
    broadcast('before', dict(beforeMessage='Congrats, you received posthumously (1)!'))
    broadcast('before', dict(beforeMessage='Congrats, you received posthumously (2)!'))
    listen('before', tstListener)
    listen('after', tstListener)
    broadcast('after', dict(afterMessage='Congrats, you received right on time!'))
    broadcast('after', dict(afterMessage='Congrats, you received right on time!!!'))
    broadcast('before', dict(beforeMessage='Congrats, you received the before after listening!'))
    pass


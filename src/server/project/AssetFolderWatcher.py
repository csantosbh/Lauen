import os
import pyinotify
import threading
from server import WSServer
from server.io import Utils
from server.project import Project

class _AssetFolderWatcher(pyinotify.ProcessEvent):
    def process_IN_CLOSE_WRITE(self, event):
        evPath = os.path.relpath(event.pathname, Project.getProjectFolder())
        asset = Project.processAsset(evPath, saveProject=True)
        if asset != None and Project.isUserAsset(evPath):
            WSServer.send('AssetWatch', dict(event="update", asset=asset))
        pass

    def process_IN_DELETE(self, event):
        evPath = os.path.relpath(event.pathname, Project.getProjectFolder())
        Project.removeAsset(evPath, saveProject=True)
        if Utils.IsTrackableAsset(evPath) and Project.isUserAsset(evPath):
            WSServer.send('AssetWatch', dict(event="delete", path=evPath))
        pass

    pass

class _AssetFolderWatcherThread(threading.Thread):
    def __init__(self, watchPath):
        super(_AssetFolderWatcherThread, self).__init__()
        self.watchPath = watchPath
        self.isRunning = True
        pass

    def stop(self):
        self.isRunning = False
        pass

    def run(self):
        wm = pyinotify.WatchManager()  # Watch Manager
        mask = pyinotify.IN_DELETE | pyinotify.IN_CLOSE_WRITE  # watched events
        notifier = pyinotify.Notifier(wm, _AssetFolderWatcher(), timeout=10)
        wdd = wm.add_watch(self.watchPath, mask, rec=True)

        while self.isRunning:
            notifier.process_events()
            while notifier.check_events(timeout=1000):  #loop in case more events appear while we are processing
                notifier.read_events()
                notifier.process_events()
                pass
            pass
        pass

    pass

_watcherThread=None

def WatchFolder(path):
    global _watcherThread

    if _watcherThread != None:
        _watcherThread.stop()
        _watcherThread.join()
        pass

    _watcherThread=_AssetFolderWatcherThread(path)
    _watcherThread.daemon = True
    _watcherThread.start()
    pass


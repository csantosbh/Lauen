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
    def run(self):
        wm = pyinotify.WatchManager()  # Watch Manager
        mask = pyinotify.IN_DELETE | pyinotify.IN_CLOSE_WRITE  # watched events
        notifier = pyinotify.Notifier(wm, _AssetFolderWatcher(), timeout=10)
        path = Project.getProjectFolder()
        wdd = wm.add_watch(path, mask, rec=True)

        while True:
            notifier.process_events()
            while notifier.check_events(timeout=10000):  #loop in case more events appear while we are processing
                notifier.read_events()
                notifier.process_events()
                pass
            pass
        pass

    pass

watcherThread=_AssetFolderWatcherThread()
watcherThread.daemon = True
watcherThread.start()


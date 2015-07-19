import pyinotify
import threading
from server import WSServer
from server.io import Utils
from server.project import Project

class _AssetFolderWatcher(pyinotify.ProcessEvent):
    def process_IN_CLOSE_WRITE(self, event):
        asset = Project.processAsset(event.pathname, saveProject=True)
        if asset != None:
            WSServer.send('AssetWatch', dict(event="update", asset=asset))
        pass

    def process_IN_DELETE(self, event):
        if Utils.IsTrackableAsset(event.pathname):
            WSServer.send('AssetWatch', dict(event="delete", path=event.pathname))
        pass

    pass

class _AssetFolderWatcherThread(threading.Thread):
    def run(self):
        wm = pyinotify.WatchManager()  # Watch Manager
        mask = pyinotify.IN_DELETE | pyinotify.IN_CLOSE_WRITE  # watched events
        notifier = pyinotify.Notifier(wm, _AssetFolderWatcher(), timeout=10)
        path = Project.getProjectFolder()
        wdd = wm.add_watch(path, mask, rec=True)

        self.isRunning = True
        while self.isRunning:
            notifier.process_events()
            while notifier.check_events():  #loop in case more events appear while we are processing
                notifier.read_events()
                notifier.process_events()
                pass
            pass
        pass
    pass

    def stop(self):
        self.isRunning = False

watcherThread=_AssetFolderWatcherThread()
watcherThread.start()

def stopWatcher():
    watcherThread.stop()
    watcherThread.join()
    pass

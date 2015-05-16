import sys
import time
import logging
import watchdog
from watchdog.observers import Observer
from watchdog.events import LoggingEventHandler

class LauEventHandler(watchdog.events.FileSystemEventHandler):
    def on_created(self, event):
        print 'Created!'
    def on_deleted(self, event):
        print 'deleted!'
    def on_modified(self, event):
        print 'modified!'
    def on_moved(self, event):
        print 'moved!'

if __name__ == "__main__":
    logging.basicConfig(level=logging.INFO,
                        format='%(asctime)s - %(message)s',
                        datefmt='%Y-%m-%d %H:%M:%S')
    path = sys.argv[1] if len(sys.argv) > 1 else '.'
    event_handler = LauEventHandler()
    observer = Observer()
    observer.schedule(event_handler, path, recursive=True)
    observer.start()
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        observer.stop()
    observer.join()

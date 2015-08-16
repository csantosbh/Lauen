import SocketServer
import SimpleHTTPServer
import urllib

###
from project import Project
import Config
###

class _LAUHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def log_message(self,format,*args):
        # Ignore HTTP log messages.
        pass

    def do_GET(self):
        if self.path == '/lau_canvas.nmf' or self.path == '/lau_canvas.pexe' or self.path == '/game':
            # TODO this will be requested in the first time
            # that the user opens the editor. But since theres no
            # project, it will issue an error. Fix this.
            projFolder = Project.getProjectFolder()
            if projFolder != None:
                self.copyfile(open(projFolder+'/build/nacl'+self.path, 'r'), self.wfile)
                pass
        elif self.path.startswith('/default_assets/') or self.path.startswith('/assets/') or self.path.startswith('/scenes/'):
            projFolder = Project.getProjectFolder()
            if projFolder != None:
                self.copyfile(open(projFolder+self.path, 'r'), self.wfile)
                pass
        else:
            return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)
            #self.copyfile(open('./'+self.path, 'r'), self.wfile)
            pass
        pass
    pass

def serve():
    SocketServer.TCPServer.allow_reuse_address = True

    http_server = SocketServer.ForkingTCPServer(('localhost', int(Config.get('server', 'http_port'))), _LAUHTTPRequestHandler)
    http_server.serve_forever()
    pass


import SocketServer
import SimpleHTTPServer
import urllib
import os

###
from project import Project
from io import Utils
import Config
###

class _LAUHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def log_message(self,format,*args):
        # Ignore HTTP log messages.
        pass

    def send_headers(self, path):

        try:
            f = open(path, 'rb')
        except IOError as error:
            self.send_response(404)
            self.send_header("Access-Control-Allow-Origin", "*")
            self.end_headers()
            return None

        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        #self.send_header("Content-type", ctype)
        fs = os.fstat(f.fileno())
        self.send_header("Content-Length", str(fs[6]))
        self.send_header("Last-Modified", self.date_time_string(fs.st_mtime))
        self.end_headers()
        return f

    def do_GET(self):
        if self.path == '/lau_canvas.nmf' or self.path == '/lau_canvas.pexe' or self.path == '/game':
            # TODO this will be requested in the first time
            # that the user opens the editor. But since theres no
            # project, it will issue an error. Fix this.
            projFolder = Project.getProjectFolder()
            path = projFolder+'/build/nacl'+self.path
            f = self.send_headers(path)
            if projFolder != None and f != None:
                self.copyfile(f, self.wfile)
                f.close()
                pass
        elif self.path.startswith('/default_assets/') or self.path.startswith('/assets/') or self.path.startswith('/scenes/'):
            projFolder = Project.getProjectFolder()
            path = projFolder+self.path
            f = self.send_headers(path)
            if projFolder != None and f != None and Utils.FileExists(projFolder+self.path):
                self.copyfile(f, self.wfile)
                f.close()
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


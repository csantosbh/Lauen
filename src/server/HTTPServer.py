import SocketServer
import SimpleHTTPServer
import urllib

###
from project import Project
import Config
###

class _LAUHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        if self.path.endswith('lau_canvas.nmf'):
            # TODO this will be requested in the first time
            # that the user opens the editor. But since theres no
            # project, it will issue an error. Fix this.
            projFolder = Project.getProjectFolder()
            print 'lau_canvas.nmf requested'
            self.copyfile(open(projFolder+'/build/nacl/lau_canvas.nmf', 'r'), self.wfile)
        elif self.path.endswith('lau_canvas.pexe'):
            projFolder = Project.getProjectFolder()
            print 'lau_canvas.pexe requested'
            self.copyfile(open(projFolder+'/build/nacl/lau_canvas.pexe', 'r'), self.wfile)
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


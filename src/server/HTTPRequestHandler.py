#!/usr/bin/python
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
import SimpleHTTPServer
import Config
import urlparse
import cgi
import json

class LauHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    # Send an empty page and close the server
    def _SendNothingAndDie(self, result=0):
        self.send_response(200, 'OK')
        self.send_header('Content-type', 'text/html')
        self.send_header('Content-length', '0')
        self.end_headers()
        self.server.Shutdown(result)

    # Handler for the GET requests
    def do_GET(self):
        _, _, _, query, _ = urlparse.urlsplit(self.path)
        if query:
            params = urlparse.parse_qs(query)
            if '1' in params.get('quit', []):
                self._SendNothingAndDie()
                return

        return SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)

    def getPostVars(self):
        ctype, pdict = cgi.parse_header(self.headers.getheader('content-type'))

        if ctype == 'application/json':
            length = int(self.headers.getheader('content-length'))
            postvars = json.loads(self.rfile.read(length))
        elif ctype == 'multipart/form-data':
            postvars = cgi.parse_multipart(self.rfile, pdict)
        elif ctype == 'application/x-www-form-urlencoded':
            length = int(self.headers.getheader('content-length'))
            postvars = cgi.parse_qs(self.rfile.read(length), keep_blank_values=1)

        return postvars

    def do_POST(self):
        print 'Got post!!'
        args = self.getPostVars()
        print args

def StartServer():
    # Create a web server and define the handler to manage the
    # incoming request
    portNumber = int(Config.get('server', 'port'))
    server = HTTPServer(('', portNumber), LauHTTPRequestHandler)
    try:
        print ('Started httpserver on port ' , portNumber)
        # Wait forever for incoming http requests
        #server.serve_forever()
        while True:
            server.handle_request()
    except KeyboardInterrupt:
        server.socket.close()

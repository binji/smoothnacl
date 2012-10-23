#!/usr/bin/env python
import os
import SimpleHTTPServer
import SocketServer
import sys

PORT = 5103

if __name__ == '__main__':
  with open('httpd.pid', 'w') as f:
    f.write('%s' % os.getpid())

  os.chdir(sys.argv[1])
  handler = SimpleHTTPServer.SimpleHTTPRequestHandler
  httpd = SocketServer.TCPServer(("", PORT), handler)
  httpd.serve_forever()

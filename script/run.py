#!/usr/bin/env python
import os
import subprocess
import sys
import test_server

if __name__ == '__main__':
  server = test_server.LocalHTTPServer(sys.argv[1])
  try:
    subprocess.call(sys.argv[2:] + [server.GetURL('')])
  finally:
    server.Shutdown()

#!/usr/bin/python
import os, sys
from server.parser import CppParser

if __name__ == '__main__':
    from pprint import pprint
    if len(sys.argv) != 2:
        print 'Usage: '+sys.argv[0]+' <cppFile>'
        exit()
        pass
    pprint(CppParser.parseCPPFile(sys.argv[1]))

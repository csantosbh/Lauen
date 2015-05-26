#!/usr/bin/python

import sys, traceback

def print_usage():
    print """Usage:
%s <mode> <params...>

Where <mode> can be:
    parse: Test python parser. Params is a path to the C++ file.
    """%sys.argv[0]
    return

def test_parser():
    from server.parser import CppParser
    from pprint import pprint
    pprint(CppParser.GetSimpleClass(sys.argv[2]))
    pass

try:
    if len(sys.argv) == 0:
        print_usage()
        exit()
        pass
    if sys.argv[1] == 'parse':
        test_parser()
        pass
except Exception as e:
    traceback.print_exc()
    print_usage()
    exit()


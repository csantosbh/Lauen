def GetFilesFromFolder(mypath):
    from os import walk
    files = []
    for (dirpath, dirnames, filenames) in walk(mypath):
        for filename in filenames:
            files.append(dirpath + '/' + filename)
        pass
    return [dict(name=f) for f in files]

def GetCPPFilesFromFolder(mypath):
    from server.parser import CppParser
    files = GetFilesFromFolder(mypath)
    cppFiles = []
    for fileInfo in files:
        fname = fileInfo['name']
        if fname.endswith('.cpp') or fname.endswith('.cxx'):
            # TODO: cache results from cpp parser
            cppFileInfo=dict(name=fname, classes=CppParser.GetSimpleClasses(fname))
            cppFiles.append(cppFileInfo)
            pass
        pass

    return cppFiles


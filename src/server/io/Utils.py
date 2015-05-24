def GetFileNameFromPath(path):
    import ntpath
    return ntpath.basename(path)

def GetFilesFromFolder(mypath):
    from os import walk
    files = []
    for (dirpath, dirnames, filenames) in walk(mypath):
        for filename in filenames:
            files.append(dirpath + '/' + filename)
        pass
    return [dict(name=f) for f in files]

def ParseHPPFilesFromFolder(mypath):
    from server.parser import CppParser
    files = GetFilesFromFolder(mypath)
    cppFiles = []
    for fileInfo in files:
        fname = fileInfo['name']
        if fname.endswith('.hpp') or fname.endswith('.hxx'):
            # TODO: cache results from cpp parser
            cppFileInfo = CppParser.GetSimpleClass(fname)
            cppFileInfo['path'] = fname
            cppFiles.append(cppFileInfo)
            pass
        pass

    return cppFiles

def ListFilesFromFolder(mypath, extensions = None):
    from server.parser import CppParser
    files = GetFilesFromFolder(mypath)
    cppFiles = []
    for fileInfo in files:
        fname = fileInfo['name']
        if extensions == None:
            cppFiles.append(fname)
            pass
        else:
            for extension in extensions:
                if fname.endswith(extension):
                    cppFiles.append(fname)
                    break
                pass
            pass
        pass

    return cppFiles

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
    filesFound = []
    for fileInfo in files:
        fname = fileInfo['name']
        if extensions == None:
            filesFound.append(fname)
            pass
        else:
            for extension in extensions:
                if fname.endswith(extension):
                    filesFound.append(fname)
                    break
                pass
            pass
        pass

    return filesFound

# Copy all files of specified types to destination folder.
# The destination folder will be created, so as to keep
# the directory structure of the original file.
def CopyFilesOfTypes(src, dst, types, baseSrcFolder=None):
    from distutils import dir_util
    import shutil, os

    if baseSrcFolder==None:
        baseSrcFolder = src
        pass

    filesToCopy = ListFilesFromFolder(src, types)
    for f in filesToCopy:
        nameSuffix = os.path.relpath(f, baseSrcFolder)
        outputFilePath = os.path.join(dst, nameSuffix)
        # Make sure the containing folder will exist
        dir_util.mkpath(os.path.dirname(outputFilePath))
        # Copy file to destination
        shutil.copy(f, outputFilePath)
        pass
    pass


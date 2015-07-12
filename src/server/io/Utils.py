def GetFileNameFromPath(path):
    import ntpath
    return ntpath.basename(path)

def _ListFilesFromFolder(mypath):
    from os import walk
    files = []
    for (dirpath, dirnames, filenames) in walk(mypath):
        for filename in filenames:
            files.append(dirpath + '/' + filename)
        pass
    return files

def ParseHPPFilesFromFolder(mypath):
    from server.parser import CppParser
    files = ListFilesFromFolder(mypath, ['.hpp', '.hxx'])
    cppFiles = []
    for fname in files:
        # TODO: cache results from cpp parser
        cppFileInfo = CppParser.GetSimpleClass(fname)
        cppFileInfo['path'] = fname
        cppFiles.append(cppFileInfo)
        pass

    return cppFiles

def ListFilesFromFolder(mypath, extensions = None):
    from server.parser import CppParser
    files = _ListFilesFromFolder(mypath)
    filesFound = []
    for fname in files:
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


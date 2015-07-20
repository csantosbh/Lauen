def GetFileNameFromPath(path):
    import ntpath
    return ntpath.basename(path)

def _ListFilesFromFolder(mypath):
    from os import walk
    files = []
    for (dirpath, dirnames, filenames) in walk(mypath, followlinks=True):
        for filename in filenames:
            files.append(dirpath + '/' + filename)
        pass
    return files

def PathHasExtension(path, extensions):
    for extension in extensions:
        if path.endswith(extension):
            return True
        pass
    return False

def IsHeaderFile(assetPath):
    return PathHasExtension(assetPath, ['.hpp', '.h'])

def IsImplementationFile(assetPath):
    return PathHasExtension(assetPath, ['.cpp', '.cxx'])

def IsScriptFile(assetPath):
    return IsHeaderFile(assetPath) or IsImplementationFile(assetPath)

def IsTrackableAsset(filePath):
    # TODO also check for other supported file type
    return IsScriptFile(filePath)

def ListFilesFromFolder(mypath, extensions = None):
    from server.parser import CppParser
    files = _ListFilesFromFolder(mypath)
    filesFound = []
    if extensions == None:
        filesFound = files
    else:
        for fname in files:
            if PathHasExtension(fname, extensions):
                filesFound.append(fname)
                pass
            pass
        pass

    return filesFound

def OpenRec(filename, mode):
    import os
    if not os.path.exists(os.path.dirname(filename)):
        os.makedirs(os.path.dirname(filename))
    return open(filename, mode)

# Returns true if path is inside directory
def IsSubdir(path, directory):
    import os
    path = os.path.realpath(path)
    directory = os.path.realpath(directory)

    relative = os.path.relpath(path, directory)

    if relative.startswith(os.pardir):
        return False
    else:
        return True

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


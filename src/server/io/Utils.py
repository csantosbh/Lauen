import threading

_utilsLock = threading.Lock()

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

def IsObjectFile(assetPath):
    return PathHasExtension(assetPath, ['.o'])

def IsHeaderFile(assetPath):
    return PathHasExtension(assetPath, ['.hpp', '.h'])

def IsPrefabFile(assetPath):
    return PathHasExtension(assetPath, ['.prefab'])

def IsImplementationFile(assetPath):
    return PathHasExtension(assetPath, ['.cpp', '.cxx'])

def IsCpyFile(assetPath):
    return PathHasExtension(assetPath, ['.cpy'])

def IsScriptFile(assetPath):
    return IsHeaderFile(assetPath) or IsImplementationFile(assetPath)

def IsProcessableFile(path):
    return IsScriptFile(path) or IsCpyFile(path)

def IsTrackableAsset(filePath):
    # TODO also check for other supported file type
    return IsHeaderFile(filePath) or IsPrefabFile(filePath)

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

def FileExists(filename):
    import os
    return os.path.exists(filename)

def RemoveFile(filename):
    import os
    Console.info('Removing ' + filename)
    _utilsLock.acquire()
    if FileExists(filename):
        os.remove(filename)
        pass
    _utilsLock.release()
    pass

def CreateFoldersRec(path):
    import os
    _utilsLock.acquire()
    if not os.path.exists(path):
        os.makedirs(path)
        pass
    _utilsLock.release()
    pass

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
    pass

def Touch(fname):
    import os
    with open(fname, 'a'):
        os.utime(fname, None)
        pass
    pass

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
        # Copy file to destination, if it doesnt exist or is outdated
        if not FileExists(outputFilePath) or os.path.getmtime(outputFilePath) < os.path.getmtime(f):
            shutil.copy(f, outputFilePath)
            pass
        pass
    pass

# Colors for terminal printing
class Console:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKGREEN = '\033[92m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'

    @staticmethod
    def ok(msg):
        _utilsLock.acquire()
        print Console.BOLD+Console.OKGREEN+'[ok] '+Console.ENDC+msg
        _utilsLock.release()
        pass

    @staticmethod
    def error(msg):
        _utilsLock.acquire()
        print Console.BOLD+Console.FAIL+'Error: '+Console.ENDC+msg
        _utilsLock.release()
        pass

    @staticmethod
    def fail(msg):
        _utilsLock.acquire()
        print Console.BOLD+Console.FAIL+'[fail] '+Console.ENDC+msg
        _utilsLock.release()
        pass

    @staticmethod
    def warning(msg):
        _utilsLock.acquire()
        print Console.BOLD+Console.WARNING+'Warning: '+Console.ENDC+msg
        _utilsLock.release()
        pass

    @staticmethod
    def info(msg):
        _utilsLock.acquire()
        print Console.BOLD+Console.OKBLUE+'Info: '+Console.ENDC+msg
        _utilsLock.release()
        pass

    @staticmethod
    def step(msg):
        _utilsLock.acquire()
        print '\t'+Console.BOLD+Console.OKBLUE+'[v] '+Console.ENDC+msg
        _utilsLock.release()
        pass

    pass

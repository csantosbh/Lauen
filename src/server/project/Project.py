import os

from server import Config
from server.io import Utils

class _Project:
    def __init__(self, project_path = None):
        import json, shutil
        from distutils import dir_util
        self.project_path = project_path
        # TODO support projects of arbitrary names (low priority)
        if self.project_path == None:
            self.assets = {}
            self.scenes = ['scenes/scene0.json']
            self.currentScene = 0
        else:
            if not os.path.isfile(self.project_path):
                project_folder = os.path.dirname(self.project_path)
                exePath = Config.env('install_location')
                # Copy template project
                dir_util.copy_tree(exePath + '/template_project', project_folder)
                # Copy default assets
                shutil.copytree(exePath + '/engine/default_assets', project_folder + '/default_assets')
                # Create assets folder
                dir_util.mkpath(project_folder + '/assets')
                # Create build folder
                dir_util.mkpath(project_folder + '/build')
                pass

            projInfo = json.loads(open(self.project_path, 'r').read())

            self.assets = projInfo['assets']
            self.scenes = projInfo['scenes']
            self.currentScene = projInfo['currentScene']
            pass

        # Internal fields
        self._processAssetRecursionLock = set()
        self._assetId2Path = dict()

        removedAssets=[]
        for asset in self.assets:
            if not Utils.FileExists(asset):
                removedAssets.append(asset)
            elif 'id' in self.assets[asset]:
                self._assetId2Path[self.assets[asset]['id']] = asset
                pass
            pass

        # Remove deleted assets from the project
        for removedAsset in removedAssets:
            self.removeAsset(removedAsset, True)
            pass

        pass

    def isUserAsset(self, assetPath):
        return Utils.IsSubdir(assetPath, self.getProjectFolder()+'/assets/')

    def getProjectFolder(self):
        import os
        return os.path.dirname(self.project_path)

    def initializeAsset(self, assetPath):
        if not assetPath in self.assets:
            if Utils.IsScriptFile(assetPath):
                if self.isUserAsset(assetPath) and Utils.IsHeaderFile(assetPath):
                    self.assets[assetPath] = {
                    'id': self.getUniqueId(),
                    'dependencies': [],
                    'mtime': 0
                    }
                else:
                    self.assets[assetPath] = {
                    'dependencies': [],
                    'mtime': 0
                    }
                pass
            pass
        pass

    def getUniqueId(self):
        from random import randint
        candidateId = randint(0,1e9)
        while candidateId in self._assetId2Path:
            candidateId = randint(0,1e9)
            pass
        return candidateId

    def getScriptId(self, scriptPath):
        self.initializeAsset(scriptPath)
        return self.assets[scriptPath]['id']

    def saveProject(self):
        import json
        with open(self.project_path, 'w') as fhandle:
            fhandle.write(json.dumps(dict(
                assets=self.assets,
                scenes=self.scenes,
                currentScene=self.currentScene
            )))
            pass
        pass
    
    def saveCurrentScene(self, sceneData):
        import json
        if self.project_path == None:
            # User must create project first!
            return False

        project_folder = os.path.dirname(self.project_path)
        with open(project_folder + '/' + self.scenes[self.currentScene], 'w') as fhandle:
            fhandle.write(json.dumps(sceneData))
            pass

        return True
        pass

    def loadCurrentScene(self):
        import json
        # No project created yet, so nothing in current scene to return
        if self.project_path == None:
            return []

        project_folder = os.path.dirname(self.project_path)
        fname = project_folder + '/' + self.scenes[self.currentScene]

        try:
            f = open(fname, 'r')
            sceneJson = f.read()
            f.close()
            return json.loads(sceneJson)
        except IOError:
            # TODO do something if scene file is gone
            return None
            pass
        pass

    def getAssetList(self):
        if self.project_path == None:
            return []

        project_folder = os.path.dirname(self.project_path)
        files = Utils.ListFilesFromFolder(project_folder)
        assetFiles = []
        for fname in files:
            processedAsset = self.processAsset(fname, False)
            if processedAsset != None and self.isUserAsset(fname) and Utils.IsHeaderFile(fname):
                assetFiles.append(processedAsset)
                pass
            pass
        # Automatically save project
        self.saveProject()
        return assetFiles
    
    # dependencies is a set
    def updateAsset(self, filePath, dependencies, saveProject):
        # Normalize path to avoid ambiguities
        filePath = os.path.abspath(filePath)

        # If file exists, update it.
        if Utils.FileExists(filePath):
            # Update dependencies list for current asset
            self.assets[filePath]['dependencies'] = list(dependencies)
            self.assets[filePath]['mtime'] = os.path.getmtime(filePath)
        elif filePath in self.assets:
            # If tile doesnt exist, remove it from project
            del self.assets[filePath]

        # Save project, if requested
        if saveProject:
            self.saveProject()
        pass

    def getAssetInfoCachePath(self, assetPath):
        relative = os.path.relpath(assetPath, self.getProjectFolder())
        return self.getProjectFolder()+'/cache/'+relative+'.json'

    def saveAssetInfoCache(self, assetPath, assetSymbols):
        import json
        content=json.dumps(assetSymbols)
        with Utils.OpenRec(self.getAssetInfoCachePath(assetPath), 'w') as f:
            f.write(content)
            pass
        pass

    def removeAssetInfoCache(self, assetPath):
        import json
        Utils.RemoveFile(self.getAssetInfoCachePath(assetPath))
        pass

    def loadAssetInfoCache(self, assetPath):
        import json
        with open(self.getAssetInfoCachePath(assetPath), 'r') as f:
            return json.loads(f.read())
            pass
        pass

    def removeAsset(self, assetPath, saveProject):
        # Handle DELETE cases
        if not Utils.FileExists(assetPath):
            if Utils.IsScriptFile(assetPath):
                # Remove cache .json files
                self.removeAssetInfoCache(assetPath)
                pass

            if Utils.IsImplementationFile(assetPath):
                # Remove preview .o files
                fname = Utils.GetFileNameFromPath(assetPath)
                objectFilePaths = [self.getProjectFolder()+'/build/'+fname+'.o',
                        self.getProjectFolder()+'/build/nacl/build/'+fname+'.o']
                for obj in objectFilePaths:
                    Utils.RemoveFile(obj)
                    pass
                pass

            if Utils.IsHeaderFile(assetPath) and self.isUserAsset(assetPath):
                fname = Utils.GetFileNameFromPath(assetPath)
                assetId = self.assets[assetPath]['id']

                # Remove FactoryPeeker
                factoryFilePath = self.getProjectFolder()+'/default_assets/factories/'+fname[:fname.rfind('.')] +'_'+str(assetId) + '.cpp'
                Utils.RemoveFile(factoryFilePath)

                pass

            # Remove asset from the project list of assets
            self.updateAsset(assetPath, None, saveProject=saveProject)
            # TODO remove asset from the dependency list of all other assets
            pass
        pass

    def processAsset(self, assetPath, saveProject):
        from server.parser import CppParser
        from server.build import BuildEventHandler

        # Avoid infinite loop
        if assetPath in self._processAssetRecursionLock:
            print '\tRequested processAsset('+assetPath+'); already processing this asset.'
            return None

        self._processAssetRecursionLock.add(assetPath)

        # Handle CHANGE/CREATE cases
        if Utils.IsScriptFile(assetPath):
            # Initialize asset reference, if none exist
            self.initializeAsset(assetPath)

            # Generate its .o object. Check if the path doesn't refer to a
            # header file
            if Utils.IsImplementationFile(assetPath):
                buildStatus = BuildEventHandler.BuildPreviewObject(assetPath)

                if buildStatus['returncode'] != 0:
                    self._processAssetRecursionLock.remove(assetPath)
                    return None

                pass

            # Check if the asset is up to date, so we dont need to process it
            if self.assets[assetPath]['mtime'] >= os.path.getmtime(assetPath):
                fileSymbols = self.loadAssetInfoCache(assetPath)
            else:
                print 'Reprocessing asset '+assetPath
                # Parse asset
                fileInfo = CppParser.GetSimpleClass(assetPath)
                fileSymbols = fileInfo['symbols']
                fileSymbols['path'] = assetPath
                if 'id' in self.assets[assetPath]:
                    # This is a user script asset! Set its id.
                    fileSymbols['id'] = self.assets[assetPath]['id']
                    # Re-generate its FactoryPeeker file.
                    BuildEventHandler.RenderFactorySources([fileSymbols])
                    pass

                self.updateAsset(assetPath, fileInfo['dependencies'], saveProject=saveProject)
                self.saveAssetInfoCache(assetPath, fileSymbols)

                # Update all files that depend on assetPath
                recurseDependencies = []
                for asset in self.assets:
                    if assetPath in self.assets[asset]['dependencies']:
                        recurseDependencies.append(asset)
                        pass
                    pass
                for asset in recurseDependencies:
                    print assetPath,' is dependency of ', asset
                    self.processAsset(asset, False)
                    pass

                pass

            self._processAssetRecursionLock.remove(assetPath)

            if Utils.IsHeaderFile(assetPath):
                return fileSymbols
            else:
                return None

        self._processAssetRecursionLock.remove(assetPath)
        return None # Untrackable file format

    def isFileOlderThanDependency(self, filePath, assetPath):
        # If file doesnt even exist, then it must be touched to begin with
        if not os.path.exists(filePath):
            print '\tFile '+filePath+' doesn\'t exist.'
            return True
        # If the asset itself is newer than the filePath, return true
        queryFileMTime = os.path.getmtime(filePath)
        if os.path.getmtime(assetPath) > queryFileMTime:
            print '\tFile '+assetPath+' is newer than '+filePath+'.'
            return True

        # If any of the asset dependencies are newer than the filePath, return true
        for dependency in self.assets[assetPath]['dependencies']:
            if os.path.getmtime(dependency) > queryFileMTime:
                print '\tFile '+dependency+' is newer than '+filePath
                return True
            pass

        return False

    def isCPYTemplateOutdated(self, cpyFilePath):
        cppFilePath = cpyFilePath[:cpyFilePath.rfind('.')] + '.cpp'
        # If C++ file doesnt even exist, then it must be touched to begin with
        if not os.path.exists(cppFilePath):
            print '\tFile '+cppFilePath+' doesn\'t exist.'
            return True

        # If the template itself is newer than the C++ file, return true
        cppFileMTime = os.path.getmtime(cppFilePath)
        if os.path.getmtime(cpyFilePath) > cppFileMTime:
            print '\tFile '+cpyFilePath+' is older than '+cppFilePath
            return True

        # If the C++ file is older than any of its dependencies, return true
        for dependency in self.assets[cppFilePath]['dependencies']:
            if os.path.getmtime(dependency) > cppFileMTime:
                print '\tFile '+dependency+' is older than '+cppFilePath
                return True
            pass

        return False

    pass

def getAssetList():
    global _currentProject
    return _currentProject.getAssetList()

def getProjectFolder():
    global _currentProject
    return _currentProject.getProjectFolder()

def createNewProject(path):
    global _currentProject
    global _currentProject
    _currentProject = _Project(path)
    pass

def saveCurrentScene(sceneData):
    global _currentProject
    return _currentProject.saveCurrentScene(sceneData)

def getScriptId(scriptPath):
    global _currentProject
    return _currentProject.getScriptId(scriptPath)

def loadCurrentScene():
    global _currentProject
    return _currentProject.loadCurrentScene()

def loadProject(path):
    global _currentProject
    _currentProject = _Project(path)
    return True

def processAsset(assetPath, saveProject):
    global _currentProject
    return _currentProject.processAsset(assetPath, saveProject)

def removeAsset(assetPath, saveProject):
    global _currentProject
    return _currentProject.removeAsset(assetPath, saveProject)

def isFileOlderThanDependency(filePath, assetPath):
    global _currentProject
    return _currentProject.isFileOlderThanDependency(filePath, assetPath)

def isCPYTemplateOutdated(cpyFilePath):
    global _currentProject
    return _currentProject.isCPYTemplateOutdated(cpyFilePath)

def isUserAsset(filePath):
    global _currentProject
    return _currentProject.isUserAsset(filePath)

# Load last opened project
if len(Config.get('runtime', 'recent_projects')) > 0:
    _currentProject = _Project(Config.get('runtime', 'recent_projects')[0])
else:
    _currentProject = _Project()
    pass


import os

from server import Config
from server.io import Utils
import AssetProcessor

class _Project:
    def __init__(self):
        # Internal fields
        self._processAssetRecursionLock = set()
        self._assetId2Path = dict()
        pass

    def initialize(self, project_path = None):
        import json, shutil
        from distutils import dir_util
        self.project_path = project_path
        # TODO support projects of arbitrary names (low priority)
        self.assets = {}
        if self.project_path == None:
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

            # Add files from the project json file
            for asset in projInfo['assets']:
                assetProc = AssetProcessor.CreateAssetProcessor(asset, projInfo['assets'][asset])
                if assetProc != None:
                    self.assets[asset] = assetProc
                    pass
                pass

            # Add new files created while the engine was closed
            newFiles = Utils.ListFilesFromFolder(self.project_path)
            for asset in newFiles:
                if asset in self.assets:
                    continue

                assetProc = AssetProcessor.CreateAssetProcessor(asset, asset)
                if assetProc != None:
                    self.assets[asset] = assetProc
                    Utils.Console.info('Adding file to project: '+asset)
                    pass
                pass

            self.scenes = projInfo['scenes']
            self.currentScene = projInfo['currentScene']
            pass

        # Remove deleted assets from the project
        removedAssets=[]
        for asset in self.assets:
            if not Utils.FileExists(asset):
                removedAssets.append(asset)
            else:
                self._assetId2Path[self.assets[asset].id()] = asset
                pass
            pass

        for removedAsset in removedAssets:
            self.removeAsset(removedAsset, True)
            pass

        pass

    def isUserAsset(self, assetPath):
        return Utils.IsSubdir(assetPath, self.getProjectFolder()+'/assets/')

    def isUserScriptFactory(self, assetPath):
        return Utils.IsSubdir(assetPath, self.getProjectFolder()+'/default_assets/factories/')

    def getProjectFolder(self):
        import os
        return os.path.dirname(self.project_path)

    def getUniqueAssetId(self):
        from random import randint
        candidateId = randint(0,1e9)
        while candidateId in self._assetId2Path:
            candidateId = randint(0,1e9)
            pass
        return candidateId

    def saveProject(self):
        import json
        print 'Saving project...'
        with open(self.project_path, 'w') as fhandle:
            fhandle.write(json.dumps(dict(
                assets=self.assets,
                scenes=self.scenes,
                currentScene=self.currentScene
            ), cls=AssetProcessor.AssetJSONEncoder))
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
        assetFiles = []
        for fname in self.assets:
            processedAsset = self.assets[fname].getMetadata()
            if processedAsset != None and self.isUserAsset(fname) and Utils.IsTrackableAsset(fname):
                assetFiles.append(processedAsset)
                pass
            pass
        return assetFiles
    
    def removeAsset(self, assetPath, saveProject):
        # TODO remove asset from the dependency list of all other assets
        if assetPath in self.assets:
            self.assets[assetPath].remove()
            del self.assets[assetPath]
        pass

    def processAsset(self, assetPath, saveProject):
        from server.parser import CppParser
        from server.build import BuildEventHandler

        # Ignore irrelevant file formats
        if not Utils.IsProcessableFile(assetPath) and not self.isUserScriptFactory(assetPath):
            return None

        # Avoid infinite loop
        if assetPath in self._processAssetRecursionLock:
            print '\tRequested processAsset('+assetPath+'); already processing this asset.'
            return None

        self._processAssetRecursionLock.add(assetPath)

        if not (assetPath in self.assets):
            # Handle CREATE cases
            assetProc = AssetProcessor.CreateAssetProcessor(assetPath)

            if assetProc == None:
                self._processAssetRecursionLock.remove(assetPath)
                return None

            self.assets[assetPath] = assetProc
            pass

        # Process modified asset
        editorData = self.assets[assetPath].getMetadata()
        self.assets[assetPath].update()

        # Update all files that depend on assetPath
        if not self.assets[assetPath].isBroken:
            recurseDependencies = []
            for asset in self.assets:
                if self.assets[asset].dependsOn(assetPath):
                    recurseDependencies.append(asset)
                    pass
                pass
            for asset in recurseDependencies:
                print assetPath+' is dependency of '+ asset
                self.processAsset(asset, False)
                pass
            pass

        self._processAssetRecursionLock.remove(assetPath)

        # After done processing all assets, save the project, if requested
        if (len(self._processAssetRecursionLock) == 0) and saveProject:
            self.saveProject()
            print 'Done!\n\n'
            pass

        return editorData

    def isFileOlderThanDependency(self, filePath, assetPath):
        # If file doesnt even exist, then it must be touched to begin with
        if not Utils.FileExists(filePath):
            print '\tFile '+filePath+' doesn\'t exist.'
            return True
        # If the asset itself is newer than the filePath, return true
        queryFileMTime = os.path.getmtime(filePath)
        if os.path.getmtime(assetPath) > queryFileMTime:
            print '\tFile '+assetPath+' is newer than '+filePath+'.'
            return True

        # If any of the asset dependencies are newer than the filePath, return true
        if assetPath in self.assets:
            for dependency in self.assets[assetPath].dependencies():
                if os.path.getmtime(dependency) > queryFileMTime:
                    print '\tFile '+dependency+' is newer than '+filePath
                    return True
                pass
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
    _currentProject = _Project()
    _currentProject.initialize(path)
    pass

def saveCurrentScene(sceneData):
    global _currentProject
    return _currentProject.saveCurrentScene(sceneData)

def loadCurrentScene():
    global _currentProject
    return _currentProject.loadCurrentScene()

def loadProject(path):
    global _currentProject
    _currentProject = _Project()
    _currentProject.initialize(path)
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

def isUserScriptFactory(filePath):
    global _currentProject
    return _currentProject.isUserScriptFactory(filePath)

def getUniqueAssetId():
    global _currentProject
    return _currentProject.getUniqueAssetId()

# Load last opened project
if len(Config.get('runtime', 'recent_projects')) > 0:
    _currentProject = _Project()
    _currentProject.initialize(Config.get('runtime', 'recent_projects')[0])
else:
    _currentProject = _Project()
    _currentProject.initialize()
    pass


import os
import threading
import Queue
import multiprocessing

from server import Config
from server.io import Utils
import AssetProcessor

class _Project:
    def __init__(self):
        self.project_path = None
        # Internal fields
        self._assetId2Path = dict()
        self._assetUpdaterQueue = Queue.Queue()
        pass

    def bootstrap(self, project_path):
        import shutil
        from distutils import dir_util
        self.project_path = project_path
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
        # Remaining initialization
        self.initialize(project_path)
        pass

    def initialize(self, project_path = None):
        import json
        import AssetFolderWatcher

        # TODO support projects of arbitrary names (low priority)
        self.assets = {}
        if project_path == None or not Utils.FileExists(project_path):
            self.project_path = None
            self.scenes = ['scenes/scene0.json']
            self.currentScene = 0
        else:
            self.project_path = project_path
            projInfo = json.loads(open(self.project_path, 'r').read())

            # Add files from the project json file
            for asset in projInfo['assets']:
                assetProc = AssetProcessor.CreateAssetProcessor(asset, projInfo['assets'][asset])
                if assetProc != None:
                    self.assets[asset] = assetProc
                    pass
                pass

            # Add new files created while the engine was closed
            newFiles = Utils.ListFilesFromFolder(self.getProjectFolder())
            for fullAssetPath in newFiles:
                asset = os.path.relpath(fullAssetPath, self.getProjectFolder())
                if asset in self.assets:
                    continue

                assetProc = AssetProcessor.CreateAssetProcessor(asset)
                if assetProc != None:
                    self.assets[asset] = assetProc
                    Utils.Console.info('Adding file to project: '+asset)
                    pass
                elif self.isUserScriptFactory(asset):
                    # TODO provavelmente nao preciso desse processamento aqui. Verificar se o assetprocessor vai resolver isso, e remover esse codigo caso positivo
                    # Remove automatically generated files that depend on user
                    # generated assets, and whose original assets are no longer
                    # present.
                    parentAsset = os.path.relpath(asset, 'default_assets/factories')
                    parentAsset = parentAsset[:parentAsset.rfind('.')]
                    if not parentAsset in self.assets:
                        # This asset doesnt have a parent. Remove it.
                        removeFile(asset)
                        pass
                    pass
                pass

            self.scenes = projInfo['scenes']
            self.currentScene = projInfo['currentScene']

            # Remove deleted assets from the project
            removedAssets=[]
            for asset in self.assets:
                if not Utils.FileExists(self.getAbsProjFilePath(asset)):
                    removedAssets.append(asset)
                else:
                    # This assest wont be removed. Add its id to the
                    # global id collection.
                    self._assetId2Path[self.assets[asset].id()] = asset
                    pass
                pass

            for removedAsset in removedAssets:
                self.removeAsset(removedAsset, True)
                pass

            self.updateAllAssets()
            self.saveProject()

            AssetFolderWatcher.WatchFolder(self.getProjectFolder())
            pass
        pass

    # Returns the absolute path of a file that belongs to a project
    def getAbsProjFilePath(self, relativePath):
        return self.getProjectFolder()+'/'+relativePath

    def isUserAsset(self, assetPath):
        return Utils.IsSubdir(assetPath, 'assets/')

    def isUserScriptFactory(self, assetPath):
        return Utils.IsSubdir(assetPath, 'default_assets/factories/')

    def getProjectFolder(self):
        import os

        if self.project_path == None:
            return None

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
        Utils.Console.info('Saving project...')
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
            scene = json.loads(sceneJson)
            # Make sure that all component ids are correct -- they might have
            # changed due to the user updating files with the engine closed
            inconsistencyDetected = False
            for gameObj in scene:
                for component in gameObj['components']:
                    if 'path' in component:
                        # TODO remove component if its source file is missing
                        if component['id'] != self.assets[component['path']].id():
                            component['id'] = self.assets[component['path']].id()
                            inconsistencyDetected = True
                            pass
                        pass
                    pass
                pass
            if inconsistencyDetected:
                f = open(fname, 'w')
                f.write(json.dumps(scene))
                f.close()
                pass

            return scene
        except IOError:
            # TODO do something if scene file is gone
            Utils.Console.error('Could not load scene file '+fname)
            return None
            pass
        pass

    def updateAllAssets(self):
        self._parallelAssetUpdate(self.assets)
        pass

    def getAssetNameList(self):
        result = []
        for asset in self.assets:
            result.append(asset)
            pass
        return result

    def getAssetProcessors(self):
        return self.assets

    def getAssetList(self):
        if self.project_path == None:
            return []

        # Update all outdated assets in parallel
        self.updateAllAssets()

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
            self._updateAssetAndDependencies(assetPath)

            if saveProject:
                self.saveProject()
        pass

    def _parallelAssetUpdate(self, filesToUpdate):
        # If a job was dispatched previously, wait for it to finish
        self._assetUpdaterQueue.join()

        def assetUpdaterThread():
            while not self._assetUpdaterQueue.empty():
                work = self._assetUpdaterQueue.get()
                work.update()
                self._assetUpdaterQueue.task_done()
                pass
            pass

        # Update all dependent assets
        for f in filesToUpdate:
            # Make sure f is still in self.assets
            if f in self.assets:
                self._assetUpdaterQueue.put(self.assets[f])
                pass
            pass

        for i in range(max(1,multiprocessing.cpu_count()/2)):
            thread = threading.Thread(target=assetUpdaterThread)
            thread.start()
            pass

        self._assetUpdaterQueue.join()
        pass

    def _updateAssetAndDependencies(self, assetPath):
        # List all files that depend directly or indirectly on this
        searchQueue = set([assetPath])
        filesToUpdate = set()
        while len(searchQueue) > 0:
            nextFile = searchQueue.pop()
            filesToUpdate.add(nextFile)
            for asset in self.assets:
                if (self.assets[asset].dependsOn(nextFile) or self.assets[asset].isBroken) and not (asset in filesToUpdate):
                    searchQueue.add(asset)
                    pass
                pass
            pass

        self._parallelAssetUpdate(filesToUpdate)
        pass

    def processAsset(self, assetPath, saveProject):
        from server.parser import CppParser
        from server.build import BuildEventHandler

        if not (assetPath in self.assets):
            # Handle CREATE cases
            assetProc = AssetProcessor.CreateAssetProcessor(assetPath)

            if assetProc == None:
                return None

            self.assets[assetPath] = assetProc
        else:
            assetProc = self.assets[assetPath]
            pass

        self._updateAssetAndDependencies(assetPath)
        editorData = assetProc.getMetadata()

        # After done processing all assets, save the project, if requested
        if saveProject:
            self.saveProject()
            pass

        return editorData

    # filePath must be absolute, and assetPath, relative
    def isFileOlderThanDependency(self, filePath, assetPath):
        # If file doesnt even exist, then it must be touched to begin with
        if not Utils.FileExists(filePath):
            return True
        # If the asset itself is newer than the filePath, return true
        queryFileMTime = os.path.getmtime(filePath)
        if os.path.getmtime(self.getAbsProjFilePath(assetPath)) > queryFileMTime:
            return True

        # If any of the asset dependencies are newer than the filePath, return true
        if assetPath in self.assets:
            if self.assets[assetPath].mostRecentDependencyTime() > queryFileMTime:
                return True
            pass

        return False
    pass

_projectLock = threading.Lock()

def getAssetNameList():
    global _currentProject
    return _currentProject.getAssetNameList()

def getAssetList():
    global _currentProject
    return _currentProject.getAssetList()

def getAssetProcessors():
    global _currentProject
    return _currentProject.getAssetProcessors()

def getProjectFolder():
    global _currentProject
    return _currentProject.getProjectFolder()

def getAbsProjFilePath(path):
    global _currentProject
    return _currentProject.getAbsProjFilePath(path)

def createNewProject(path):
    global _currentProject
    global _currentProject
    _currentProject = _Project()
    _currentProject.bootstrap(path)
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
    global _projectLock
    _projectLock.acquire()
    assetData = _currentProject.processAsset(assetPath, saveProject)
    _projectLock.release()
    return assetData

def removeAsset(assetPath, saveProject):
    global _currentProject
    global _projectLock
    _projectLock.acquire()
    _currentProject.removeAsset(assetPath, saveProject)
    _projectLock.release()

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

def fileExists(relativeFilePath):
    return Utils.FileExists(getAbsProjFilePath(relativeFilePath))

def getRelProjFilePath(absoluteFilePath):
    return os.path.relpath(absoluteFilePath, getProjectFolder())

def removeFile(relativeFilePath):
    Utils.RemoveFile(getAbsProjFilePath(relativeFilePath))

def getUniqueAssetId():
    global _currentProject
    return _currentProject.getUniqueAssetId()

def getModificationTime(path):
    return os.path.getmtime(getAbsProjFilePath(path))

_currentProject = None
def initialize():
    global _currentProject
    if len(Config.get('runtime', 'recent_projects')) > 0:
        # Load last opened project
        _currentProject = _Project()
        _currentProject.initialize(Config.get('runtime', 'recent_projects')[0])
    else:
        # Just create an empty project
        _currentProject = _Project()
        _currentProject.initialize()
        pass


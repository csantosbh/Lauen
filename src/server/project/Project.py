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
            self.scripts = {}
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

            self.scripts = projInfo['scripts']
            self.scenes = projInfo['scenes']
            self.currentScene = projInfo['currentScene']
            # TODO start watchdog at the assets folder
            pass
        self._scriptId2Path = dict()
        pass

    def getScriptId(self, scriptPath):
        from random import randint
        if not scriptPath in self.scripts:
            candidateId = randint(0,1e9)
            while candidateId in self._scriptId2Path:
                candidateId = randint(0,1e9)
                pass
            self.scripts[scriptPath] = candidateId
            self._scriptId2Path[candidateId] = scriptPath
            self.saveProject()
            pass

        return self.scripts[scriptPath]

    def saveProject(self):
        import json
        with open(self.project_path, 'w') as fhandle:
            fhandle.write(json.dumps(dict(
                scripts=self.scripts,
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

        # TODO return other asset types as well
        project_folder = os.path.dirname(self.project_path)
        return Utils.ParseHPPFilesFromFolder(project_folder+'/assets')
    pass

def getAssetList():
    global _currentProject
    return _currentProject.getAssetList()

def getProjectFolder():
    global _currentProject
    return os.path.dirname(_currentProject.project_path)

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

# Load last opened project
if len(Config.get('runtime', 'recent_projects')) > 0:
    _currentProject = _Project(Config.get('runtime', 'recent_projects')[0])
else:
    _currentProject = _Project()
    pass


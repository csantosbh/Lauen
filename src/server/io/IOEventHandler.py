from server import Event, RPC
from server.project import Project

def save(sceneData):
    # TODO define the scene name from the editor
    return Project.saveCurrentScene(sceneData)

def getAssetList(evData):
    # Load list of assets
    return Project.getAssetList()

def loadCurrentScene(e):
    return Project.loadCurrentScene()

def savePrefab(prefab):
    return Project.savePrefab(prefab)

RPC.listen(save)
RPC.listen(loadCurrentScene)
RPC.listen(getAssetList)
RPC.listen(savePrefab)

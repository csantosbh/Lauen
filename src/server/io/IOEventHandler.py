from server import Event
from server import RPC
from server.project import Project
from server import WebSocketServer

def save(sceneData):
    # TODO define the scene name from the editor
    return Project.saveCurrentScene(sceneData)

def getAssetList(evData):
    # Load list of assets
    return Project.getAssetList()

def loadCurrentScene(e):
    return Project.loadCurrentScene()

RPC.listen(save)
RPC.listen(loadCurrentScene)
RPC.listen(getAssetList)

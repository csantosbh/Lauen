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

def createPrefab(prefab):
    import json
    with open(Project.getProjectFolder() + '/assets/'+prefab['name']+'.prefab', 'w') as f:
        f.write(json.dumps(prefab))
        pass
    return True

RPC.listen(save)
RPC.listen(loadCurrentScene)
RPC.listen(getAssetList)
RPC.listen(createPrefab)

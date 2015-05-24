from server import Event
from server import Project
from server import WebSocketServer

def saveScene(sceneData):
    import json
    # TODO define the output name from the editor
    fname = Project.getProjectFolder() + '/scene.json'

    f = open(fname, 'w')
    f.write(json.dumps(sceneData))
    f.close()
    pass

def _loadAssetList():
    from server.io import Utils

    # Load list of assets
    WebSocketServer.send('assetlist', dict(files=Utils.ParseHPPFilesFromFolder(Project.getProjectFolder()+'/assets')))
    pass

def _loadCurrentScene():
    # TODO define the output name from the editor
    fname = Project.getProjectFolder() + '/scene.json'

    try:
        f = open(fname, 'r')
        sceneJson = f.read()
        f.close()
        WebSocketServer.send('loadCurrentScene', sceneJson)
    except IOError:
        # TODO do something if project file is gone
        pass

    pass

def loadEditor(e):
    _loadAssetList()
    _loadCurrentScene()
    pass

Event.listen('save', saveScene)
Event.listen('clientConnected', loadEditor)

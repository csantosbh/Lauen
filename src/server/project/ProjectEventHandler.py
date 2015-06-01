from Tkinter import Tk
from ttk import Style
import tkFileDialog

from server import RPC, Config
from server.build import BuildEventHandler
import Project

root = Tk()
root.style = Style()
root.style.theme_use('clam')
root.withdraw()

def getRecentProjects(evData):
    return Config.get('runtime', 'recent_projects')

def _pushProjectToRecentList(path):
    recent_projects = Config.get('runtime', 'recent_projects')
    try:
        # Remove current folder from path list, if it's there
        recent_projects.remove(path)
    except:
        pass

    recent_projects.insert(0, path)
    if len(recent_projects) > Config.get('project', 'recent_project_history'):
        recent_projects.pop(-1)
        pass
    Config.set('runtime', 'recent_projects', recent_projects)
    pass

def loadProject(evData):
    if evData:
        in_path = evData
    else:
        in_path = tkFileDialog.askopenfilename()
        pass

    if len(in_path) != 0:
        _pushProjectToRecentList(in_path)
        # Load project!
        return Project.loadProject(in_path)
    return False

def createNewProject(evData):
    in_path = tkFileDialog.askopenfilename(initialfile='project.json')

    if len(in_path)!=0:
        _pushProjectToRecentList(in_path)

        # Create new project!
        Project.createNewProject(in_path)
        pass

    return in_path

def exportGame(params):
    out_path = tkFileDialog.askdirectory()

    if len(out_path)!=0:
        return BuildEventHandler.ExportGame(params['buildAndRun'], params['compilationMode'], out_path)
    return False
    pass

RPC.listen(createNewProject)
RPC.listen(getRecentProjects)
RPC.listen(loadProject)
RPC.listen(exportGame)

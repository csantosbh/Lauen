from server import RPC
from server import Config

def getRecentProjects(evData):
    return Config.get('runtime', 'recent_projects')

def createNewProject(evData):
    from Tkinter import Tk
    from ttk import Style
    import tkFileDialog
    root=Tk()
    root.style=Style()
    root.style.theme_use('clam')
    root.withdraw()
    in_path = tkFileDialog.askdirectory()

    if len(in_path)!=0:
        # Create new project!
        recent_projects = Config.get('runtime', 'recent_projects')
        recent_projects.insert(0, in_path)
        if len(recent_projects) > Config.get('project', 'recent_project_history'):
            recent_projects.pop(-1)
            pass
        Config.set('runtime', 'recent_projects', recent_projects)
        pass

    return in_path

RPC.listen(createNewProject)
RPC.listen(getRecentProjects)

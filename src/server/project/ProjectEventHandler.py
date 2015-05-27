from server import RPC

def getRecentProjects(evData):
    return ['/home/csantos/workspace/LauEngine/']

def createNewProject(evData):
    from Tkinter import Tk
    from ttk import Style
    import tkFileDialog
    root=Tk()
    root.style=Style()
    root.style.theme_use('clam')
    root.withdraw()
    in_path = tkFileDialog.askdirectory()
    return in_path

RPC.listen(createNewProject)
RPC.listen(getRecentProjects)

from server import Event
from server.project import Project
from server import WebSocketServer
from server import io

link_flags='-rdynamic -lglfw3 -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp -lGLEW'
cxx_compiler='g++'

def generateComponentFactory(componentFiles):
    from mako.template import Template

    project_folder = Project.getProjectFolder()

    template = open(project_folder+'/default_assets/Factories.hpy').read()
    with open(project_folder+'/default_assets/Factories.hpp', 'w') as outputHandle:
        outputHandle.write(Template(template).render(components=componentFiles))
        pass
    pass

def run_game(path, workFolder):
    from subprocess import Popen, PIPE
    from time import sleep

    process = Popen(path, stdout=PIPE, stderr=PIPE, cwd=workFolder)
    strMsg = ''
    while process.returncode == None:
        process.poll()
        strMsg = process.stdout.read()
        print strMsg
        WebSocketServer.send('executionMessage', strMsg)
        strMsg = ''
        sleep(0.1)
        pass
    pass

def build_game(event_msg):
    import subprocess

	# TODO get third_party folder from config (not saved, maybe detect at runtime or installation time)
    cxx_flags='-I/home/csantos/workspace/LauEngine/third_party/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/'

    project_folder = Project.getProjectFolder()
    compilationStatus = dict(returncode=0, message='')
    try:
        # TODO use threads
        # TODO incremental build will solve this, but I need to perform dependency checking besides modification time comparison
        componentScripts = io.Utils.ParseHPPFilesFromFolder(project_folder+'/assets')

        # Generate component factory
        generateComponentFactory(componentScripts)
        
        # Generate build list
        sourceFiles = []
        # Append component scripts to the list of build files
        """
        for componentScript in componentScripts:
            sourceFiles.append(componentScript['path'])
            pass
        """
        # Append default assets to the list of build files
        for defaultAsset in io.Utils.ListFilesFromFolder(project_folder+'/default_assets', ['cpp', 'cxx']):
            sourceFiles.append(defaultAsset)
            pass

        # Compile
        precompiledFiles = ''
        for sourceFile in sourceFiles:
            precompiledFile = project_folder+'/build/'+io.Utils.GetFileNameFromPath(sourceFile)+'.o '
            compilationStatus['message'] += subprocess.check_output(cxx_compiler + ' -c ' + sourceFile +' -o '+precompiledFile + cxx_flags, shell=True, stderr=subprocess.STDOUT)
            precompiledFiles += precompiledFile
            pass

        # Link
        compilationStatus['message'] += subprocess.check_output(cxx_compiler + ' ' + precompiledFiles +' -o '+project_folder+'/game ' + link_flags, shell=True, stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as e:
        # TODO show compilation error messages on console
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
        print 'Compilation error: ', e.returncode
        pass

    WebSocketServer.send('compilationStatus', compilationStatus)

    if compilationStatus['returncode'] == 0:
        # TODO use threads
        run_game(project_folder+'/game', project_folder)
    pass

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

Event.listen('build', build_game)

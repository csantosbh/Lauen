from server import RPC, WebSocketServer, io
from server.project import Project
from server.components import DefaultComponentManager

link_flags='-rdynamic -lglfw3 -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp -lGLEW'
cxx_compiler='g++'
cxx_mode_flags={
    'DEBUG': ' -g',
    'RELEASE': ' -O3'
}

# TODO only re-call this when we change the number of scripts available
def generateComponentFactory(componentFiles):
    from mako.template import Template

    project_folder = Project.getProjectFolder()

    template = open(project_folder+'/default_assets/Factories.hpy').read()
    with open(project_folder+'/default_assets/Factories.hpp', 'w') as outputHandle:
        outputHandle.write(Template(template).render(components=componentFiles, default_components=DefaultComponentManager.getDefaultComponents()))
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

def buildGame(event_msg, runGame = True, compilationMode='DEBUG', outputFolder = None):
    import subprocess
    # TODO get third_party folder from config (not saved, maybe detect at runtime or installation time)
    cxx_flags=' -I/home/csantos/workspace/LauEngine/third_party/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/' + cxx_mode_flags[compilationMode]

    project_folder = Project.getProjectFolder()

    if outputFolder == None:
        outputFolder = project_folder

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
        compilationModeFlags = ' -D'+compilationMode
        precompiledFiles = ''
        for sourceFile in sourceFiles:
            precompiledFile = outputFolder+'/build/'+io.Utils.GetFileNameFromPath(sourceFile)+'.o '
            compilationStatus['message'] += subprocess.check_output(cxx_compiler + ' -c ' + sourceFile +' -o '+precompiledFile + compilationModeFlags + cxx_flags, shell=True, stderr=subprocess.STDOUT)
            precompiledFiles += precompiledFile
            pass

        # Link
        compilationStatus['message'] += subprocess.check_output(cxx_compiler + ' ' + precompiledFiles +' -o '+outputFolder+'/game ' + link_flags, shell=True, stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as e:
        # TODO show compilation error messages on console
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
        print 'Compilation error: ', e.returncode
        pass

    WebSocketServer.send('compilationStatus', compilationStatus)

    if compilationStatus['returncode'] == 0 and runGame:
        # TODO use threads
        run_game(outputFolder+'/game', outputFolder)
        pass

    return compilationStatus
    pass

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

def ExportGame(buildAndRun, compilationMode, outputFolder):
    from distutils import dir_util
    from server import Config
    import shutil, os
    # Create folder for temporary .o files
    dir_util.mkpath(outputFolder + '/build')
    buildGame(None, buildAndRun, compilationMode, outputFolder)
    # Copy scenes to destination folder
    project_folder = Project.getProjectFolder()
    dir_util.copy_tree(project_folder + '/scenes', outputFolder+'/scenes')
    # Copy assets to destination folder
    io.Utils.CopyFilesOfTypes(project_folder+'/assets', outputFolder, Config.env('asset_extensions'), project_folder)
    io.Utils.CopyFilesOfTypes(project_folder+'/default_assets', outputFolder, Config.env('asset_extensions'), project_folder)
    # Remove temporary build folder
    dir_util.remove_tree(outputFolder + '/build')
    return True
    pass

RPC.listen(buildGame)

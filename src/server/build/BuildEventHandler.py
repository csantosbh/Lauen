from server import RPC, WSServer, io, Config
from server.project import Project
from server.components import DefaultComponentManager

link_flags={
    'linux': '-rdynamic -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp -lGLEW',
    # TODO get third_party folder from config (not saved, maybe detect at runtime or installation time)
    'windows': '-lglew32 -lglfw3 -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32 -mwindows -L /home/csantos/workspace/LauEngine/third_party/cross_compiling/windows/glew-1.12.0/lib/ -L /home/csantos/workspace/LauEngine/third_party/cross_compiling/windows/glfw-3.1.1/build/src/',
    # TODO the -L depends on the cxx_mode flag (RELEASE/DEBUG)
    'nacl': '-L/home/csantos/workspace/nacl_sdk/pepper_41/lib/pnacl/Debug -lppapi_cpp -lppapi -lppapi_gles2'
}
cxx_preprocessors={
    'linux': '-DLINUX -DDESKTOP',
    'windows': '-DLINUX -DDESKTOP',
    'nacl': '-DNACL',
}
cxx_compiler={
    'linux': 'g++',
    'windows': Config.get('export', 'win_compilers')['g++'],
    'nacl': Config.get('export', 'nacl')['pepper_folder']+'/'+Config.get('export', 'nacl')['compiler'],
}

def _cxx_flags(compilationMode):
    cxx_mode_flags={
            'DEBUG': ' -g',
            'RELEASE': ' -O3'
    }
    return {
        'linux': ' -I/home/csantos/workspace/LauEngine/third_party/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/' + cxx_mode_flags[compilationMode],
        'windows': ' -I/home/csantos/workspace/LauEngine/third_party/rapidjson/include -I /home/csantos/workspace/LauEngine/third_party/cross_compiling/windows/glfw-3.1.1/include/ -I /home/csantos/workspace/LauEngine/third_party/cross_compiling/windows/glew-1.12.0/include/ -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/',
        'nacl': ' -I/home/csantos/workspace/LauEngine/third_party/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+'/default_assets/ -I' + Config.get('export', 'nacl')['pepper_folder']+'/include' + cxx_mode_flags[compilationMode],
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
    while process.returncode == None:
        process.poll()
        strMsg = process.stdout.readline()
        if len(strMsg)>0:
            WSServer.send('executionMessage', strMsg)
            pass
        pass
    pass

def buildGame(event_msg, platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None):
    import subprocess
    # TODO get third_party folder from config (not saved, maybe detect at runtime or installation time)
    cxx_flags=_cxx_flags(compilationMode)

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
            compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' -c ' + sourceFile +' -o '+precompiledFile + compilationModeFlags + ' ' + cxx_preprocessors[platform] + ' ' + cxx_flags[platform], shell=True, stderr=subprocess.STDOUT)
            precompiledFiles += precompiledFile
            pass

        # Link
        compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' ' + precompiledFiles +' -o '+outputFolder+'/game ' + link_flags[platform], shell=True, stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as e:
        # TODO show compilation error messages on console
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
        print 'Compilation error: ', e.returncode
        pass

    WSServer.send('compilationStatus', compilationStatus)

    if compilationStatus['returncode'] == 0 and runGame:
        # TODO use threads
        run_game(outputFolder+'/game', outputFolder)
        pass

    return compilationStatus
    pass

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

def _PostExportStep(platform, outputFolder):
    import subprocess
    if platform == 'windows':
        # TODO third_party folder must come from config
        io.Utils.CopyFilesOfTypes('/home/csantos/workspace/LauEngine/third_party/cross_compiling/windows/redist', outputFolder, ['.dll'])
        pass
    elif platform == 'nacl':
        # TODO finalize build
        subprocess.check_output('/home/csantos/workspace/nacl_sdk/pepper_41/toolchain/linux_pnacl/bin/pnacl-finalize '+outputFolder+'/game -o '+outputFolder+'/lau_canvas.pexe', shell=True)
        pass
    pass

def ExportGame(platform, buildAndRun, compilationMode, outputFolder):
    from distutils import dir_util
    import shutil, os
    # Create folder for temporary .o files
    dir_util.mkpath(outputFolder + '/build')
    if buildGame(None, platform, buildAndRun, compilationMode, outputFolder)['returncode'] != 0:
        return False
    else:
        # Copy scenes to destination folder
        project_folder = Project.getProjectFolder()
        dir_util.copy_tree(project_folder + '/scenes', outputFolder+'/scenes')
        # Copy assets to destination folder
        io.Utils.CopyFilesOfTypes(project_folder+'/assets', outputFolder, Config.env('asset_extensions'), project_folder)
        io.Utils.CopyFilesOfTypes(project_folder+'/default_assets', outputFolder, Config.env('asset_extensions'), project_folder)
        # Platform specific post-build steps
        _PostExportStep(platform, outputFolder)
        # Remove temporary build folder
        dir_util.remove_tree(outputFolder + '/build')
        return True
    pass

def previewGame(event_msg):
    ExportGame('nacl', False, 'DEBUG', Project.getProjectFolder()+'/build/nacl/')
    return True

RPC.listen(buildGame)
RPC.listen(previewGame)

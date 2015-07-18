from server import RPC, WSServer, io, Config
from server.project import Project
from server.components import DefaultComponentManager

platform_preprocessors={
    'linux': '-DLINUX -DDESKTOP',
    'windows': '-DLINUX -DDESKTOP',
    'nacl': '-DNACL',
    'preview': '-DNACL -DPREVIEW_MODE',
}
cxx_compiler={
    'linux': 'g++',
    'windows': Config.get('export', 'win_compilers')['g++'],
    'nacl': Config.get('export', 'nacl')['pepper_folder']+'/'+Config.get('export', 'nacl')['compiler'],
    'preview': Config.get('export', 'nacl')['pepper_folder']+'/'+Config.get('export', 'nacl')['compiler'],
}

def _getFlags(compilationMode):
    cxxModeFlags = {
        'DEBUG': ' -g -DDEBUG',
        'RELEASE': ' -O3 -DRELEASE'
    }
    modeFolder = {
        'DEBUG': 'Debug',
        'RELEASE': 'Release'
    }
    thirdPartyFolder = Config.get('export', 'third_party_folder')
    naclFolder = Config.get('export', 'nacl')['pepper_folder']
    return {
        'link_flags': {
            'linux': '-rdynamic -lglfw3 -lrt -lXrandr -lXinerama -lXi -lXcursor -lGL -lm -ldl -lXrender -ldrm -lXdamage -lX11-xcb -lxcb-glx -lxcb-dri2 -lxcb-dri3 -lxcb-present -lxcb-sync -lxshmfence -lXxf86vm -lXfixes -lXext -lX11 -lpthread -lxcb -lXau -lXdmcp -lGLEW',
            # TODO check licenses of libraries so I can determine if I can install their binaries with the engine (which means these folders could be specified as relatives to the instalation folder instead of having to set them in the config)
            'windows': '-lglew32 -lglfw3 -lglu32 -lopengl32 -lgdi32 -luser32 -lkernel32 -mwindows -L '+thirdPartyFolder+'/cross_compiling/windows/glew-1.12.0/lib/ -L '+thirdPartyFolder+'/cross_compiling/windows/glfw-3.1.1/build/src/',
            # TODO Add the Native Client target to the export menu on the editor
            'nacl': '-L'+naclFolder+'/lib/pnacl/'+modeFolder[compilationMode]+' -lppapi_cpp -lppapi -lppapi_gles2 -lm',
            'preview': '-L'+naclFolder+'/lib/pnacl/'+modeFolder[compilationMode]+' -lppapi_cpp -lppapi -lppapi_gles2 -lm',
        },
        'cxx_flags': {
            'linux': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/' + cxxModeFlags[compilationMode],
            'windows': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -I '+thirdPartyFolder+'/cross_compiling/windows/glfw-3.1.1/include/ -I '+thirdPartyFolder+'/cross_compiling/windows/glew-1.12.0/include/ -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/',
            'nacl': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include' + cxxModeFlags[compilationMode],
            'preview': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include' + cxxModeFlags[compilationMode],
        }
    }

# TODO only re-call this when we change the number of scripts available, or when a script flyweight changes
def _isVecType(type):
    return type == 'v4f' or type == 'v3f' or type == 'v2f'

def _renderTemplateSources(componentFiles):
    from mako.template import Template

    projectFolder = Project.getProjectFolder()

    templateFiles = ['Factories.cpy', 'Peekers.cpy']

    renderParameters = dict(components=componentFiles,
                            default_components=DefaultComponentManager.getDefaultComponents(),
                            isVecType=_isVecType,
                            vecIterations=dict(v4f=4, v3f=3, v2f=2))

    for templateFile in templateFiles:
        componentFactoryTemplate = open(projectFolder+'/default_assets/'+templateFile).read()
        outputName = templateFile[:templateFile.rfind('.')] + '.cpp'
        with open(projectFolder+'/default_assets/'+outputName, 'w') as outputHandle:
            outputHandle.write(Template(componentFactoryTemplate).render(**renderParameters))
            pass
        pass
    pass

def _runGame(path, workFolder):
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

def BuildProject(platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None):
    import subprocess
    compilationFlags=_getFlags(compilationMode)

    projectFolder = Project.getProjectFolder()

    if outputFolder == None:
        outputFolder = projectFolder

    compilationStatus = dict(returncode=0, message='')
    try:
        # TODO use threads
        # TODO incremental build will solve this, but I need to perform dependency checking besides modification time comparison
        componentScripts = io.Utils.ParseHPPFilesFromFolder(projectFolder+'/assets')

        # Generate component factory
        _renderTemplateSources(componentScripts)
        
        # Generate build list
        sourceFiles = []
        # Append component scripts to the list of build files
        """
        for componentScript in componentScripts:
            sourceFiles.append(componentScript['path'])
            pass
        """
        # Append default assets to the list of build files
        for defaultAsset in io.Utils.ListFilesFromFolder(projectFolder+'/default_assets', ['cpp', 'cxx']):
            sourceFiles.append(defaultAsset)
            pass

        # Compile
        compilationModeFlags = ' -D'+compilationMode
        precompiledFiles = ''
        for sourceFile in sourceFiles:
            precompiledFile = outputFolder+'/build/'+io.Utils.GetFileNameFromPath(sourceFile)+'.o '
            compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' -c ' + sourceFile +' -o '+precompiledFile + compilationModeFlags + ' ' + platform_preprocessors[platform] + ' ' + compilationFlags['cxx_flags'][platform], shell=True, stderr=subprocess.STDOUT)
            precompiledFiles += precompiledFile
            pass

        # Link
        compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' ' + precompiledFiles +' -o '+outputFolder+'/game ' + compilationFlags['link_flags'][platform], shell=True, stderr=subprocess.STDOUT)

    except subprocess.CalledProcessError as e:
        # TODO show compilation error messages on console
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
        print 'Compilation error: ', e.returncode
        pass

    WSServer.send('compilationStatus', compilationStatus)

    if compilationStatus['returncode'] == 0 and runGame:
        # TODO use threads
        _runGame(outputFolder+'/game', outputFolder)
        pass

    return compilationStatus
    pass

def buildGame(event_msg):
    BuildProject()

def AutoBuild():
    # TODO: Watch for file modifications and re-generate .o's
    pass

def _PostExportStep(platform, outputFolder):
    import subprocess
    if platform == 'windows':
        thirdPartyFolder = Config.get('export', 'third_party_folder')
        io.Utils.CopyFilesOfTypes(thirdPartyFolder+'/cross_compiling/windows/redist', outputFolder, ['.dll'])
        pass
    elif platform == 'nacl' or platform == 'preview':
        naclFolder = Config.get('export', 'nacl')['pepper_folder']
        subprocess.check_output(naclFolder+'/toolchain/linux_pnacl/bin/pnacl-finalize '+outputFolder+'/game -o '+outputFolder+'/lau_canvas.pexe', shell=True)
        pass
    pass

def ExportGame(platform, buildAndRun, compilationMode, outputFolder):
    from distutils import dir_util
    import shutil, os
    # Create folder for temporary .o files
    dir_util.mkpath(outputFolder + '/build')
    if BuildProject(platform, buildAndRun, compilationMode, outputFolder)['returncode'] != 0:
        return False
    else:
        # Copy scenes to destination folder
        projectFolder = Project.getProjectFolder()
        dir_util.copy_tree(projectFolder + '/scenes', outputFolder+'/scenes')
        # Copy assets to destination folder
        io.Utils.CopyFilesOfTypes(projectFolder+'/assets', outputFolder, Config.env('exportable_asset_extensions'), projectFolder)
        io.Utils.CopyFilesOfTypes(projectFolder+'/default_assets', outputFolder, Config.env('exportable_asset_extensions'), projectFolder)
        # Platform specific post-build steps
        _PostExportStep(platform, outputFolder)
        # Remove temporary build folder
        dir_util.remove_tree(outputFolder + '/build')
        return True
    pass

def previewGame(event_msg):
    ExportGame('preview', False, 'DEBUG', Project.getProjectFolder()+'/build/nacl/')
    return True

RPC.listen(buildGame)
RPC.listen(previewGame)

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
        'DEBUG': '-g -DDEBUG',
        'RELEASE': '-O3 -DRELEASE'
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
            'linux': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/ ' + cxxModeFlags[compilationMode],
            'windows': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -I '+thirdPartyFolder+'/cross_compiling/windows/glfw-3.1.1/include/ -I '+thirdPartyFolder+'/cross_compiling/windows/glew-1.12.0/include/ -std=c++11 -I'+Project.getProjectFolder()+'/default_assets/',
            'nacl': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include ' + cxxModeFlags[compilationMode],
            'preview': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include ' + cxxModeFlags[compilationMode],
        }
    }

def _isVecType(type):
    return type == 'v4f' or type == 'v3f' or type == 'v2f'

def RenderFactorySources(componentFiles):
    from mako.template import Template

    projectFolder = Project.getProjectFolder()
    templatePath = projectFolder+'/default_assets/factories/FactoryTemplates.cpy'
    for component in componentFiles:
        renderParameters = dict(component=component,
                                isVecType=_isVecType,
                                vecIterations=dict(v4f=4, v3f=3, v2f=2))
        fname = io.Utils.GetFileNameFromPath(component['path'])
        outputPath = projectFolder+'/default_assets/factories/'+fname[:fname.rfind('.')] +'_'+str(component['id']) + '.cpp'

        # Only update the auto-generated files when their CPY sources have been
        # updated, or when one of its dependencies have been updated.
        if Project.isFileOlderThanDependency(outputPath, outputPath):
            print 'Updating template '+outputPath
            componentFactoryTemplate = open(templatePath).read()
            with open(outputPath, 'w') as outputHandle:
                outputHandle.write(Template(componentFactoryTemplate).render(**renderParameters))
                pass
            pass
        pass
    pass

def _renderTemplateSources(componentFiles):
    from mako.template import Template

    projectFolder = Project.getProjectFolder()
    RenderFactorySources(componentFiles)

    # TODO deprecate these templates below, use regular cpp files instead
    renderParameters = dict(default_components=DefaultComponentManager.getDefaultComponents(),
                            vecIterations=dict(v4f=4, v3f=3, v2f=2))
    templateFiles = ['Factories.cpy', 'Peekers.cpy']
    for templateFile in templateFiles:
        templatePath = projectFolder+'/default_assets/'+templateFile
        outputPath = projectFolder+'/default_assets/'+templateFile[:templateFile.rfind('.')] + '.cpp'

        # Only update the auto-generated files when their CPY sources have been
        # updated, or when one of its dependencies have been updated.
        if Project.isCPYTemplateOutdated(templatePath):
            print 'Updating template '+templatePath
            componentFactoryTemplate = open(templatePath).read()
            with open(outputPath, 'w') as outputHandle:
                outputHandle.write(Template(componentFactoryTemplate).render(**renderParameters))
                pass
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

def _buildObjectFile(sourceFile, outputFolder, platform, compilationFlags):
    import subprocess
    outputFilePath = outputFolder+'/build/'+io.Utils.GetFileNameFromPath(sourceFile)+'.o'
    success=True
    compilationMessage = ''
    returncode=0

    try:
        # Only build the object file if it is older than any of its dependencies
        if Project.isFileOlderThanDependency(outputFilePath, sourceFile):
            print 'building '+outputFilePath
            compilationMessage = subprocess.check_output(cxx_compiler[platform] + ' -c ' + sourceFile +' -o '+outputFilePath +' '+ platform_preprocessors[platform] + ' ' + compilationFlags['cxx_flags'][platform], shell=True, stderr=subprocess.STDOUT)
            pass
    except subprocess.CalledProcessError as e:
        compilationMessage = e.output
        returncode = e.returncode
        success = False
        pass

    return dict(output=outputFilePath, message=compilationMessage, returncode=returncode)

def BuildProject(platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None):
    import subprocess
    compilationFlags=_getFlags(compilationMode)

    projectFolder = Project.getProjectFolder()

    if outputFolder == None:
        outputFolder = projectFolder

    compilationStatus = dict(returncode=0, message='')
    try:
        # Generate component factory
        _renderTemplateSources(Project.getAssetList())
        
        # Generate build list
        sourceFiles = []
        componentScripts = io.Utils.ListFilesFromFolder(projectFolder)
        # Append component scripts to the list of build files
        for componentScript in componentScripts:
            if io.Utils.IsImplementationFile(componentScript):
                sourceFiles.append(componentScript)
                pass
            pass

        # Compile
        precompiledFiles = ''
        for sourceFile in sourceFiles:
            buildStatus = _buildObjectFile(sourceFile, outputFolder, platform, compilationFlags)
            compilationStatus['returncode'] = buildStatus['returncode']
            compilationStatus['message'] += buildStatus['message']

            if compilationStatus['returncode'] != 0:
                break

            precompiledFiles += ' '+buildStatus['output']

            pass

        # Link
        # TODO only link if the final executable is older than any of the object files
        print 'linking...'
        if compilationStatus['returncode'] == 0:
            compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' ' + precompiledFiles +' -o '+outputFolder+'/game ' + compilationFlags['link_flags'][platform], shell=True, stderr=subprocess.STDOUT)
            pass
        print 'linking done.'

    except subprocess.CalledProcessError as e:
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
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

def ExportGame(platform, buildAndRun, compilationMode, outputFolder, cleanObjects=True):
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
        if cleanObjects:
            dir_util.remove_tree(outputFolder + '/build')
            pass
        return True
    pass

def previewGame(event_msg):
    ExportGame('preview', False, 'DEBUG', Project.getProjectFolder()+'/build/nacl/', cleanObjects=False)
    return True

def BuildPreviewObject(inputFile):
    buildStatus = _buildObjectFile(inputFile, Project.getProjectFolder()+'/build/nacl/', 'preview', _getFlags('DEBUG'))
    WSServer.send('compilationStatus', buildStatus)
    return buildStatus

RPC.listen(buildGame)
RPC.listen(previewGame)

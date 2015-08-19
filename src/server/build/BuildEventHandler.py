import os
import threading
import multiprocessing
import Queue

from server import RPC, WSServer, Config
from server.io import Utils
from server.project import Project
from server.components import DefaultComponentManager

platform_preprocessors={
    'linux': '-DLINUX -DDESKTOP',
    'windows': '-DWINDOWS -DDESKTOP',
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
            'nacl': '-L'+naclFolder+'/lib/pnacl/'+modeFolder[compilationMode]+' -lppapi_cpp -lppapi -lppapi_gles2 -lm -fms-extensions',
            'preview': '-L'+naclFolder+'/lib/pnacl/'+modeFolder[compilationMode]+' -lppapi_cpp -lppapi -lppapi_gles2 -lm -fms-extensions',
        },
        'cxx_flags': {
            'linux': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=c++11 -I'+Project.getProjectFolder()+' -I'+Project.getProjectFolder()+'/default_assets/ ' + cxxModeFlags[compilationMode],
            'windows': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -I '+thirdPartyFolder+'/cross_compiling/windows/glfw-3.1.1/include/ -I '+thirdPartyFolder+'/cross_compiling/windows/glew-1.12.0/include/ -std=c++11 -I'+Project.getProjectFolder()+' -I'+Project.getProjectFolder()+'/default_assets/',
            'nacl': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+' -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include -fms-extensions ' + cxxModeFlags[compilationMode],
            'preview': ' -I'+thirdPartyFolder+'/Eigen -I'+thirdPartyFolder+'/rapidjson/include -std=gnu++11 -I'+Project.getProjectFolder()+' -I'+Project.getProjectFolder()+'/default_assets/ -I' + naclFolder+'/include -fms-extensions ' + cxxModeFlags[compilationMode],
        }
    }

def _isVecType(type):
    return type == 'v4f' or type == 'v3f' or type == 'v2f'

def RenderFactorySources(componentFiles):
    from mako.template import Template

    projectFolder = Project.getProjectFolder()
    templatePath = projectFolder+'/default_assets/factories/FactoryTemplates.cpy'
    outputPaths = []

    for component in componentFiles:
        renderParameters = dict(component=component,
                                isVecType=_isVecType,
                                vecIterations=dict(v4f=4, v3f=3, v2f=2))
        fname = component['path'] + '.cpp'
        outputRelativePath = 'default_assets/factories/'+fname
        outputPath = projectFolder + '/' + outputRelativePath
        outputPaths.append(outputRelativePath)

        componentFactoryTemplate = open(templatePath).read()
        Utils.Console.step('Generating factory '+outputPath)
        with Utils.OpenRec(outputPath, 'w') as outputHandle:
            outputHandle.write(Template(componentFactoryTemplate).render(**renderParameters))
            pass
        pass
    return outputPaths

def _runGame(path, workFolder):
    from subprocess import Popen, PIPE

    process = Popen(path, stdout=PIPE, stderr=PIPE, cwd=workFolder)
    while process.returncode == None:
        process.poll()
        strMsg = process.stdout.readline()
        if len(strMsg)>0:
            WSServer.send('executionMessage', strMsg)
            pass
        pass
    pass

def _buildObjectFile(sourceFile, outputFolder, platform, compilationFlags, callback):
    global workQueue
    fileIsBeingBuilt = False
    fileSetLock.acquire()
    if sourceFile in fileSet:
        fileIsBeingBuilt = True
        fileCallbacks[sourceFile].append(callback)
    else:
        fileSet.add(sourceFile)
        fileCallbacks[sourceFile] = [callback]
        pass
    fileSetLock.release()

    if not fileIsBeingBuilt:
        workQueue.put(dict(sourceFile=sourceFile, outputFolder=outputFolder, platform=platform, compilationFlags=compilationFlags))
        pass
    pass

def isOutputOutdated(outputFile, dependencies):
    if not Utils.FileExists(outputFile):
        return True

    baseMTime = os.path.getmtime(outputFile)
    for dep in dependencies:
        if os.path.getmtime(dep) > baseMTime:
            return True
        pass
    return False

def BuildProject(platform = 'linux', runGame = True, compilationMode='DEBUG', outputFolder = None):
    import subprocess, time
    compilationFlags=_getFlags(compilationMode)

    projectFolder = Project.getProjectFolder()

    if outputFolder == None:
        outputFolder = projectFolder

    compilationStatus = dict(returncode=0, message='')
    try:
        # Generate build list
        sourceFiles = []
        componentScripts = Utils.ListFilesFromFolder(projectFolder)
        # Append component scripts to the list of build files
        for componentScript in componentScripts:
            if Utils.IsImplementationFile(componentScript):
                sourceFiles.append(Project.getRelProjFilePath(componentScript))
                pass
            pass

        # Compile
        internalCompStatus = dict(precompiledFiles = [])
        resultLock = threading.Lock()
        def threadResult(output, message, returncode):
            resultLock.acquire()
            if compilationStatus['returncode'] == 0:
                compilationStatus['returncode'] = returncode
                compilationStatus['message'] += message
                internalCompStatus['precompiledFiles'].append(output)
                pass

            resultLock.release()
            pass

        for sourceFile in sourceFiles:
            _buildObjectFile(sourceFile, outputFolder, platform, compilationFlags, threadResult)
            if compilationStatus['returncode'] != 0:
                break
            pass

        workQueue.join()

        # Link
        # TODO only link if the final executable is older than any of the object files
        Utils.Console.info('Linking '+outputFolder+'...')
        startTime=time.time()
        outputFile = outputFolder+'/game'
        if compilationStatus['returncode'] == 0 and isOutputOutdated(outputFile, internalCompStatus['precompiledFiles']):
            compilationStatus['message'] += subprocess.check_output(cxx_compiler[platform] + ' ' + (' '.join(internalCompStatus['precompiledFiles'])) +' -o '+outputFile+' ' + compilationFlags['link_flags'][platform], shell=True, stderr=subprocess.STDOUT)
            Utils.Console.info('Linking done ('+str(time.time()-startTime)+'s)')
        else:
            Utils.Console.info('Linking not necessary')
            pass

    except subprocess.CalledProcessError as e:
        compilationStatus['message'] = e.output
        compilationStatus['returncode'] = e.returncode
        Utils.Console.error('Could not link program')
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
        Utils.CopyFilesOfTypes(thirdPartyFolder+'/cross_compiling/windows/redist', outputFolder, ['.dll'])
        pass
    elif platform == 'preview' or platform == 'nacl':
        naclFolder = Config.get('export', 'nacl')['pepper_folder']
        unfinalizedFile = outputFolder+'/game'
        finalizedFile = outputFolder+'/lau_canvas.pexe'
        if isOutputOutdated(finalizedFile, [unfinalizedFile]):
            subprocess.check_output(naclFolder+'/toolchain/linux_pnacl/bin/pnacl-finalize '+unfinalizedFile + ' -o '+finalizedFile, shell=True)
            pass
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
        #dir_util.copy_tree(projectFolder + '/scenes', outputFolder+'/scenes')
        Utils.CopyFilesOfTypes(projectFolder+'/scenes', outputFolder, ['.json'], projectFolder)
        # Copy assets to destination folder
        Utils.CopyFilesOfTypes(projectFolder+'/assets', outputFolder, Config.env('exportable_asset_extensions'), projectFolder)
        Utils.CopyFilesOfTypes(projectFolder+'/default_assets', outputFolder, Config.env('exportable_asset_extensions'), projectFolder)
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

def BuildPreviewObject(inputFile, callback=None):
    def buildResult(output, message, returncode):
        WSServer.send('compilationStatus', dict(output=output, message=message, returncode=returncode))
        if callback!=None:
            callback(output, message, returncode)
            pass
        pass

    _buildObjectFile(inputFile, Project.getProjectFolder()+'/build/nacl/', 'preview', _getFlags('DEBUG'), buildResult)
    pass

class ObjectBuilderThread(threading.Thread):
    def buildObjectFile(self, sourceFile, outputFolder, platform, compilationFlags):
        import subprocess
        outputFilePath = os.path.normpath(outputFolder+'/build/'+sourceFile+'.o')
        compilationMessage = ''
        returncode=0

        # Create recursive folders, if necessary
        Utils.CreateFoldersRec(os.path.dirname(outputFilePath))

        try:
            # Only build the object file if it is older than any of its dependencies
            if Project.isFileOlderThanDependency(outputFilePath, sourceFile):
                Utils.Console.step('Building '+outputFilePath)
                compilationMessage = subprocess.check_output(cxx_compiler[platform] + ' -c ' + Project.getAbsProjFilePath(sourceFile) +' -o '+outputFilePath +' '+ platform_preprocessors[platform] + ' ' + compilationFlags['cxx_flags'][platform], shell=True, stderr=subprocess.STDOUT)
                if returncode == 0:
                    Utils.Console.ok('Built '+outputFilePath)
                    pass
                pass
        except subprocess.CalledProcessError as e:
            Utils.Console.fail('Error building '+outputFilePath)
            compilationMessage = e.output
            returncode = e.returncode
            pass

        fileSetLock.acquire()
        callbacks = fileCallbacks[sourceFile]
        del fileCallbacks[sourceFile]
        fileSet.remove(sourceFile)
        fileSetLock.release()

        for callback in callbacks:
            if callback!=None:
                callback(output=outputFilePath, message=compilationMessage, returncode=returncode)
                pass
            pass
        pass

    def run(self):
        global workQueue
        self.isRunning = True
        while self.isRunning:
            request = workQueue.get()
            self.buildObjectFile(**request)
            workQueue.task_done()
            pass
        pass

    def stop(self):
        self.isRunning = False
        pass
    pass

workQueue = Queue.Queue()
fileSetLock = threading.Lock()
fileSet = set()
fileCallbacks = dict()
for i in range(multiprocessing.cpu_count()):
    builderThread = ObjectBuilderThread()
    builderThread.daemon = True
    builderThread.start()
    pass

RPC.listen(buildGame)
RPC.listen(previewGame)

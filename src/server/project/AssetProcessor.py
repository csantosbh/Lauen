import os
import time
import json
import threading

from server.io import Utils
import Project

# Helper functions
def _removePreviewObjFiles(fpath):
    objectFilePaths = ['build/'+fpath+'.o',
            'build/nacl/build/'+fpath+'.o']
    for obj in objectFilePaths:
        Project.removeFile(obj)
        pass
    pass

class AssetJSONEncoder(json.JSONEncoder):
    def default(self, o):
        if isinstance(o, AssetProcessor):
            return o.persistent_fields
        else:
            return o
        pass
    pass

class AssetProcessor(object):
    def __init__(self, path, persistent_fields):
        self.isBroken = False
        self.path = path
        if persistent_fields != None:
            self.persistent_fields = persistent_fields
        else:
            self.persistent_fields = dict(mtime = 0, dependencies=[], id=Project.getUniqueAssetId())
        pass

    def id(self):
        return self.persistent_fields['id']

    def getModificationTime(self):
        return Project.getModificationTime(self.path)

    def dependencies(self):
        return self.persistent_fields['dependencies']

    def mostRecentDependencyTime(self):
        mostRecentTime = self.getModificationTime()
        for dependency in self.persistent_fields['dependencies']:
            if Project.fileExists(dependency):
                depTime = Project.getModificationTime(dependency)
                if depTime > mostRecentTime:
                    mostRecentTime = depTime
                    pass
            else:
                # The dependency was removed! Since we dont know when, lets
                # just assumed it happened right now
                mostRecentTime = time.gmtime()
                pass
            pass
        return mostRecentTime
    pass

# Responsible for processing script file events (add, remove, update).
class ScriptProcessor(AssetProcessor):
    def __init__(self, path, persistent_fields):
        super(ScriptProcessor, self).__init__(path, persistent_fields)
        self._metadataLock = threading.Lock()
        pass

    def dependsOn(self, path):
        return (path in self.persistent_fields['dependencies']) or self.isBroken

    # Handle file removal event
    def remove(self):
        if Utils.IsScriptFile(self.path):
            # Remove cache .json files
            self.removeAssetInfoCache(self.path)
            pass

        if Utils.IsImplementationFile(self.path):
            # Remove preview .o files
            _removePreviewObjFiles(self.path)
            pass

        if Utils.IsHeaderFile(self.path) and Project.isUserAsset(self.path):
            # If this is a user asset, remove its factory cpp file
            factoryFilePath = 'default_assets/factories/'+self.path+'.cpp'
            Project.removeFile(factoryFilePath)

            # Remove its object files as well
            _removePreviewObjFiles(factoryFilePath)
            pass
        pass

    def getMetadata(self):
        from server.parser import CppParser
        from server.build import BuildEventHandler

        with self._metadataLock:
            # Check if the asset is up to date, so we dont need to update it
            mostRecentDepTime = self.mostRecentDependencyTime()
            if self.persistent_fields['mtime'] >= mostRecentDepTime and not self.isBroken and Utils.FileExists(self.getAssetInfoCachePath(self.path)):
                fileSymbols = self.loadAssetInfoCache(self.path)
            else:
                fileInfo = CppParser.GetSimpleClass(self.path)
                if not fileInfo['success']:
                    Utils.Console.warning('Script '+self.path+' has error(s):\n  '+('\n  '.join(fileInfo['diagnostics']))+'\n\n')
                    self._diagnosticMessages = fileInfo['diagnostics']
                    self.isBroken = True
                    return None
                else:
                    self.isBroken = False
                    pass

                fileSymbols = fileInfo['symbols']
                fileSymbols['path'] = self.path
                fileSymbols['id'] = self.persistent_fields['id']

                Utils.Console.step('Updating dependencies for '+self.path)
                self.persistent_fields['dependencies'] = list(fileInfo['dependencies'])

                # Recalculate the most recent dep time, since my dependencies
                # were updated
                self.persistent_fields['mtime'] = self.mostRecentDependencyTime()

                self.saveAssetInfoCache(self.path, fileSymbols)
                pass
            return fileSymbols
        pass

    def update(self):
        from server.build import BuildEventHandler
        def buildCallback(output, message, returncode):
            if returncode != 0:
                self.isBroken = True
            else:
                self.isBroken = False
                pass
            pass

        # Generate its .o object if the path refers to a cpp file
        if Utils.IsImplementationFile(self.path):
            BuildEventHandler.BuildPreviewObject(self.path, buildCallback)
            # Update dependency list
            self.getMetadata()
        elif Utils.IsHeaderFile(self.path):
            # The file was updated. Re-generate its Factory file.
            if self.persistent_fields['mtime'] < self.mostRecentDependencyTime() or self.isBroken:
                assetMetadata = self.getMetadata()
                if assetMetadata != None and Project.isUserAsset(self.path):
                    outPaths = BuildEventHandler.RenderFactorySources([self.getMetadata()])
                    # Re-build the factories corresponding .o files
                    for path in outPaths:
                        BuildEventHandler.BuildPreviewObject(path)
                        pass
                    pass
                pass
            pass

        pass

    # Private methods
    def getAssetInfoCachePath(self, assetPath):
        return Project.getAbsProjFilePath('cache/'+assetPath+'.json')

    def saveAssetInfoCache(self, assetPath, assetSymbols):
        import json
        content=json.dumps(assetSymbols)
        with Utils.OpenRec(self.getAssetInfoCachePath(assetPath), 'w') as f:
            f.write(content)
            pass
        pass

    def removeAssetInfoCache(self, assetPath):
        import json
        Utils.RemoveFile(self.getAssetInfoCachePath(assetPath))
        pass

    def loadAssetInfoCache(self, assetPath):
        import json
        with open(self.getAssetInfoCachePath(assetPath), 'r') as f:
            return json.loads(f.read())
            pass
        pass
    pass

class UserFactoriesProcessor(AssetProcessor):
    def __init__(self, path, persistent_fields):
        super(UserFactoriesProcessor, self).__init__(path, persistent_fields)
        self.persistent_fields['dependencies'] = [
            "default_assets/Factories.hpp",
            "default_assets/Peekers.hpp",
            "default_assets/LauCommon.h"
        ]
        pass

    def dependsOn(self, path):
        return path in self.persistent_fields['dependencies']

    def getMetadata(self):
        return None

    def update(self):
        from server.build import BuildEventHandler
        mostRecentDepTime = self.mostRecentDependencyTime()
        if self.persistent_fields['mtime'] < mostRecentDepTime:
            # Re-generate all factory files
            assetProcessors = Project.getAssetProcessors()
            for key in assetProcessors:
                asset = assetProcessors[key]
                if Utils.IsHeaderFile(asset.path) and Project.isUserAsset(asset.path):
                    outPaths = BuildEventHandler.RenderFactorySources([asset.getMetadata()])
                    # Re-generate its .o file
                    for path in outPaths:
                        BuildEventHandler.BuildPreviewObject(path)
                        pass
                    pass
                pass

            self.persistent_fields['mtime'] = self.mostRecentDependencyTime()
            pass
        pass

    def remove(self):
        Utils.Console.error('FactoryTemplates.cpy file is missing!')
        pass

    pass

def CreateAssetProcessor(assetPath, persistent_fields=None):
    if Utils.IsScriptFile(assetPath) and not Project.isUserScriptFactory(assetPath):
        return ScriptProcessor(assetPath, persistent_fields)
    elif Utils.IsCpyFile(assetPath):
        if assetPath.endswith('FactoryTemplates.cpy'):
            return UserFactoriesProcessor(assetPath, persistent_fields)
        else:
            Utils.Console.error('Unsupported cpy file detected: '+assetPath)
            return None
        pass
    else:
        return None
    pass


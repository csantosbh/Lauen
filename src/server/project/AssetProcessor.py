import os
import json

from server.io import Utils
import Project

# Helper functions
def _removePreviewObjFiles(fname):
    objectFilePaths = [Project.getProjectFolder()+'/build/'+fname+'.o',
            Project.getProjectFolder()+'/build/nacl/build/'+fname+'.o']
    for obj in objectFilePaths:
        Utils.RemoveFile(obj)
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
            # TODO RMOVE
            if not 'id' in self.persistent_fields:
                self.persistent_fields['id'] = Project.getUniqueAssetId()
                pass
        else:
            self.persistent_fields = dict(mtime = 0, dependencies=[], id=Project.getUniqueAssetId())
        pass

    def id(self):
        return self.persistent_fields['id']

    def dependencies(self):
        return self.persistent_fields['dependencies']

    pass

# Responsible for processing script file events (add, remove, update).
class ScriptProcessor(AssetProcessor):
    def __init__(self, path, persistent_fields):
        super(ScriptProcessor, self).__init__(path, persistent_fields)
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
            fname = Utils.GetFileNameFromPath(self.path)
            _removePreviewObjFiles(fname)
            pass

        if Utils.IsHeaderFile(self.path) and Project.isUserAsset(self.path):
            # If this is a user asset, remove its factory cpp file
            fname = Utils.GetFileNameFromPath(self.path)

            factoryFilePath = Project.getProjectFolder()+'/default_assets/factories/'+fname[:fname.rfind('.')] +'_'+str(self.id()) + '.cpp'
            Utils.RemoveFile(factoryFilePath)

            # Remove its object files as well
            fname = Utils.GetFileNameFromPath(factoryFilePath)
            _removePreviewObjFiles(fname)

            # Finally, remove its json symbol cache
            self.removeAssetInfoCache(factoryFilePath)
            pass
        pass

    def getMetadata(self):
        from server.parser import CppParser
        from server.build import BuildEventHandler
        # Check if the asset is up to date, so we dont need to update it
        #if self.persistent_fields['mtime'] >= os.path.getmtime(self.path) and not self.isBroken:
        if self.persistent_fields['mtime'] >= os.path.getmtime(self.path) and (not Project.isFileOlderThanDependency(self.path, self.path)) and not self.isBroken:
            fileSymbols = self.loadAssetInfoCache(self.path)
        else:
            fileInfo = CppParser.GetSimpleClass(self.path)
            if not fileInfo['success']:
                Utils.Console.warning('Stop: File '+self.path+' has an error.')
                self._diagnosticMessages = fileInfo['diagnostics']
                self.isBroken = True
                return None
            else:
                self.isBroken = False
                pass

            fileSymbols = fileInfo['symbols']
            fileSymbols['path'] = self.path
            fileSymbols['id'] = self.persistent_fields['id']

            # The file was updated. Re-generate its Factory file.
            if Utils.IsHeaderFile(self.path):
                if Project.isUserAsset(self.path):
                    outPaths = BuildEventHandler.RenderFactorySources([fileSymbols])
                else:
                    outPaths = [BuildEventHandler.RenderStandardFactorySources()]
                    pass

                # Re-build the factory corresponding .o file
                for path in outPaths:
                    status = BuildEventHandler.BuildPreviewObject(path)
                    pass
                pass

            print 'updating dependencies for '+self.path
            self.persistent_fields['dependencies'] = list(fileInfo['dependencies'])
            self.persistent_fields['mtime'] = os.path.getmtime(self.path)

            self.saveAssetInfoCache(self.path, fileSymbols)
            pass
        return fileSymbols

    def update(self):
        from server.build import BuildEventHandler
        # Generate its .o object if the path refers to a cpp file
        if Utils.IsImplementationFile(self.path):
            buildStatus = BuildEventHandler.BuildPreviewObject(self.path)

            if buildStatus['returncode'] != 0:
                self.isBroken = True
                Utils.Console.error('Failed building '+self.path)
            else:
                self.isBroken = False
                pass
            pass

        pass

    # Private methods
    def getAssetInfoCachePath(self, assetPath):
        relative = os.path.relpath(assetPath, Project.getProjectFolder())
        return Project.getProjectFolder()+'/cache/'+relative+'.json'

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
        pass

    def dependsOn(self, path):
        # FactoryTemplates.cpy only depends on itself, as user factories are
        # generated by ScriptProcessor.
        return False

    def getMetadata(self):
        return None

    def update(self):
        from server.build import BuildEventHandler
        if self.persistent_fields['mtime'] < os.path.getmtime(self.path):
            print 'User factory updated'
            # Re-generate all factory files
            for asset in Project.getAssetList():
                if Utils.IsHeaderFile(asset['path']) and Project.isUserAsset(asset['path']):
                    outPaths = BuildEventHandler.RenderFactorySources([asset])
                    # Re-generate its .o file
                    for path in outPaths:
                        BuildEventHandler.BuildPreviewObject(path)
                        pass
                    pass
                pass

            self.persistent_fields['mtime'] = os.path.getmtime(self.path)
            pass
        pass

    def remove(self):
        Utils.Console.error('Error: FactoryTemplates.cpy file is missing!')
        pass

    pass

class StandardFactoriesProcessor(AssetProcessor):
    def __init__(self, path, persistent_fields):
        super(StandardFactoriesProcessor, self).__init__(path, persistent_fields)
        pass

    def dependsOn(self, path):
        # Factories.cpy only depends on itself, as Factories.cpp is generated
        # by ScriptProcessor.
        return False

    def getMetadata(self):
        return None

    def update(self):
        from server.build import BuildEventHandler
        if self.persistent_fields['mtime'] < os.path.getmtime(self.path):
            BuildEventHandler.RenderStandardFactorySources()
            self.persistent_fields['mtime'] = os.path.getmtime(self.path)
            pass
        pass

    def remove(self):
        print 'Error: Factories.cpy file is missing!'
        pass

    pass

def CreateAssetProcessor(assetPath, persistent_fields=None):
    if Utils.IsScriptFile(assetPath) and not Project.isUserScriptFactory(assetPath):
        return ScriptProcessor(assetPath, persistent_fields)
    elif Utils.IsCpyFile(assetPath):
        # TODO rename factories.cpy to standardfactories.cpy and factorytemplates to userfactories.cpy
        if assetPath.endswith('Factories.cpy'):
            return StandardFactoriesProcessor(assetPath, persistent_fields)
        elif assetPath.endswith('FactoryTemplates.cpy'):
            return UserFactoriesProcessor(assetPath, persistent_fields)
        else:
            print 'Error: unsupported cpy file detected: '+assetPath
            return None
        pass
    else:
        return None
    pass


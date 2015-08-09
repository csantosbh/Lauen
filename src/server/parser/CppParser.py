#!/usr/bin/env python
import sys
import os
import clang.cindex
from clang.cindex import Index, TokenKind, TranslationUnit

###
# Globals
_clangIndex = Index(clang.cindex.conf.lib.clang_createIndex(False, 1))
#_clangIndex = Index.create()
_translationUnits = dict()

def translateFieldType(typeName):
    if typeName == 'c:Matrix.h@N@Eigen@T@Vector3f':
        return 'v3f'
    elif typeName == 'c:Matrix.h@N@Eigen@T@Vector2f':
        return 'v2f'
    else:
        return typeName
    pass

# Iterate over the source file, retrieving all classes and their
# public fields and methods
def parseCPPFileChildren(output, node, pragmaList, context):
    childNodes = []
    for child in node.get_children():
        childNode = parseCPPFileRec(child, pragmaList, context)
        childNodes += childNode['symbols']
        output['dependencies'].update(childNode['dependencies'])

    if len(childNodes)>0:
        output['symbols']+=childNodes
        pass
    pass

def parseCPPFileRec(node, pragmaList, context=dict()):
    from clang.cindex import Cursor, CursorKind, AccessSpecifier
    output={'symbols': [], 'dependencies': set()}


    if node.kind != CursorKind.TRANSLATION_UNIT:
        fileNameStr = str(node.location.file)
    
        if fileNameStr != context['requestedFileName']:
            output['dependencies'].add(os.path.abspath(fileNameStr))
            return output

    if node.kind == CursorKind.NAMESPACE:
        subContext = context.copy()
        if len(subContext['currentNameSpace']) == 0:
            subContext['currentNameSpace'] = node.spelling;
        else:
            subContext['currentNameSpace'] = subContext['currentNameSpace'] + '::' + node.spelling
            pass
        parseCPPFileChildren(output, node, pragmaList, subContext)
    elif node.kind == CursorKind.CLASS_DECL:
        if not context['withinClassContext']:
            childPragmas = []
            if len(pragmaList) > 0:
                while len(pragmaList)>0 and pragmaList[0]['line'] < node.extent.start.line:
                    childPragmas.append(pragmaList.pop(0)['identifier'])

            childNodes = []
            subContext = context.copy()
            subContext['depth'] = context['depth']+1
            subContext['withinClassContext'] = True
            for child in node.get_children():
                childNode = parseCPPFileRec(child, pragmaList, subContext)
                childNodes+=childNode['symbols']
                output['dependencies'].update(childNode['dependencies'])
                pass

            node = {
                'kind': node.kind.name,
                'name': node.spelling,
                'children': childNodes,
                'pragmas': childPragmas,
                'namespace': context['currentNameSpace'],
            }
            output['symbols'].append(node)
    elif node.kind == CursorKind.FIELD_DECL or (node.kind == CursorKind.FUNCTION_DECL and context['withinClassContext']):
        childPragmas = []
        if len(pragmaList) > 0:
            while len(pragmaList) > 0 and pragmaList[0]['line'] < node.extent.start.line:
                childPragmas.append(pragmaList.pop(0)['identifier'])

        nodeType = node.type.get_declaration().get_usr()
        if nodeType == '':
            nodeType = node.type.spelling
            pass

        dictNode = {
            'kind': node.kind.name,
            'name': node.spelling,
            'visibility': 0,
            'pragmas': childPragmas,
            'type': translateFieldType(nodeType),
        }
        if node.access_specifier == AccessSpecifier.PROTECTED:
            dictNode['visibility'] = 1
        elif node.access_specifier == AccessSpecifier.PRIVATE:
            dictNode['visibility'] = 2

        output['symbols'].append(dictNode)
    else:
        subContext = context.copy()
        subContext['depth'] = context['depth']+1
        parseCPPFileChildren(output, node, pragmaList, subContext)
        pass

    return output
    pass

def parseCPPFile(fileName):
    import re
    from server import Config
    from server.project import Project
    from server.io import Utils

    global _clangIndex
    global _translationUnits

    # Remove #include's from source files, to prevent clang
    # from parsing them. I know this is a horrible solution,
    # but clang will take forever to parse the files if they
    # have headers, and it doesn't have an option for disabling
    # recursive parsing.
    if fileName in _translationUnits:
        tu = _translationUnits[fileName]
        tu.reparse([(fileName, open(fileName,'r'))])
    else:
        try :
            # TODO maybe get the flags from BuildEventHandler?
            ####
            # TODO documentar q foi gerado com:
            # clang++ -x c++-header -std=c++11 Eigen -relocatable-pch -o Eigen.pch
            # (dentro da pasta Eigen)
            ####
            tu = _clangIndex.parse(fileName, ['-std=c++11',
                                        '-Werror',
                                        '-DLINUX',
                                        '-DDESKTOP',
                                        '-DDEBUG',
                                        '-I',Config.get('export', 'third_party_folder')+'/rapidjson/include',
                                        '-I',Project.getProjectFolder()+'/default_assets/',
                                        '-include-pch',Config.get('export', 'third_party_folder')+'/Eigen/Eigen.pch'],
                                        options=TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)
            _translationUnits[fileName] = tu
        except Exception as err:
            Utils.Console.error('Error processing translation unit!')
            return dict(symbols=[], success=False, diagnostics=[str(err)], dependencies=[])
            pass
        pass

    # Check for diagnostic messages
    diagnostics = []
    success = True
    for diag in tu.diagnostics:
        diagnostics.append(diag.spelling)
        if diag.severity==clang.cindex.Diagnostic.Error or diag.severity==clang.cindex.Diagnostic.Fatal:
            success = False
        pass

    # Get list of pragmas
    if success:
        prevToken = None
        nextIsPragma = False
        pragmaList = []
        for token in tu.cursor.get_tokens():
            if token.kind==TokenKind.IDENTIFIER and prevToken.kind == TokenKind.PUNCTUATION and token.spelling=='pragma':
                nextIsPragma = True
            elif nextIsPragma:
                if token.kind==TokenKind.IDENTIFIER:
                    pragmaList.append(dict(line=token.extent.start.line, identifier=token.spelling))
                nextIsPragma=False
            prevToken = token

        parseContext=dict(depth=0,withinClassContext=False,requestedFileName=fileName,currentNameSpace='')
        parseResult=parseCPPFileRec(tu.cursor, pragmaList, parseContext)
    else:
        parseResult=dict(dependencies=[])
        pass

    parseResult['diagnostics'] = diagnostics
    parseResult['success'] = success

    return parseResult

# Get only what matters from the parseCPPFile function
def GetSimpleClass(fileName):
    from server.project import Project
    from server.components import DefaultComponentManager
    print 'parsing ' + fileName
    parsedFile = parseCPPFile(fileName)
    simpleClass = dict()
    dependencies = []

    if parsedFile['success'] and len(parsedFile['symbols']) > 0:
        fileSymbols = parsedFile['symbols']
        if fileSymbols[0]['kind'] == 'CLASS_DECL':
            simpleClass['class'] = fileSymbols[0]['name']
            simpleClass['namespace']  = fileSymbols[0]['namespace']

            simpleClass['fields'] = {}
            simpleClass['types'] = {}
            simpleClass['pragmas'] = {}
            simpleClass['visibilities'] = {}
            for child in fileSymbols[0]['children']:
                if child['kind'] == 'FIELD_DECL' and child['visibility']==0:
                    simpleClass['fields'][child['name']] = DefaultComponentManager.DefaultFieldValue(child['type'])
                    simpleClass['types'][child['name']] = child['type']
                    simpleClass['pragmas'][child['name']] = child['pragmas']
                    simpleClass['visibilities'][child['name']] = child['visibility']
                    pass
                pass
            pass
        pass

    dependencies = parsedFile['dependencies']

    return dict(symbols=simpleClass,
            success=parsedFile['success'],
            diagnostics=parsedFile['diagnostics'],
            dependencies=dependencies)


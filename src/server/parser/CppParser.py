#!/usr/bin/env python
import sys
import os

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
        output['dependencies'].add(os.path.abspath(fileNameStr))
    
        if fileNameStr != context['requestedFileName']:
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
    from clang.cindex import Index, TokenKind, TranslationUnit
    import re

    index = Index.create()
    fileContent = open(fileName,'r').read()
    # Remove #include's from source files, to prevent clang
    # from parsing them. I know this is a horrible solution,
    # but clang will take forever to parse the files if they
    # have headers, and it doesn't have an option for disabling
    # recursive parsing.
    #fileContent = re.sub(r'^.*#include.*$', '', fileContent, 0, re.MULTILINE)
    tu = index.parse(fileName, ['-std=c++11', '-fsyntax-only', '-I','/home/csantos/workspace/LauEngine/third_party/Eigen'], unsaved_files=[(fileName,fileContent)], options=TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)

    # Get list of pragmas
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
    rtrn=parseCPPFileRec(tu.cursor, pragmaList, parseContext)

    del tu
    del index
    return rtrn

# Get only what matters from the parseCPPFile function
def GetSimpleClass(fileName):
    from server.project import Project
    from server.components import DefaultComponentManager
    parsedFile = parseCPPFile(fileName)
    fileSymbols = parsedFile['symbols']
    simpleClass = dict()

    if len(fileSymbols) > 0:
        if fileSymbols[0]['kind'] == 'CLASS_DECL':
            simpleClass['class'] = fileSymbols[0]['name']
            #simpleClass['id'] = Project.getScriptId(fileName)
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

    return dict(symbols=simpleClass, dependencies=parsedFile['dependencies'])

if __name__ == '__main__':
    from pprint import pprint
    if len(sys.argv) != 2:
        print 'Usage: '+sys.argv[0]+' <cppFile>'
        exit()
        pass
    pprint(parseCPPFile(sys.argv[1]))

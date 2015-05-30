#!/usr/bin/env python
import sys
from server.project import Project

# Iterate over the source file, retrieving all classes and their
# public fields and methods
def parseCPPFileChildren(output, node, pragmaList, context):
    childNodes = []
    for child in node.get_children():
        childNodes += parseCPPFileRec(child, pragmaList, context)

    if len(childNodes)>0:
        output+=childNodes
        pass
    pass

def parseCPPFileRec(node, pragmaList, context=dict()):
    from clang.cindex import CursorKind, AccessSpecifier
    output=[]

    # TODO: Get class namespace as well!
    if node.kind != CursorKind.TRANSLATION_UNIT and str(node.location.file) != context['requestedFileName']:
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
                childNodes += parseCPPFileRec(child, pragmaList, subContext)

            node = {
                'kind': node.kind.name,
                'name': node.spelling,
                'children': childNodes,
                'pragmas': childPragmas,
                'namespace': context['currentNameSpace'],
            }
            output.append(node)
    elif node.kind == CursorKind.FIELD_DECL or (node.kind == CursorKind.FUNCTION_DECL and context['withinClassContext']):
        childPragmas = []
        if len(pragmaList) > 0:
            while len(pragmaList) > 0 and pragmaList[0]['line'] < node.extent.start.line:
                childPragmas.append(pragmaList.pop(0)['identifier'])

        dictNode = {
            'kind': node.kind.name,
            'name': node.spelling,
            'visibility': 0,
            'pragmas': childPragmas,
            'type': node.type.spelling,
        }
        if node.access_specifier == AccessSpecifier.PROTECTED:
            dictNode['visibility'] = 1
        elif node.access_specifier == AccessSpecifier.PRIVATE:
            dictNode['visibility'] = 2

        output.append(dictNode)
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
    fileContent = re.sub(r'^.*#include.*$', '', fileContent, 0, re.MULTILINE)
    tu = index.parse(fileName, ['-std=c++11'], unsaved_files=[(fileName,fileContent)], options=TranslationUnit.PARSE_SKIP_FUNCTION_BODIES)

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
    parsedFile = parseCPPFile(fileName)
    if len(parsedFile) == 0:
        return dict()

    simpleClass = dict()
    if parsedFile[0]['kind'] == 'CLASS_DECL':
        simpleClass['class'] = parsedFile[0]['name']
        simpleClass['id'] = Project.getScriptId(fileName)
        simpleClass['namespace']  = parsedFile[0]['namespace']
        simpleClass['fields'] = []
        for child in parsedFile[0]['children']:
            if child['kind'] == 'FIELD_DECL':
                simpleClass['fields'].append(dict(name=child['name'], type=child['type'], pragmas=child['pragmas'], visibility=child['visibility']))
                pass
            pass
        pass

    return simpleClass


#!/usr/bin/env python
import sys

# Iterate over the source file, retrieving all classes and their
# public fields and methods
def parseCPPFileRec(node, pragmaList, depth=0, withinClassContext=False, requestedFile=None):
    from clang.cindex import CursorKind, AccessSpecifier
    output=[]

    # TODO: Get class namespace as well!
    if requestedFile != None and node.location.file != None and str(node.location.file) != requestedFile:
        return output

    if node.kind == CursorKind.CLASS_DECL:
        if not withinClassContext:
            childPragmas = []
            if len(pragmaList) > 0:
                while len(pragmaList)>0 and pragmaList[0]['line'] < node.extent.start.line:
                    childPragmas.append(pragmaList.pop(0)['identifier'])

            childNodes = []
            for child in node.get_children():
                childNodes += parseCPPFileRec(child, pragmaList, depth+1, True, requestedFile)

            node = {
                'kind': node.kind.name,
                'name': node.spelling,
                'children': childNodes,
                'pragmas': childPragmas,
            }
            output.append(node)
    elif node.kind == CursorKind.FIELD_DECL or (node.kind == CursorKind.FUNCTION_DECL and withinClassContext):
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
        childNodes = []
        for child in node.get_children():
            childNodes += parseCPPFileRec(child, pragmaList, depth+1, False, requestedFile)

        if len(childNodes)>0:
            output+=childNodes

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

    rtrn=parseCPPFileRec(tu.cursor, pragmaList, 0, False, fileName)

    del tu
    del index
    return rtrn

_className2Id = dict()
_classId2Name = dict()

def GetIdFromClass(className):
    from random import randint
    if not className in _className2Id:
        candidateId = randint(0,1e9)
        while candidateId in _classId2Name:
            candidateId = randint(0,1e9)
            pass
        _className2Id[className] = candidateId
        _classId2Name[candidateId] = className
        pass

    return _className2Id[className]

def GetSimpleClass(fileName):
    parsedFile = parseCPPFile(fileName)
    if len(parsedFile) == 0:
        return dict()

    simpleClass = dict()
    if parsedFile[0]['kind'] == 'CLASS_DECL':
        simpleClass['class'] = parsedFile[0]['name']
        simpleClass['id'] = GetIdFromClass(simpleClass['class'])
        simpleClass['fields'] = []
        for child in parsedFile[0]['children']:
            if child['kind'] == 'FIELD_DECL':
                simpleClass['fields'].append(dict(name=child['name'], type=child['type'], pragmas=child['pragmas'], visibility=child['visibility']))
            pass

    return simpleClass


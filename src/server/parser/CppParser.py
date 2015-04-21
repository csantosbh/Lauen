#!/usr/bin/env python

# Iterate over the source file, retrieving all classes and their
# public fields and methods
def parseCPPFileRec(node, depth=0, withinClassContext=False):
    from clang.cindex import CursorKind, AccessSpecifier
    output=[]

    if node.kind == CursorKind.CLASS_DECL:
        if not withinClassContext:
            childNodes = []
            for child in node.get_children():
                childNodes += parseCPPFileRec(child, depth+1, True)

            node = {
                'kind': node.kind.name,
                'name': node.spelling,
                'children': childNodes,
            }
            output.append(node)
    elif node.kind == CursorKind.FIELD_DECL or (node.kind == CursorKind.FUNCTION_DECL and withinClassContext):
        dictNode = {
            'kind': node.kind.name,
            'name': node.spelling,
            'visibility': 0,
        }
        if node.access_specifier == AccessSpecifier.PROTECTED:
            dictNode['visibility'] = 1
        elif node.access_specifier == AccessSpecifier.PRIVATE:
            dictNode['visibility'] = 2

        output.append(dictNode)
    else:
        childNodes = []
        for child in node.get_children():
            childNodes += parseCPPFileRec(child, depth+1, False)

        if len(childNodes)>0:
            output+=childNodes

    return output
    pass

def parseCPPFile(fileName):
    from clang.cindex import Index
    from pprint import pprint

    index = Index.create()
    tu = index.parse(None, [fileName])

    rtrn=parseCPPFileRec(tu.cursor)

    del tu
    del index
    return rtrn

def main():
    pprint(parseCPPFile(tu.cursor))

if __name__ == '__main__':
    main()

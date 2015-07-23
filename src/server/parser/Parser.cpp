#include <iostream>
#include <clang-c/Index.h>
#include <string>
#include <ctime>

using namespace std;

bool hasEnding (std::string const &fullString, std::string const &ending) {
    if (fullString.length() >= ending.length()) {
        return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

template<typename T, void destroyer(T)> struct manager {
    T data;
    manager() {}
    manager(T t) : data(t) {}
    operator T () {return data;}
    ~manager() {destroyer(data);}
};

const char* FNAME;
CXChildVisitResult visitar(CXCursor cursor, CXCursor parent, CXClientData client_data) {
    CXSourceLocation symbolSrc = clang_getCursorLocation (cursor);
    CXFile symbolFile;
    clang_getSpellingLocation   (symbolSrc,
            &symbolFile,
            NULL,//unsigned *  line,
            NULL,//unsigned *  column,
            NULL//unsigned *  offset 
            );

    CXString fileName = clang_getFileName(symbolFile);
    if(clang_getCString(fileName) != NULL && !hasEnding(std::string(clang_getCString(fileName)), FNAME)) {
        // I only want to know about symbols in my parsing file
        return CXChildVisit_Continue;
    }

    switch(cursor.kind) {
        case CXCursor_ClassDecl: {
            CXString name = clang_getCursorSpelling(cursor);
            CXString usr = clang_getCursorUSR(cursor);
            cout << "eh class: " << clang_getCString(name) << "("<< clang_getCString(usr) << ")"<< endl;
            }
            break;
        case CXCursor_FieldDecl: {
            CXString name = clang_getCursorSpelling(cursor);
            CXString usr = clang_getCursorUSR(cursor);

            CXType type = clang_getCursorType(cursor);
            CXString typespelling = clang_getTypeSpelling(type);
            cout << "\teh field: " << clang_getCString(name) << "("<< clang_getCString(usr) << ") type: " << clang_getCString(typespelling);
            //cout << type.kind << endl;
            switch(type.kind) {
                case CXType_Unexposed: {
                CXCursor typeClass = clang_getTypeDeclaration(type);
                CXString className = clang_getCursorSpelling(typeClass);
                CXString classUSR = clang_getCursorUSR(typeClass);
                cout << "(" << clang_getCString(classUSR) << ")";
                }
                break;
            }
            cout << endl;
            }
            break;
    }

    //return CXChildVisit_Continue;
    return CXChildVisit_Recurse;
}

int main(int argc, char* argv[])
{
    if(argc != 2) {
        cout << argv[0] << " <filename>" << endl;
        return 0;
    }
    manager<CXIndex, clang_disposeIndex> managed_index(clang_createIndex(1, 0));
    CXIndex index = managed_index;
    if (!index) {
        cerr << "failed creating index" << std::endl;
        return 1;
    }

    FNAME = argv[1];

    // clang_parseTranslationUnit2 (CXIndex CIdx, const char *source_filename, const char *const *command_line_args, int num_command_line_args, struct CXUnsavedFile *unsaved_files, unsigned num_unsaved_files, unsigned options, CXTranslationUnit *out_TU)
    manager<CXTranslationUnit, clang_disposeTranslationUnit> managed_translation_unit;
    const char* params[] = {"-std=c++11","-Werror", "-I", "/home/csantos/workspace/LauEngine/third_party/Eigen", "-include-pch", "/home/csantos/workspace/LauEngine/third_party/Eigen/Eigen.pch"};
    const int paramCount = 6;
    clock_t begin = clock();
    /*
    CXErrorCode error = clang_parseTranslationUnit2 (index, argv[1], params, paramCount, nullptr, 0,
            CXTranslationUnit_PrecompiledPreamble|CXTranslationUnit_Incomplete|CXTranslationUnit_SkipFunctionBodies|clang_defaultEditingTranslationUnitOptions(),///*CXTranslationUnit_PrecompiledPreamble
            &managed_translation_unit.data);
            */
    //CXTranslationUnit tu = clang_createTranslationUnitFromSourceFile(index, argv[1], paramCount, params, 0, NULL);
    CXTranslationUnit tu = clang_parseTranslationUnit(index,
            argv[1], params, paramCount, nullptr, 0,
            clang_defaultEditingTranslationUnitOptions());

    if(tu) {
        managed_translation_unit.data = tu;
        CXTranslationUnit translation_unit = managed_translation_unit;
        CXCursor cursor = clang_getTranslationUnitCursor(translation_unit);

        while(clang_visitChildren(cursor, visitar, NULL) != 0) {
        }
        clock_t end = clock();
        double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
        cout << "Elapsed " << elapsed_secs << "s"<<endl;

        //////// Reparse
        clang_reparseTranslationUnit(managed_translation_unit.data, 0, nullptr, clang_defaultReparseOptions(managed_translation_unit.data));
        begin = clock();
        elapsed_secs = double(begin - end) / CLOCKS_PER_SEC;
        cout << "Reparse " << elapsed_secs << "s"<<endl;

        //////// Reparse again
        clang_reparseTranslationUnit(managed_translation_unit.data, 0, nullptr, clang_defaultReparseOptions(managed_translation_unit.data));
        end = clock();
        elapsed_secs = double(end-begin) / CLOCKS_PER_SEC;
        cout << "Reparse2 " << elapsed_secs << "s"<<endl;

        //////// Reparse
        clang_reparseTranslationUnit(managed_translation_unit.data, 0, nullptr, clang_defaultReparseOptions(managed_translation_unit.data));
        begin = clock();
        elapsed_secs = double(begin - end) / CLOCKS_PER_SEC;
        cout << "Reparse3 " << elapsed_secs << "s"<<endl;

        //////// Reparse again
        system("mv demo.cpp demo3.cpp");
        system("mv demo2.cpp demo.cpp");
        system("mv demo3.cpp demo2.cpp");

        clang_reparseTranslationUnit(managed_translation_unit.data, 0, nullptr, clang_defaultReparseOptions(managed_translation_unit.data));
        end = clock();
        elapsed_secs = double(end-begin) / CLOCKS_PER_SEC;
        cout << "Reparse4 " << elapsed_secs << "s"<<endl;

        cursor = clang_getTranslationUnitCursor(managed_translation_unit.data);
        while(clang_visitChildren(cursor, visitar, NULL) != 0) { }

    } else {
        cerr << "Could not create translation unit!" << endl;
        return 1;
    }

    return 0;
}

#include <memory>
#include <Python.h>
#include <iostream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>

#include "Parser.h"
#include "utils/IO.h"

using namespace std;
using namespace rapidjson;

class PythonParser : public Parser {
private:
    void parseFileRec(PyObject *currentNode, Value& jsonContainer, Document& document) const {
        if(PyList_Check(currentNode)) {
            // Iterate over list
            jsonContainer.SetArray();
            for(int i = 0; i < PyList_Size(currentNode); ++i) {
                PyObject *nextElement = PyList_GetItem(currentNode, i);
                Value jsonElement;
                parseFileRec(nextElement, jsonElement, document);
                jsonContainer.PushBack(jsonElement, document.GetAllocator());
            }
        } else if(PyDict_Check(currentNode)) {
            // Iterate over dict
            Py_ssize_t t = 0;
            PyObject *dictKey, *dictValue;
            jsonContainer.SetObject();
            while(PyDict_Next(currentNode, &t, &dictKey, &dictValue)) {
                Value jsonKey, jsonValue;
                jsonKey.SetString(PyString_AS_STRING(dictKey),
                        PyString_Size(dictKey),
                        document.GetAllocator());
                if(PyInt_Check(dictValue)) {
                    jsonValue.SetInt(PyInt_AS_LONG(dictValue));
                }
                else if(PyString_Check(dictValue)) {
                    jsonValue.SetString(PyString_AS_STRING(dictValue),
                            PyString_Size(dictValue),
                            document.GetAllocator());
                }
                else {
                    parseFileRec(dictValue, jsonValue, document);
                }

                jsonContainer.AddMember(jsonKey, jsonValue, document.GetAllocator());
            }
        }
    }

public:
    virtual void parseFile(const string& fileName) const {
        // Prepare output
        Document document;
        document.SetArray();

        // Set function arguments
        PyObject *fName = PyString_FromString(fileName.c_str());
        PyTuple_SetItem(pArgs, 0, fName);

        // Call function
        PyObject *parseResult = PyObject_CallObject(pFunc, pArgs);

        // Grab returned values
        if(parseResult != NULL) {
            parseFileRec(parseResult, document, document);

            Py_DECREF(parseResult);

            // print
            StringBuffer buff;
            Writer<StringBuffer> writer(buff);
            document.Accept(writer);
            cout << buff.GetString() << endl;

        } else {
            PyErr_Print();
        }
    }

    PythonParser() : pModule(NULL), pFunc(NULL), pArgs(NULL) {
        char progName[] = "parser.py";
        Py_SetProgramName(progName);
        Py_Initialize();

        // Add the scripts folder to the python path, so
        // it can find the modules inside it
        PyObject* sysPath = PySys_GetObject((char*)"path");
        PyObject* scriptsPath = PyString_FromString("scripts");
        PyList_Append(sysPath, scriptsPath);
        Py_DECREF(scriptsPath);

        // Load parser.py module
        PyObject *pName = PyString_FromString("CppParser");
        pModule = PyImport_Import(pName);
        Py_DECREF(pName);

        if(pModule != NULL) {
            pFunc = PyObject_GetAttrString(pModule, "parseCPPFile");
            if(pFunc == NULL) {
                // TODO: exception: could not load parseCPPFile function
                PyErr_Print();
            }
            else if (pFunc && PyCallable_Check(pFunc)) {
                pArgs = PyTuple_New(1);
            } else {
                // TODO: exception: parseCPPFile is not a function!
            }
        } else {
            PyErr_Print();
            // TODO: exception: could not load python module
        }
    }
    ~PythonParser() {
        if(pModule != NULL) Py_DECREF(pModule);
        if(pFunc != NULL) Py_XDECREF(pFunc);
        if(pArgs != NULL) Py_DECREF(pArgs);

        Py_Finalize();
    }

private:
    string sourceCode;
    PyObject *pModule, *pFunc, *pArgs;
};

shared_ptr<Parser> Parser::instance;
const Parser* Parser::getInstance() {
    if(instance==NULL) instance = shared_ptr<Parser>(new PythonParser());
    return instance.get();
}


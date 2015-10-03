#include <unistd.h>

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <queue>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#include "LauCommon.h"

#ifdef NACL
#include "ppapi/cpp/url_loader.h"
#include "utils/UrlLoaderHandler.h"
#else
#include <fstream>
#endif

#include "Game.hpp"
#include "window/NaCl.hpp"
#include "utils/IO.h"
#include "utils/ThreadPool.hpp"

using namespace std;

namespace lau {

namespace utils {

#ifdef NACL

using namespace pp;

class NaClIO: public IO {
public:
    virtual void requestFiles(const initializer_list<string>& filenames,
            const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        requestFiles<const initializer_list<string>&>(filenames, callback);
    }

    virtual void requestFiles(const vector<string>& filenames,
            const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        requestFiles<const vector<string>&>(filenames, callback);
    }

    NaClIO() :
        handler(NULL),
        onLoadDone(bind(&NaClIO::onLoadDoneImpl, this, placeholders::_1, placeholders::_2))
    {}

private:
    URLLoaderHandler* handler;
    function<void(bool success, const string& data)> onLoadDone;

    queue<queue<string>> pendingRequests;
    queue<function<void(deque<pair<bool,vector<uint8_t>>>&)>> pendingCallbacks;
    deque<pair<bool, vector<uint8_t>>> filesRead;

    void onLoadDoneImpl(bool success, const string& data) {
        filesRead.push_back(make_pair(success, vector<uint8_t>(data.begin(), data.end())));
        if(!pendingRequests.front().empty()) {
            // We still have files to load
            handler = URLLoaderHandler::Create(NaCl::getInstance(), pendingRequests.front().front());
            pendingRequests.front().pop();
            handler->onFinishCallback = onLoadDone;
            handler->Start();
        } else {
            // The current request is done! Forward the requested data
            function<void()> mainThreadCallback = bind(pendingCallbacks.front(), filesRead);
            Game::scheduleMainThreadTask(mainThreadCallback);

            // Clear filesRead container
            filesRead.clear();

            pendingRequests.pop();
            pendingCallbacks.pop();
            if(!pendingRequests.empty()) {
                // Proceed to next request
                handler = URLLoaderHandler::Create(NaCl::getInstance(), pendingRequests.front().front());
                pendingRequests.front().pop();
                handler->onFinishCallback = onLoadDone;
                handler->Start();
            } else {
                // No more requests to process.
                handler = NULL;
            }
        }
    }

    template<class Container>
    void requestFiles(Container requestedFiles, const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        queue<string> pendingFiles;
        for(const auto& file: requestedFiles)
        {
            pendingFiles.push("http://localhost:9002/"+file);
        }

        if(!pendingFiles.empty()) {
            pendingRequests.push(pendingFiles);
            pendingCallbacks.push(callback);
        }

        if(handler == NULL) {
            handler = URLLoaderHandler::Create(NaCl::getInstance(), pendingRequests.front().front());
            pendingRequests.front().pop();
            // TODO investigate if this is going to be called from a different thread
            handler->onFinishCallback = onLoadDone;
            handler->Start();
        }
    }
};

#else

class DesktopIO: public IO {
public:
    virtual void requestFiles(const initializer_list<string>& filenames,
            const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        vector<string> filenamesCopy;
        for(auto& fname: filenames)
            filenamesCopy.push_back(fname);

        ThreadPool::startJob(bind(&DesktopIO::requestFiles<const vector<string>&>, this, filenamesCopy, callback));
    }

    virtual void requestFiles(const vector<string>& filenames,
            const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        ThreadPool::startJob(bind(&DesktopIO::requestFiles<const vector<string>&>, this, filenames, callback));
    }

    DesktopIO()
    {}

private:
    template<class Container>
    void requestFiles(Container requestedFiles, const function<void(deque<pair<bool, vector<uint8_t>>>&)> callback)
    {
        deque<pair<bool, vector<uint8_t>>> filesRead;

        for(const auto& filename: requestedFiles) {
            ifstream in;
            in.open(filename.c_str(), ios::in | ios::binary);
            if (in) {
                filesRead.push_back(make_pair(true, vector<uint8_t>((istreambuf_iterator<char>(in)), istreambuf_iterator<char>())));
            } else {
                filesRead.push_back(make_pair(false, vector<uint8_t>()));
            }
        }

        function<void()> mainThreadCallback = bind(callback, filesRead);
        Game::scheduleMainThreadTask(mainThreadCallback);
    }
};

#endif

shared_ptr<IO> IO::instance STATIC_INITIALIZER_GROUP_A;
IO& IO::getInstance() {
    if(instance==NULL) {
#ifdef NACL
        instance = shared_ptr<IO>(dynamic_cast<IO*>(new NaClIO()));
#else
        instance = shared_ptr<IO>(dynamic_cast<IO*>(new DesktopIO()));
#endif
    }
    return *instance;
}

}} // namespace lau::utils

// TODO make this an operator<< on ostream
void printJson(const rapidjson::Value& v) {
    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    v.Accept(writer);
    const char* json = buffer.GetString();
    lau::lout << json << endl;
}


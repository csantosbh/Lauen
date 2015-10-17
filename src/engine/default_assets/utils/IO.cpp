#include <unistd.h>

#include <iostream>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <queue>

#ifdef JAVASCRIPT
#include <emscripten.h>
#endif

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

const IO::LoadStatus& IO::loadStatus() const {
    return loadStatus_;
}

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
        loadStatus_.completedRequests++;
        if(!pendingRequests.front().empty()) {
            // We still have files to load
            handler = URLLoaderHandler::Create(NaCl::getInstance(), pendingRequests.front().front());
            pendingRequests.front().pop();
            handler->onFinishCallback = onLoadDone;
            handler->Start();
        } else {
            // The current request is done! Forward the requested data
            pendingCallbacks.front()(filesRead);

            // Clear filesRead container
            // TODO this is BAD. I should use a smart pointer and never clear here -- rather just create a new deque and let the smart pointer decide when to kill the filesRead.
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

        if(pendingRequests.size() > 0) {
            loadStatus_.totalRequests += requestedFiles.size();
        } else {
            loadStatus_.totalRequests = requestedFiles.size();
            loadStatus_.completedRequests = 0;
        }

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

#elif defined(JAVASCRIPT)

class JavaScriptIO: public IO {
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

    JavaScriptIO()
    {}

private:
    queue<queue<string>> pendingRequests;
    queue<function<void(deque<pair<bool,vector<uint8_t>>>&)>> pendingCallbacks;
    deque<pair<bool, vector<uint8_t>>> filesRead;

    template<class Container>
    void requestFiles(Container requestedFiles, const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        // TODO make this function thread safe! A mutex must be created, and be
        // checked in other places in this class where
        // pendingRequests/pendingCallbacks are touched.
        queue<string> pendingFiles;

        if(pendingRequests.size() > 0) {
            loadStatus_.totalRequests += requestedFiles.size();
        } else {
            loadStatus_.totalRequests = requestedFiles.size();
            loadStatus_.completedRequests = 0;
        }

        for(const auto& file: requestedFiles) {
            pendingFiles.push("http://localhost:9002/"+file);
        }

        if(!pendingFiles.empty()) {
            pendingRequests.push(pendingFiles);
            pendingCallbacks.push(callback);
        }

        // Only calls the async loader if it isn't already running
        if(pendingRequests.size() == 1) {
            emscripten_async_wget_data(pendingRequests.front().front().c_str(), static_cast<void*>(this), &JavaScriptIO::onLoadDoneSuccess, &JavaScriptIO::onLoadDoneError);
            pendingRequests.front().pop();
        }
    }

    static void onLoadDoneError(void* arg) {
        JavaScriptIO* _this = static_cast<JavaScriptIO*>(arg);
        onLoadDone(_this, nullptr, 0);
    }
    static void onLoadDoneSuccess(void* arg, void* data, int size) {
        JavaScriptIO* _this = static_cast<JavaScriptIO*>(arg);
        onLoadDone(_this, data, size);
    }
    static void onLoadDone(JavaScriptIO* _this, void* data, int size) {
        vector<uint8_t> data_vec;

        loadStatus_.completedRequests++;

        if(data != nullptr) {
            data_vec.assign((uint8_t*)data, (uint8_t*)data+size);
        }
        _this->filesRead.push_back(make_pair(data != nullptr, data_vec));

        if(!_this->pendingRequests.front().empty()) {
            // We still have files to load
            emscripten_async_wget_data(_this->pendingRequests.front().front().c_str(), static_cast<void*>(_this), &JavaScriptIO::onLoadDoneSuccess, &JavaScriptIO::onLoadDoneError);
            _this->pendingRequests.front().pop();
        } else {
            // The current request is done! Forward the requested data
            _this->pendingCallbacks.front()(_this->filesRead);

            // Clear filesRead container
            _this->filesRead = deque<pair<bool, vector<uint8_t>>>();

            _this->pendingRequests.pop();
            _this->pendingCallbacks.pop();
            if(!_this->pendingRequests.empty()) {
                // Proceed to next request
                emscripten_async_wget_data(_this->pendingRequests.front().front().c_str(), _this, &JavaScriptIO::onLoadDoneSuccess, &JavaScriptIO::onLoadDoneError);
                _this->pendingRequests.front().pop();
            }
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

        accountForRequests(filenames.size());

        ThreadPool::startJob(bind(&DesktopIO::requestFiles<const vector<string>&>, this, filenamesCopy, callback));
    }

    virtual void requestFiles(const vector<string>& filenames,
            const function<void(deque<pair<bool, vector<uint8_t>>>&)>& callback)
    {
        accountForRequests(filenames.size());

        ThreadPool::startJob(bind(&DesktopIO::requestFiles<const vector<string>&>, this, filenames, callback));
    }

    DesktopIO()
    {}

private:
    mutex mtx_;
    int runningRequests = 0;

    void accountForRequests(unsigned short size) {
        unique_lock<mutex> lock(mtx_);
        if(runningRequests > 0) {
            loadStatus_.totalRequests += size;
        } else {
            loadStatus_.totalRequests = size;
            loadStatus_.completedRequests = 0;
        }

        runningRequests++;
    }

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

        callback(filesRead);

        {
        unique_lock<mutex> lock(mtx_);
        loadStatus_.completedRequests += requestedFiles.size();
        runningRequests--;
        }
    }

};

#endif

shared_ptr<IO> IO::instance STATIC_INITIALIZER_GROUP_A;
IO& IO::getInstance() {
    if(instance==NULL) {
#ifdef NACL
        instance = shared_ptr<IO>(dynamic_cast<IO*>(new NaClIO()));
#elif defined(JAVASCRIPT)
        instance = shared_ptr<IO>(dynamic_cast<IO*>(new JavaScriptIO()));
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


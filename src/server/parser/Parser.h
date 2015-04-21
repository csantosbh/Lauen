#pragma once
#include <memory>
#include <string>

class Parser {
public:
    static const Parser* getInstance();
    virtual void parseFile(const std::string& fileName) const = 0;

protected:
    Parser() { }
    virtual ~Parser() { }

private:
    static std::shared_ptr<Parser> instance;
};


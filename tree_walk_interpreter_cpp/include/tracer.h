#pragma once
#include "ast.h"
#include <string>

class Tracer {
public:
    void setEnabled(bool enabled);
    bool enabled() const;

    void enter(const std::string& node);
    void exit(const std::string& node, const std::string& result = "");
    void event(const std::string& message);

private:
    bool enabled_ = false;
    int depth_ = 0;

    std::string indent() const;
};

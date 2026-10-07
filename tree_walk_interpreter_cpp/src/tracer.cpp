#include "tracer.h"
#include <iostream>

void Tracer::setEnabled(bool enabled) {
    enabled_ = enabled;
    depth_ = 0;
}

bool Tracer::enabled() const {
    return enabled_;
}

std::string Tracer::indent() const {
    return std::string(static_cast<size_t>(depth_) * 2, ' ');
}

void Tracer::enter(const std::string& node) {
    if (!enabled_) return;
    std::cout << indent() << "[ENTER] " << node << '\n';
    ++depth_;
}

void Tracer::exit(const std::string& node, const std::string& result) {
    if (!enabled_) return;
    if (depth_ > 0) --depth_;
    std::cout << indent() << "[EXIT]  " << node;
    if (!result.empty()) std::cout << " -> " << result;
    std::cout << '\n';
}

void Tracer::event(const std::string& message) {
    if (!enabled_) return;
    std::cout << indent() << "[TRACE] " << message << '\n';
}

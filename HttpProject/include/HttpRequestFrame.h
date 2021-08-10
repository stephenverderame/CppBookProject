#pragma once
#include "HttpFrame.h"
/// The HTTP data frame sent to servers to request
/// a resource
class HttpRequestFrame : public HttpFrame {
public:
    /// Content data of request
    std::string content;
    /// Path of resource to request
    std::string path;
};
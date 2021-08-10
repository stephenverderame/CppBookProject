#pragma once
#include "HttpFrame.h"
/// The HTTP response frame sent from servers to clients
/// to return information
class HttpResponseFrame : public HttpFrame {
public:
    std::string responseCode;
};
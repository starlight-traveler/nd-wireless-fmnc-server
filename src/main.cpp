#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include <iostream>
#include "logger.h"
#include "version.h"

#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
// HTTPS Server
httplib::SSLServer svr("./cert.pem", "./key.pem");
#else
// HTTP Server
httplib::Server svr;
#endif

int main() {

    quill::Logger *logger = initialize_logger();

    LOG_DEBUG(logger, "Build date: {}", BUILD_DATE);
    LOG_DEBUG(logger, "Project version: {}", PROJECT_VERSION);

    svr.Get("/hi", [](const httplib::Request&, httplib::Response& res) {
        res.set_content("Hello World!", "text/plain");
    });

    svr.listen("0.0.0.0", 8085);

}


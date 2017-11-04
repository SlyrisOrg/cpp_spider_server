//
// Created by doom on 03/11/17.
//

#ifndef SPIDER_SERVER_API_HPP
#define SPIDER_SERVER_API_HPP

#undef always_inline
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <cpprest/asyncrt_utils.h>

namespace spi
{
    class API
    {
    public:
        static const std::string apiHost;

        static pplx::task<void> addRequest(const web::json::value &value)
        {
//            std::cout << __PRETTY_FUNCTION__ << std::endl;
//            std::cout << value.serialize().c_str() << std::endl;
            web::http::client::http_client client(U(apiHost + "log"));
            web::http::http_request request;
            request.set_method(web::http::methods::POST);
            request.headers().add(U("Content-Type"), U("application/json"));
            request.set_body(value.serialize());
            return client.request(request).then([](const web::http::http_response &response) {
                try {
                    if (response.status_code() == web::http::status_codes::OK) {
                        return response.extract_json();
                    }
                }
                catch (const web::http::http_exception &error) {
                    std::cerr << error.what() << std::endl;
                }
                return pplx::task_from_result(web::json::value());
            }).then([](const pplx::task<web::json::value> &previousTask) {
                try {
                    if (previousTask._GetImpl()->_HasUserException()) {
                        auto holder = previousTask._GetImpl()->_GetExceptionHolder();
                        holder->_RethrowUserException();
                    }
                    web::json::value obj = previousTask.get();
                    std::cout << obj.serialize().c_str() << std::endl;
                }
                catch (const web::http::http_exception &e) {
                    std::cerr << e.what() << std::endl;
                }
            });
        }
    };

    const std::string API::apiHost = "http://localhost:3000/";
}

#endif //SPIDER_SERVER_API_HPP

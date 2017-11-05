//
// Created by milerius on 02/11/17.
//

#ifndef SPIDER_SERVER_APICONNECTEDLOGHANDLE_HPP
#define SPIDER_SERVER_APICONNECTEDLOGHANDLE_HPP

#include <string>
#include <vector>
#include <Logging/LogModule.hpp>
#include "API.hpp"


namespace spi::log
{
    class APIConnectedLogModule final : public LogModule
    {
    public:
        void appendEntry(const ILoggable &loggable) noexcept final
        {
            auto value = web::json::value::parse(U(loggable.JSONify()));
            value[U("clientID")] = web::json::value::string(_id);
            _values.emplace_back(value);
            if (_values.size() == 25) {
                flush();
            }
        }

        void flush() noexcept final
        {
            web::json::value v;

            v[U("logs")] = std::move(web::json::value::array(_values));
            std::cout << v.serialize() << std::endl;
            API::addRequest(v).wait();
            _values.clear();
        }

        void setRoot([[maybe_unused]] const std::string &root) noexcept override
        {
        }

        void setID(const std::string &id) noexcept override
        {
            _id = id;
        }

        void setIOManager([[maybe_unused]] net::IOManager &mgr) noexcept override
        {
        }

        bool setup() noexcept override
        {
            return true;
        }

        static LogModule *create() noexcept
        {
            return new APIConnectedLogModule();
        }

    private:
        std::string _id;
        std::vector<web::json::value> _values;
    };
}

#endif //SPIDER_SERVER_APICONNECTEDLOGHANDLE_HPP

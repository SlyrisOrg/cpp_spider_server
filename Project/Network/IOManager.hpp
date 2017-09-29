//
// Created by doom on 28/09/17.
//

#ifndef SPIDER_SERVER_IOSERVICE_HPP
#define SPIDER_SERVER_IOSERVICE_HPP

#include <boost/asio/io_service.hpp>

namespace asio = boost::asio;

namespace spi::net
{
    class IOManager
    {
    public:
        using InternalT = asio::io_service;

        InternalT &get() noexcept
        {
            return _service;
        }

        void run()
        {
            _service.run();
        }

        void stop()
        {
            _service.stop();
        }

    private:
        InternalT _service;
    };
}

#endif //SPIDER_SERVER_IOSERVICE_HPP

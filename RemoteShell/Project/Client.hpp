//
// Created by roman sztergbaum on 04/10/2017.
//

#ifndef SPIDER_CLIENT_REMOTESHELL_HPP
#define SPIDER_CLIENT_REMOTESHELL_HPP

#include <memory>
#include <functional>
#include <utils/NonCopyable.hpp>
#include <utils/NonMovable.hpp>
#include <utils/CLI.hpp>
#include <Network/IOManager.hpp>
#include <Network/SSLContext.hpp>
#include <Network/SSLConnection.hpp>
#include "ClientSession.hpp"

namespace sh
{
    class Client : private utils::NonCopyable, private utils::NonMovable
    {
    public:
        explicit Client(const sh::Config &cfg) noexcept : _cfg(cfg)
        {
        }

        void run()
        {
            _clientSession->setup(_cfg);
            _clientSession->run();
        }

    private:
        sh::Config _cfg;
        spi::net::SSLContext _ctx{spi::net::SSLContext::SSLv23Client};
        spi::net::IOManager _service;
        std::unique_ptr<ClientSession> _clientSession{std::make_unique<ClientSession>(_ctx, _service)};
    };
}

#endif //SPIDER_CLIENT_REMOTESHELL_HPP

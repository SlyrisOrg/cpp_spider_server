//
// Created by doom on 26/09/17.
//

#ifndef SPIDER_SERVER_SPIDERCORE_HPP
#define SPIDER_SERVER_SPIDERCORE_HPP

#include <string>
#include <Logging/LogManager.hpp>
#include <Server/Server.hpp>
#include <log/Logger.hpp>

namespace spi
{
    /**
     * Class responsible for spawning the different server components
     */
    class Core
    {
    public:
        struct Config
        {
            std::string keyFile;
            std::string certFile;
            std::string logRoot;
            unsigned short port;
            unsigned short shellPort;
        };

        explicit Core(const Config &config) noexcept : _conf(config), _log("core", logging::Level::Debug)
        {
            _log(logging::Level::Info) << "Configuring Spider Core" << std::endl;
            _log(logging::Level::Info) << "Using port " << _conf.port << " for clients" << std::endl;
            _log(logging::Level::Info) << "Using port " << _conf.shellPort << " for remote control" << std::endl;
            _log(logging::Level::Info) << "Using SSL certificate " << _conf.certFile << std::endl;
            _log(logging::Level::Info) << "Using SSL private key " << _conf.keyFile << std::endl;
            _log(logging::Level::Info) << "Using '" << _conf.logRoot << "' to store logs" << std::endl;
        }

        bool start()
        {
            _log(logging::Level::Debug) << "Starting now" << std::endl;

            if (!_logMgr.setup(_conf.logRoot)
                || !_server.setup(_conf.port, _conf.shellPort, _conf.certFile, _conf.keyFile, _conf.logRoot))
                return false;

            _log(logging::Level::Info) << "Core started successfully" << std::endl;
            return _server.run();
        }

        virtual ~Core() noexcept
        {
        }

    private:
        Config _conf;
        logging::Logger _log;
        spi::LogManager _logMgr;
        spi::Server _server;
    };
}

#endif //SPIDER_SERVER_SPIDERCORE_HPP

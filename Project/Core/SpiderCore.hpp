//
// Created by doom on 26/09/17.
//

#ifndef SPIDER_SERVER_SPIDERCORE_HPP
#define SPIDER_SERVER_SPIDERCORE_HPP

#include <string>
#include <Logging/LogManager.hpp>
#include <Server/Server.hpp>
#include <log/Logger.hpp>
#include <lib/Lib.hpp>
#include <Logging/Modules/RotatingFileLogModule/RotatingFileLogModule.hpp>

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
            std::string logModule;
            unsigned short port;
            unsigned short shellPort;
        };

    private:
        bool __loadLogModule(const std::string &name) noexcept
        {
            if (name == "default") {
                _logCtor = &log::RotatingFileLogModule::create;
            } else {
                _lib.load(name);
                if (!_lib.isLoaded()) {
                    _log(logging::Level::Error) << "Unable to load the log module" << std::endl;
                    return false;
                }
                try {
                    _logCtor = _lib.get<LogHandleConstructor>("create");
                } catch (const lib::SymbolNotFound &e) {
                    _log(logging::Level::Error) << "Unable to load a symbol from the log module" << std::endl;
                    return false;
                }
            }
            _log(logging::Level::Info) << "Logging module loaded successfully" << std::endl;
            return true;
        }

    public:
        explicit Core(const Config &config) noexcept : _conf(config), _log("core", logging::Level::Debug)
        {
            _log(logging::Level::Info) << "Configuring Spider Core" << std::endl;
            _log(logging::Level::Info) << "Using port " << _conf.port << " for clients" << std::endl;
            _log(logging::Level::Info) << "Using port " << _conf.shellPort << " for remote control" << std::endl;
            _log(logging::Level::Info) << "Using SSL certificate " << _conf.certFile << std::endl;
            _log(logging::Level::Info) << "Using SSL private key " << _conf.keyFile << std::endl;
            _log(logging::Level::Info) << "Using '" << _conf.logRoot << "' to store logs" << std::endl;
            _log(logging::Level::Info) << "Using '" << config.logModule << "' module as loghandle" << std::endl;
        }

        bool start()
        {
            _log(logging::Level::Debug) << "Starting now" << std::endl;

            if (!__loadLogModule(_conf.logModule) || !_logMgr.setup(_conf.logRoot)
                || !_server.setup(_conf.port, _conf.shellPort, _conf.certFile, _conf.keyFile, _conf.logRoot, _logCtor))
                return false;

            _log(logging::Level::Info) << "Core started successfully" << std::endl;
            return _server.run();
        }

        virtual ~Core() noexcept
        {
        }

    private:
        Config _conf;
        lib::SharedLibrary _lib;
        LogHandleConstructor _logCtor;
        logging::Logger _log;
        spi::LogManager _logMgr;
        spi::Server _server;
    };
}

#endif //SPIDER_SERVER_SPIDERCORE_HPP

//
// Created by doom on 26/09/17.
//

#ifndef SPIDER_SERVER_SERVER_HPP
#define SPIDER_SERVER_SERVER_HPP

#include <boost/bind.hpp>
#include <log/Logger.hpp>
#include <Network/ErrorCode.hpp>
#include <Network/IOManager.hpp>
#include <Network/TCPAcceptor.hpp>
#include <Network/SSLContext.hpp>
#include <Server/SpiderClientSession.hpp>
#include <Server/ShellClientSession.hpp>
#include <Logging/LogModule.hpp>

namespace asio = boost::asio;

namespace spi
{
    class Server
    {
    public:
        Server() noexcept : _ctx(net::SSLContext::Version::SSLv23),
                            _clientsAcceptor(_ioMgr), _rshAcceptor(_ioMgr),
                            _log("server", logging::Level::Debug)
        {
        }

        virtual ~Server() noexcept
        {
            for (auto &cur : _clients) {
                delete cur;
            }
            delete _nextSession;
            delete _shellSession;
        }

        bool setup(unsigned short port, unsigned short shellPort,
                   const std::string &certFile, const std::string &keyFile,
                   const std::string &logRoot, LogHandleConstructor ctor)
        {
            _logCtor = ctor;
            _log(logging::Level::Debug) << "Setting up server on ports " << port << " and " << shellPort << std::endl;

            if (_clientsAcceptor.bind(port) || _rshAcceptor.bind(shellPort)) {
                _log(logging::Level::Error) << "Unable to listen for incoming connections" << std::endl;
                return false;
            }

            if (!_ctx.useCertificateFile(certFile) || !_ctx.usePrivateKeyFile(keyFile)) {
                _log(logging::Level::Error) << "Failed setting up a valid SSL context" << std::endl;
                return false;
            }

            _log(logging::Level::Info) << "Setup successful" << std::endl;

            _logRoot = logRoot;
            return true;
        }

    private:
        void __addClient(SpiderClientSession *clt) noexcept
        {
            _clients.push_back(clt);
        }

        void __removeClient(CommandableSession *clt)
        {
            auto *spiClt = static_cast<SpiderClientSession *>(clt);

            _clients.erase(std::remove_if(_clients.begin(), _clients.end(), [spiClt](const auto &cur) {
                return cur == spiClt;
            }));
            delete spiClt;
        }

        void __removeRemoteShell(CommandableSession *clt)
        {
            delete clt;
            __setupRshAcceptor();
        }

        void __onClientAccept(SpiderClientSession *sess, const ErrorCode &ec)
        {
            if (!ec) {
                _log(logging::Level::Debug) << "Got a new connection" << std::endl;
                sess->startSession();
                __addClient(sess);
                sess->onError(boost::bind(&Server::__removeClient, this, _1));
            } else {
                delete sess;
            }
            __setupClientAcceptor();
        }

        void __onShellAccept(const ErrorCode &ec)
        {
            if (!ec) {
                _log(logging::Level::Info) << "Remote shell connected" << std::endl;
                _shellSession->startSession();
            } else {
                delete _shellSession; //Will call the cleanup function, which will restart the acceptor
            }
        }

        void __setupClientAcceptor()
        {
            _nextSession = new SpiderClientSession(_ioMgr, _ctx, _logRoot, _logCtor());

            _clientsAcceptor.onAccept(_nextSession->connection(),
                                      boost::bind(&Server::__onClientAccept, this, _nextSession,
                                                  net::ErrorPlaceholder));
        }

        void __setupRshAcceptor()
        {
            _shellSession = new ShellClientSession(_ioMgr, _ctx, _clients);

            _shellSession->onError(boost::bind(&Server::__removeRemoteShell, this, _1));
            _rshAcceptor.onAccept(_shellSession->connection(),
                                  boost::bind(&Server::__onShellAccept, this, net::ErrorPlaceholder));
        }

        void __setupSigHandlers()
        {
            _ioMgr.onTerminationSignals(boost::bind(&Server::stop, this));
        }

    public:
        bool run()
        {
            __setupClientAcceptor();
            __setupRshAcceptor();
            __setupSigHandlers();
            _ioMgr.run();
            return true;
        }

        void stop()
        {
            _ioMgr.stop();
        }

    private:
        net::SSLContext _ctx;
        net::IOManager _ioMgr;

        net::TCPAcceptor _clientsAcceptor;
        net::TCPAcceptor _rshAcceptor;

        SpiderClientSession *_nextSession{nullptr};
        std::vector<SpiderClientSession *> _clients;

        ShellClientSession *_shellSession{nullptr};

        std::string _logRoot;

        logging::Logger _log;

        LogHandleConstructor _logCtor{nullptr};
    };
}

#endif //SPIDER_SERVER_SERVER_HPP

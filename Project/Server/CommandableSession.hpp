//
// Created by doom on 07/10/17.
//

#ifndef SPIDER_SERVER_COMMANDABLECLIENTSESSION_HPP
#define SPIDER_SERVER_COMMANDABLECLIENTSESSION_HPP

#include <boost/bind.hpp>
#include <log/Logger.hpp>
#include <Network/ErrorCode.hpp>
#include <Network/BufferView.hpp>
#include <Network/SSLConnection.hpp>
#include <Protocol/CommandHandler.hpp>

namespace spi
{
    class CommandableSession
    {
    public:
        CommandableSession(net::IOManager &io, net::SSLContext &ctx, const std::string &name) :
            _conn(io, ctx), _log(name, logging::Level::Debug)
        {
        }

        virtual ~CommandableSession() noexcept
        {
        }

        void onError(std::function<void(CommandableSession *)> &&fct) noexcept
        {
            _errorCb = std::forward<std::function<void(CommandableSession *)>>(fct);
        }

        template <typename CallBackT>
        void asyncHandshake(net::SSLConnection::HandshakeType type, CallBackT &&cb)
        {
            _conn.asyncHandshake(type, std::forward<CallBackT>(cb));
        }

        net::SSLConnection &connection() noexcept
        {
            return _conn;
        }

    protected:
        void handleHandshake(const ErrorCode &ec)
        {
            if (!ec) {
                __readCommandHeader();
            } else {
                _log(logging::Level::Warning) << "Unable to perform SSL handshake with client: "
                                              << ec.message() << std::endl;
                _errorCb(this);
            }
        }

        /** Command reading / handling */

    private:
        void __readCommandHeaderChunk() noexcept
        {
            _conn.asyncReadSome(net::BufferView(_readBuff.data() + _nbReadBytes,
                                                _readBuff.size() - _nbReadBytes),
                                boost::bind(&CommandableSession::__handleCommandHeader, this,
                                            net::ErrorPlaceholder, net::BytesTransferredPlaceholder));
        }

        void __readCommandHeader() noexcept
        {
            _nbReadBytes = 0;
            _expectedSize = 4;
            _readBuff.resize(4);
            __readCommandHeaderChunk();
        }

        void __handleCommandHeader(const ErrorCode &ec, size_t bytesTransferred)
        {
            if (ec) {
                _log(logging::Level::Warning) << "Unable to read command header: " << ec.message() << std::endl;
                _errorCb(this);
                return;
            }

            _nbReadBytes += bytesTransferred;
            if (_nbReadBytes < _expectedSize) {
                __readCommandHeaderChunk();
            } else {
                auto type = _cmdHandler.identifyMessage(_readBuff);

                if (type == spi::proto::MessageType::Unknown) {
                    _log(logging::Level::Warning) << "Ignoring unrecognized command" << std::endl;
                    __readCommandHeader();
                }
                __handleCommandType(type);
            }
        }

        void __handleCommandType(proto::MessageType type)
        {
            if (!_cmdHandler.canHandleCommand(type)) {
                _log(logging::Level::Warning) << "Rejecting unexpected command " << type.toString() << std::endl;
                _errorCb(this);
            } else if (_cmdHandler.getSerializedSize(type) == 0) {
                _cmdHandler.handleBinaryCommand(type, Buffer());
            } else {
                __readCommandBody(type);
            }
        }

        void __readCommandChunk() noexcept
        {
            _conn.asyncReadSome(net::BufferView(_readBuff.data() + _nbReadBytes,
                                                _readBuff.size() - _nbReadBytes),
                                boost::bind(&CommandableSession::__handleCommand, this,
                                            net::ErrorPlaceholder, net::BytesTransferredPlaceholder));
        }

        void __readCommandBody(spi::proto::MessageType type) noexcept
        {
            if (!_cmdHandler.canHandleCommand(type)) {
                _log(logging::Level::Warning) << "Received unexpected command " << type.toString() << std::endl;
                _errorCb(this);
                return;
            }

            size_t size = _cmdHandler.getSerializedSize(type);

            if (size == CommandHandler::invalidSize) {
                _log(logging::Level::Warning) << "Received unexpected command " << type.toString() << std::endl;
                _errorCb(this);
            } else {
                _nextCmdType = type;
                _nbReadBytes = 0;
                _expectedSize = size;
                _readBuff.resize(size);
                __readCommandChunk();
            }
        }

        void __handleCommand(const ErrorCode &ec, size_t bytesTransferred)
        {
            if (!ec) {
                _nbReadBytes += bytesTransferred;
                if (_nbReadBytes < _expectedSize) {
                    __readCommandChunk();
                } else {
                    _cmdHandler.handleBinaryCommand(_nextCmdType, _readBuff);
                    __readCommandHeader();
                }
            } else {
                _log(logging::Level::Warning) << "Unable to read command data: " << ec.message() << std::endl;
                _errorCb(this);
            }
        }

    protected:
        net::SSLConnection _conn;
        logging::Logger _log;
        CommandHandler _cmdHandler;
        std::function<void(CommandableSession *)> _errorCb{[](CommandableSession *) {}};

        std::vector<spi::Byte> _readBuff;
        size_t _nbReadBytes{0};
        size_t _expectedSize{0};
        proto::MessageType _nextCmdType{proto::MessageType::Unknown};
    };
}

#endif //SPIDER_SERVER_COMMANDABLECLIENTSESSION_HPP

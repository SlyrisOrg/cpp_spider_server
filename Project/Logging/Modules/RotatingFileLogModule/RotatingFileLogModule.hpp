//
// Created by doom on 29/09/17.
//

#ifndef SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP
#define SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP

#include <fstream>
#include <boost/filesystem.hpp>
#include <Logging/LogModule.hpp>

namespace bfs = boost::filesystem;

namespace spi::log
{
    class RotatingFileLogModule : public LogModule
    {
    public:
        ~RotatingFileLogModule() noexcept override
        {
            flush();
        }

    private:
        unsigned long __getFileNb() const noexcept
        {
            unsigned long max = 0;
            bool looped = false;

            bfs::directory_iterator end;
            for (bfs::directory_iterator it(_handleDirectory); it != end; ++it) {
                try {
                    auto n = std::stoul(it->path().stem().string());
                    if (max <= n) {
                        looped = true;
                        max = n;
                    }
                } catch (const std::invalid_argument &e) {
                }
            }
            return looped ? max + 1 : 0;
        }

        void __rotate() noexcept
        {
            if (_out.is_open()) {
                flush();
                _out.close();
            }
            if (_fileNb == 0) {
                _fileNb = __getFileNb();
            } else {
                ++_fileNb;
            }
            bfs::path outPath = _handleDirectory / std::to_string(_fileNb).append(".log");
            _out.open(outPath.string());
            _logSize = 0;
        }

    public:
        void setRoot(const std::string &root) noexcept override
        {
            _baseDirectory = root;
        }

        void setID(const std::string &name) noexcept override
        {
            auto cpy = name;
            const std::string reject = "\\:?| ";

            for (auto &cur : cpy) {
                if (reject.find(cur) != std::string::npos) {
                    cur = '_';
                }
            }
            _handleDirectory = _baseDirectory / cpy;
        }

        void setIOManager([[maybe_unused]] net::IOManager &manager) noexcept override
        {
            //This logger doesn't need an IOManager
        }

        bool setup() noexcept override
        {
            if (!bfs::exists(_baseDirectory) && !bfs::create_directories(_baseDirectory)) {
                return false;
            }

            if (!bfs::exists(_handleDirectory) && !bfs::create_directories(_handleDirectory)) {
                return false;
            }

            __rotate();
            return true;
        }

        void appendEntry(const ILoggable &loggable) noexcept override
        {
            if (_logSize > _logThreshold) {
                __rotate();
            }
            _out << loggable.stringify() << std::endl;
        }

        void flush() noexcept override
        {
            _out.flush();
        }

        static LogModule *create() noexcept
        {
            return new RotatingFileLogModule();
        }

    private:
        static constexpr const unsigned long _logThreshold = 1024;

        bfs::path _baseDirectory;
        bfs::path _handleDirectory;
        unsigned long _logSize{0};
        unsigned long _fileNb{0};
        std::ofstream _out;
    };
}

#endif //SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP

//
// Created by doom on 29/09/17.
//

#ifndef SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP
#define SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP

#include <fstream>
#include <boost/filesystem.hpp>
#include <Logging/AbstractLogHandle.hpp>

namespace fs = boost::filesystem;

namespace spi::log
{
    class RotatingFileLogHandle : public AbstractLogHandle
    {
    public:
        RotatingFileLogHandle(const fs::path &root) noexcept : _baseDirectory(root)
        {
        }

        ~RotatingFileLogHandle() noexcept override
        {
        }

    private:
        unsigned long __getFileNb() const noexcept
        {
            unsigned long max = 0;
            bool looped = false;

            fs::directory_iterator end;
            for (fs::directory_iterator it(_handleDirectory); it != end; ++it) {
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
            if (_out.is_open())
                _out.close();
            if (_fileNb == 0) {
                _fileNb = __getFileNb();
            } else {
                ++_fileNb;
            }
            fs::path outPath = _handleDirectory / std::to_string(_fileNb).append(".log");
            _out.open(outPath.string());
            _logSize = 0;
        }

    public:
        void setHandleName(std::string name) noexcept
        {
            const std::string reject = "\\:?| ";

            for (auto &cur : name) {
                if (reject.find(cur) != std::string::npos) {
                    cur = '_';
                }
            }
            _handleDirectory = _baseDirectory / name;
        }

        bool setup() noexcept
        {
            if (!fs::exists(_handleDirectory) && !fs::create_directories(_handleDirectory)) {
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

    private:
        static constexpr const unsigned long _logThreshold = 1024;

        fs::path _baseDirectory;
        fs::path _handleDirectory;
        unsigned long _logSize{0};
        unsigned long _fileNb{0};
        std::ofstream _out;
    };
}

#endif //SPIDER_LOG_ROTATINGFILELOGHANDLE_HPP

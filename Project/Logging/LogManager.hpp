//
// Created by doom on 26/09/17.
//

#ifndef SPIDER_LOG_ROTATINGFILELOGMANAGER_HPP
#define SPIDER_LOG_ROTATINGFILELOGMANAGER_HPP

#include <string>
#include <unordered_map>
#include <boost/filesystem.hpp>
#include <log/Logger.hpp>

namespace fs = boost::filesystem;

namespace spi
{
    class LogManager
    {
    public:
        bool setup(const fs::path &logRoot) noexcept
        {
            _log(logging::Level::Debug) << "Setting up logging at " << logRoot << std::endl;
            if (fs::exists(logRoot) && !fs::is_directory(logRoot)) {
                _log(logging::Level::Error) << "Setup failure: " << logRoot
                                            << " already exists and is not a directory" << std::endl;
                return false;
            }
            if (!fs::exists(logRoot) && !fs::create_directories(logRoot)) {
                _log(logging::Level::Error) << "Setup failure: unable to create directory " << logRoot << std::endl;
                return false;
            }
            _log(logging::Level::Info) << "LogManager started successfully" << std::endl;
            _logRoot = logRoot;
            return true;
        }

    private:
        logging::Logger _log{"log-manager", logging::Level::Debug};
        fs::path _logRoot;
    };
}

#endif //SPIDER_LOG_ROTATINGFILELOGMANAGER_HPP

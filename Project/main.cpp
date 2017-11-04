//
// Created by roman sztergbaum on 25/09/2017.
//

#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <Core/SpiderCore.hpp>

namespace opt = boost::program_options;

static bool fillConf(spi::Core::Config &conf, const opt::variables_map &vm) noexcept
{
    conf.logRoot = vm["log_dir"].as<std::string>();
    conf.port = vm["port"].as<unsigned short>();
    conf.shellPort = vm["cmd_port"].as<unsigned short>();
    conf.certFile = vm["cert"].as<std::string>();
    conf.keyFile = vm["key"].as<std::string>();
    conf.logModule = vm["log_module"].as<std::string>();
    return true;
}

int main(int ac, char **av)
{
    opt::options_description desc("Available options");

    desc.add_options()
        ("port", opt::value<unsigned short>()->default_value(31337), "the port at which to listen")
        ("log_dir", opt::value<std::string>()->default_value("spider_logs"), "the directory in which to store log data")
        ("cmd_port", opt::value<unsigned short>()->default_value(31338), "port on which to allow shell connection")
        ("cert", opt::value<std::string>()->default_value("cert.pem"), "the certificate to use for SSL connections")
        ("key", opt::value<std::string>()->default_value("key.pem"), "the private key to use for SSL connections")
        ("log_module", opt::value<std::string>()->default_value("default"), "the path to the log module to use")
        ("help", "display this help message");

    opt::variables_map vm;

    try {
        opt::store(opt::parse_command_line(ac, (const char *const *)av, desc), vm);
        opt::notify(vm);
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (!vm["help"].empty()) {
        std::cout << desc << std::endl;
        return 0;
    }

    spi::Core::Config conf;
    if (!fillConf(conf, vm)) {
        std::cerr << "Invalid configuration !" << std::endl;
        return 1;
    }
    spi::Core server(conf);

    try {
        if (!server.start()) {
            std::cerr << "Unable to start server !" << std::endl;
            return 1;
        }
    } catch (...) {
        std::cerr << "An unknown error occurred !" << std::endl;
    }
    return 0;
}
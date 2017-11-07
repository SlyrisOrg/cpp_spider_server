//
// Created by roman sztergbaum on 04/10/2017.
//

#include <boost/program_options.hpp>
#include <iostream>
#include "Client.hpp"

namespace opt = boost::program_options;

static bool fillConf(sh::Config &conf, const opt::variables_map &vm) noexcept
{
    conf.port = vm["port"].as<unsigned short>();
    conf.address = vm["address"].as<std::string>();
    return true;
}

int main(int ac, char **av)
{
    try {
        opt::options_description desc("Available options");
        desc.add_options()
            ("port", opt::value<unsigned short>()->default_value(31338), "the port at which to bind")
            ("address", opt::value<std::string>()->default_value("79.137.42.80"))
            ("help", "display this help message");
        opt::variables_map vm;
        opt::store(opt::parse_command_line(ac, static_cast<const char *const *>(av), desc), vm);
        opt::notify(vm);
        sh::Config conf{};
        if (!fillConf(conf, vm)) {
            std::cerr << "Invalid configuration !" << std::endl;
            return EXIT_FAILURE;
        }

        if (!vm["help"].empty()) {
            std::cout << desc << std::endl;
            return 0;
        }

        sh::Client shell(conf);

        shell.run();
    }
    catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
    }
    return 0;
}
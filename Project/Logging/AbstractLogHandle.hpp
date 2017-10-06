//
// Created by doom on 02/10/17.
//

#ifndef SPIDER_LOG_ABSTRACTLOGHANDLER_HPP
#define SPIDER_LOG_ABSTRACTLOGHANDLER_HPP

#include <string>
#include <utils/NonCopyable.hpp>
#include <Utils/ILoggable.hpp>

namespace spi
{
    class AbstractLogHandle : public utils::NonCopyable
    {
    public:
        ~AbstractLogHandle() noexcept override
        {
        }

        virtual void appendEntry(const ILoggable &) = 0;

        virtual void flush() = 0;
    };
}

#endif //SPIDER_LOG_ABSTRACTLOGHANDLER_HPP

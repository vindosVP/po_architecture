#include "cache.h"
#include "../config/config.h"

#include <exception>
#include <mutex>

#include <redis-cpp/stream.h>
#include <redis-cpp/execute.h>

namespace database
{
    std::mutex _mtx;
    Cache::Cache()
    {
        std::string host = Config::get().get_cache().substr(0, Config::get().get_cache().find(':'));
        std::string port = Config::get().get_cache().substr(Config::get().get_cache().find(':') + 1);

        std::cout << "cache host:" << host <<" port:" << port << std::endl;
        _stream = rediscpp::make_stream(host, port);
    }

    Cache Cache::get()
    {
        static Cache instance;
        return instance;
    }

    void Cache::put([[maybe_unused]] long key, [[maybe_unused]] const std::string &val)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        rediscpp::value response = rediscpp::execute(*_stream, "set", std::to_string(key), val, "ex", "60");
    }

    void Cache::put([[maybe_unused]] std::string key, [[maybe_unused]] const std::string &val)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        rediscpp::value response = rediscpp::execute(*_stream, "set", key, val, "ex", "60");
    }

    bool Cache::get([[maybe_unused]] long key, [[maybe_unused]] std::string &val)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        rediscpp::value response = rediscpp::execute(*_stream, "get", std::to_string(key));

        if (response.is_error_message())
            return false;
        if (response.empty())
            return false;

        val = response.as<std::string>();
        return true;
    }

    bool Cache::get([[maybe_unused]] std::string key, [[maybe_unused]] std::string &val)
    {
        std::lock_guard<std::mutex> lck(_mtx);
        rediscpp::value response = rediscpp::execute(*_stream, "get", key);

        if (response.is_error_message())
            return false;
        if (response.empty())
            return false;

        val = response.as<std::string>();
        return true;
    }
}
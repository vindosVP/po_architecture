#ifndef CACHE_H
#define CACHE_H

#include <string>
#include <iostream>
#include <memory>


namespace database
{
    class Cache
    {
        private:
            std::shared_ptr<std::iostream> _stream;

            Cache();
        public:
            static Cache get();
            void put(long key, const std::string& val);
            void put(std::string key, const std::string& val);
            bool get(long key, std::string& val);
            bool get(std::string key, std::string& val);

    };
}

#endif
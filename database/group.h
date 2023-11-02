#ifndef GROUP_H
#define GROUP_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include <optional>
#include "message.h"

namespace database
{
    class Group{
        private:
            long _id;
            long _admin_id;
            std::string _title;

        public:

            static Group fromJSON(const std::string & str);
            Poco::JSON::Object::Ptr toJSON() const;

            long                  get_id() const;
            long                  get_admin_id() const;
            const std::string     &get_title() const;

            long&            id();
            long&            admin_id();
            std::string&     title();

            static void init();
            static std::optional<Group> get_by_id(long id);
            void save_to_mysql();
            void add_user(long user_id);
            std::vector<std::tuple<Message, bool>> list_messages(long requester_id);
    };
}

#endif
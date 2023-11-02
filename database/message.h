#ifndef MESSAGE_H
#define MESSAGE_H

#include <string>
#include <vector>
#include "Poco/JSON/Object.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTime.h"
#include <optional>
#include "user.h"

namespace database
{
    class Message{
        private:
            long _id;
            long _author_id;
            long _recipient_id;
            long _group_id;
            std::string _text;
            Poco::DateTime _created_at;

        public:

            static Message fromJSON(const std::string & str);
            Poco::JSON::Object::Ptr toJSON() const;

            long                 get_id() const;
            long                 get_author_id() const;
            long                 get_recipient_id() const;
            long                 get_group_id() const;
            const std::string    &get_text() const;
            const Poco::DateTime &get_created_at() const;

            long&            id();
            long&            author_id();
            long&            recipient_id();
            long&            group_id();
            std::string&     text();
            Poco::DateTime& created_at();

            static void init();
            static std::optional<Message> get_by_id(long id);
            void save_to_mysql(bool to_group=false);
            static std::vector<std::tuple<Message, bool>> list_chat(long requester_id, long other_id);         


    };
}

#endif
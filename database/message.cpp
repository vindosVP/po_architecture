#include "message.h"
#include "user.h"
#include "database.h"
#include "../config/config.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void Message::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement create_stmt(session);
                create_stmt << "CREATE TABLE IF NOT EXISTS `Message` (`id` INT NOT NULL AUTO_INCREMENT,"
                            << "`text` VARCHAR(4096) NULL,"
                            << "`created_at` DATETIME NOT NULL DEFAULT NOW(),"
                            << "`author_id` INT NOT NULL,"
                            << "`recipient_id` INT,"
                            << "`group_id` INT,"
                            << "PRIMARY KEY (`id`),KEY `a_id` (`author_id`),KEY `r_id` (`recipient_id`),KEY `g_id` (`group_id`));"
                            << hint,
                    now;
            }
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    Poco::JSON::Object::Ptr Message::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("text", _text);
        root->set("created_at", _created_at);
        root->set("author_id", _author_id);
        root->set("recipient_id", _recipient_id);
        root->set("group_id", _group_id);
        return root;
    }

    Message Message::fromJSON(const std::string &str)
    {
        Message message;
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        message.id() = object->getValue<long>("id");
        message.created_at() = object->getValue<Poco::DateTime>("created_at");
        message.author_id() = object->getValue<long>("author_id");
        message.recipient_id() = object->getValue<long>("recipient_id");
        message.group_id() = object->getValue<long>("group_id");
        message.text() = object->getValue<std::string>("text");

        return message;
    }

    std::optional<Message> Message::get_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Message a;
            select << "SELECT id, text, created_at, author_id, recipient_id, group_id FROM Message WHERE id=?",
                into(a._id),
                into(a._text),
                into(a._created_at),
                into(a._author_id),
                into(a._recipient_id),
                into(a._group_id),
                use(id),
                range(0, 1); 

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return a;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;   
        }
        return {};
    }

    std::vector<std::tuple<Message, bool>> Message::list_chat(long requester_id, long other_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<std::tuple<Message, bool>> result;
            Message a;
            select << "SELECT id, text, created_at, author_id, recipient_id, group_id FROM Message WHERE"
                   << " (author_id=? AND recipient_id=?) OR (recipient_id=? AND author_id=?)",
                into(a._id),
                into(a._text),
                into(a._created_at),
                into(a._author_id),
                into(a._recipient_id),
                into(a._group_id),
                use(requester_id),
                use(other_id),
                use(requester_id),
                use(other_id),
                range(0, 1); 

            while (!select.done())
            {
                if (select.execute())
                {
                    bool from_requester = a.author_id() == requester_id;
                    result.push_back(std::tuple<Message, bool>(a, from_requester));
                }
            }
            return result;
        }
        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << '\n' << e.message() << std::endl;
            
            throw;
        }
    }

    void Message::save_to_mysql(bool to_group)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            if (to_group) insert << "INSERT INTO Message (text,author_id,group_id) VALUES(?, ?, ?)",
                use(_text),
                use(_author_id),
                use(_group_id);
            else insert << "INSERT INTO Message (text,author_id,recipient_id) VALUES(?, ?, ?)",
                use(_text),
                use(_author_id),
                use(_recipient_id);
            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID(), created_at FROM Message WHERE id=LAST_INSERT_ID()",
                into(_id),
                into(_created_at),
                range(0, 1); 

            if (!select.done())
            {
                select.execute();
            }
            std::cout << "inserted:" << _id << std::endl;
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection:" << e.what() << std::endl;
            throw;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {
            std::cout << "statement:" << e.what() << std::endl;
            std::cout << e.message() << std::endl;
            throw;
        }
    }

    long Message::get_id() const
    {
        return _id;
    }

    const std::string &Message::get_text() const
    {
        return _text;
    }

    const Poco::DateTime &Message::get_created_at() const
    {
        return _created_at;
    }

    long Message::get_author_id() const
    {
        return _author_id;
    }

    long Message::get_recipient_id() const
    {
        return _recipient_id;
    }

    long Message::get_group_id() const
    {
        return _group_id;
    }

    long &Message::id()
    {
        return _id;
    }

    std::string &Message::text()
    {
        return _text;
    }

    Poco::DateTime &Message::created_at()
    {
        return _created_at;
    }

    long &Message::author_id()
    {
        return _author_id;
    }

    long &Message::recipient_id()
    {
        return _recipient_id;
    }

    long &Message::group_id()
    {
        return _group_id;
    }
}
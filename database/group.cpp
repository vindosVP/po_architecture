#include "message.h"
#include "user.h"
#include "database.h"
#include "group.h"
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

    void Group::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement create_stmt(session);
                create_stmt << "CREATE TABLE IF NOT EXISTS `Group` (`id` INT NOT NULL AUTO_INCREMENT,"
                            << "`title` VARCHAR(256) NULL,"
                            << "`admin_id` INT NOT NULL,"
                            << "PRIMARY KEY (`id`),KEY `a_id` (`admin_id`));"
                            << hint,     
                        now;

                Statement another_create_stmt(session);
                another_create_stmt << "CREATE TABLE IF NOT EXISTS `GroupMembership` ("
                                    << "`group_id` INT NOT NULL,"
                                    << "`user_id` INT NOT NULL,"
                                    << "PRIMARY KEY (`group_id`, `user_id`));"
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

    Poco::JSON::Object::Ptr Group::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("title", _title);
        root->set("admin_id", _admin_id);
        return root;
    }

    Group Group::fromJSON(const std::string &str)
    {
        Group group;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        group.id() = object->getValue<long>("id");
        group.title() = object->getValue<std::string>("title");
        group.admin_id() = object->getValue<long>("admin_id");

        return group;
    }

    std::optional<Group> Group::get_by_id(long id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            Group a;
            select << "SELECT id, title, admin_id FROM `Group` WHERE id=?",
                into(a._id),
                into(a._title),
                into(a._admin_id),
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
            std::cout << e.message() << std::endl;
        }
        return {};
    }

    std::vector<std::tuple<Message, bool>> Group::list_messages(long requester_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Statement select(session);
            std::vector<std::tuple<Message, bool>> result;
            Message a;
            select << "SELECT id, text, created_at, author_id, recipient_id, group_id FROM Message WHERE"
                   << " group_id=?",
                into(a.id()),
                into(a.text()),
                into(a.created_at()),
                into(a.author_id()),
                into(a.recipient_id()),
                into(a.group_id()),
                use(_id),
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

    void Group::save_to_mysql()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO `Group` (title,admin_id) VALUES(?, ?)",
                use(_title),
                use(_admin_id);

            insert.execute();

            Poco::Data::Statement select(session);
            select << "SELECT LAST_INSERT_ID()",
                into(_id),
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
            throw;
        }
    }

    void Group::add_user(long user_id)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement insert(session);

            insert << "INSERT INTO GroupMembership (group_id, user_id) VALUES(?, ?)",
                use(_id),
                use(user_id);
            insert.execute();
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

    long Group::get_id() const
    {
        return _id;
    }

    const std::string &Group::get_title() const
    {
        return _title;
    }

    long Group::get_admin_id() const
    {
        return _admin_id;
    }

    long &Group::id()
    {
        return _id;
    }

    std::string &Group::title()
    {
        return _title;
    }

    long &Group::admin_id()
    {
        return _admin_id;
    }
}
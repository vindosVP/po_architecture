#include "user.h"
#include "database.h"
#include "cache.h"
#include "../config/config.h"
#include "../helper.h"

#include <Poco/Data/MySQL/Connector.h>
#include <Poco/Data/MySQL/MySQLException.h>
#include <Poco/Data/SessionFactory.h>
#include <Poco/Data/RecordSet.h>
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>
#include <cppkafka/cppkafka.h>

#include <sstream>
#include <exception>

using namespace Poco::Data::Keywords;
using Poco::Data::Session;
using Poco::Data::Statement;

namespace database
{

    void User::init()
    {
        try
        {

            Poco::Data::Session session = database::Database::get().create_session();
            std::cout << "[DEBUG] start creating User table on shards" << std::endl;
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement create_stmt(session);
                std::cout << "[DEBUG] hint: " << hint << std::endl;
                create_stmt << "CREATE TABLE IF NOT EXISTS `User` (`id` INT NOT NULL,"
                            << "`first_name` VARCHAR(256) NOT NULL,"
                            << "`last_name` VARCHAR(256) NOT NULL,"
                            << "`login` VARCHAR(256) NOT NULL,"
                            << "`password` VARCHAR(256) NOT NULL,"
                            << "`email` VARCHAR(256) NULL,"
                            << "`title` VARCHAR(1024) NULL,"
                            << "UNIQUE(`email`),UNIQUE(`login`),"
                            << "PRIMARY KEY (`id`),KEY `fn` (`first_name`),KEY `ln` (`last_name`));"
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

    Poco::JSON::Object::Ptr User::toJSON() const
    {
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();

        root->set("id", _id);
        root->set("first_name", _first_name);
        root->set("last_name", _last_name);
        root->set("email", _email);
        root->set("title", _title);
        root->set("login", _login);
        root->set("password", _password);

        return root;
    }

    User User::fromJSON(const std::string &str)
    {
        User user;
        Poco::JSON::Parser parser;
        Poco::Dynamic::Var result = parser.parse(str);
        Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();

        user.id() = object->getValue<long>("id");
        user.first_name() = object->getValue<std::string>("first_name");
        user.last_name() = object->getValue<std::string>("last_name");
        user.email() = object->getValue<std::string>("email");
        user.title() = object->getValue<std::string>("title");
        user.login() = object->getValue<std::string>("login");
        user.password() = object->getValue<std::string>("password");

        return user;
    }

    std::optional<long> User::auth(std::string &login, std::string &password)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            Poco::Data::Statement select(session);
            long id;
            User user;
            user.login() = login;
            user.password() = password;
            select << "SELECT id FROM `User` where login=? and password=?;" + user.get_sharding_hint(),
                into(id),
                use(login),
                use(password),
                range(0, 1); 

            select.execute();
            Poco::Data::RecordSet rs(select);
            if (rs.moveFirst()) return id;
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
    std::optional<User> User::read_by_id(long id, bool use_cache)
    {
        if (use_cache) {
            try {
                std::string result;
                if (database::Cache::get().get(id, result)) {
                    std::cout << "[INFO] Cache hit for id=" << id << std::endl;  
                    return fromJSON(result);
                }
                else std::cout << "[INFO] No cache hit for id=" << id << std::endl;    
            }
            catch (std::exception &err) {
                std::cout << "Error: " << err.what() << std::endl;
            }
        }

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            User a;
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Poco::Data::Statement select(session);
                select << "SELECT id, first_name, last_name, email, title, login, password FROM User where id=?" << hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._title),
                    into(a._login),
                    into(a._password),
                    use(id),
                    range(0, 1); 

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()) {
                    if (use_cache) {
                        std::stringstream ss;
                        Poco::JSON::Stringifier::stringify(a.toJSON(), ss);
                        std::string message = ss.str();
                        database::Cache::get().put(a._id, message);
                        database::Cache::get().put(a._login, message);
                    }
                    return a;
                }
            }
        }

        catch (Poco::Data::MySQL::ConnectionException &e)
        {
            std::cout << "connection: " << e.what() << std::endl;
            std::cout << "details: " << e.displayText() << std::endl;
        }
        catch (Poco::Data::MySQL::StatementException &e)
        {

            std::cout << "statement: " << e.what() << std::endl;
            std::cout << "details: " << e.displayText() << std::endl;
            
        }
        return {};
    }

    std::optional<User> User::read_by_login(std::string &login, bool use_cache)
    {
        if (use_cache) {
            try {
                std::string result;
                if (database::Cache::get().get(login, result)) {
                    std::cout << "[INFO] Cache hit for login=" << login <<std::endl;
                    return fromJSON(result);
                }
                else std::cout << "[INFO] No cache hit for login=" << login <<std::endl;
            }
            catch (std::exception &err) {
                std::cout << "Error: " << err.what() << std::endl;
            }
        }

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            User a;
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Poco::Data::Statement select(session);
                select << "SELECT id, first_name, last_name, email, title,login,password FROM User where login=?" + hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._title),
                    into(a._login),
                    into(a._password),
                    use(login),
                    range(0, 1); 

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()){
                    if (use_cache) {
                        std::stringstream ss;
                        Poco::JSON::Stringifier::stringify(a.toJSON(), ss);
                        std::string message = ss.str();
                        database::Cache::get().put(a._id, message);
                        database::Cache::get().put(a._login, message);
                    }
                    return a;
                }
            }
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

    std::string User::get_sharding_hint() {
        long hash = get_hash(_login + _password);
        return database::Database::get_sharding_hint(hash);
    }

    std::vector<User> User::read_all()
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<User> result;
            User a;
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement select(session);
                select << "SELECT id, first_name, last_name, email, title, login, password FROM User" + hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._title),
                    into(a._login),
                    into(a._password),
                    range(0, 1); 

                while (!select.done())
                {
                    if (select.execute())
                        result.push_back(a);
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

            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    std::vector<User> User::search(std::string first_name, std::string last_name)
    {
        try
        {
            Poco::Data::Session session = database::Database::get().create_session();
            std::vector<User> result;
            User a;
            first_name += "%";
            last_name += "%";
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                Statement select(session);
                select << "SELECT id, first_name, last_name, email, title, login, password FROM User where first_name LIKE ? and last_name LIKE ?" + hint,
                    into(a._id),
                    into(a._first_name),
                    into(a._last_name),
                    into(a._email),
                    into(a._title),
                    into(a._login),
                    into(a._password),
                    use(first_name),
                    use(last_name),
                    range(0, 1); 

                while (!select.done())
                {
                    if (select.execute())
                        result.push_back(a);
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
            std::cout << "statement:" << e.what() << std::endl;
            throw;
        }
    }

    void User::save_to_mysql(bool use_cache)
    {

        try
        {
            Poco::Data::Session session = database::Database::get().create_session();

            _id = 0; 
            for (const auto& hint : database::Database::get_all_sharding_hints()) {
                long tmp_id = 0;
                Statement select(session);
                select << "SELECT MAX(`id`) from `User`" + hint,
                    into(tmp_id),
                    range(0, 1); 

                select.execute();
                Poco::Data::RecordSet rs(select);
                if (rs.moveFirst()) {
                    _id = std::max(_id, tmp_id);
                    std::cout << "[DEBUG] Max id on shard " << hint << ": " << tmp_id << std::endl;
                }
            }
            _id += 1;

            Poco::Data::Statement insert(session);

            insert << "INSERT INTO User (id,first_name,last_name,email,title,login,password) VALUES(?, ?, ?, ?, ?, ?, ?)" + get_sharding_hint(),
                use(_id),
                use(_first_name),
                use(_last_name),
                use(_email),
                use(_title),
                use(_login),
                use(_password);

            insert.execute();
            if (use_cache) {
                std::stringstream ss;
                Poco::JSON::Stringifier::stringify(toJSON(), ss);
                std::string message = ss.str();
                database::Cache::get().put(_id, message);
                database::Cache::get().put(_login, message);
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
            std::cout << "details: " << e.message() << std::endl;
            throw;
        }
    }

    void User::send_to_queue() {
        static cppkafka::Configuration config = {
            {"metadata.broker.list", Config::get().get_queue_host()},
            {"acks", "all"}
        };

        static cppkafka::Producer producer(config);
        static std::mutex mtx;
        static int message_key{0};
        using Hdr = cppkafka::MessageBuilder::HeaderType;

        std::lock_guard<std::mutex> lock(mtx);
        std::stringstream ss;
        Poco::JSON::Stringifier::stringify(toJSON(), ss);
        std::string message = ss.str();
        bool not_sent = true;

        cppkafka::MessageBuilder builder(Config::get().get_queue_topic());
        std::string mk = std::to_string(++message_key);
        builder.key(mk);                                       
        builder.header(Hdr{"producer_type", "author writer"}); 
        builder.payload(message);                              

        while (not_sent) {
            try {
                producer.produce(builder);
                not_sent = false;
            }
                catch (...) {
            }
        }
    }


    const std::string &User::get_login() const
    {
        return _login;
    }

    const std::string &User::get_password() const
    {
        return _password;
    }

    std::string &User::login()
    {
        return _login;
    }

    std::string &User::password()
    {
        return _password;
    }

    long User::get_id() const
    {
        return _id;
    }

    const std::string &User::get_first_name() const
    {
        return _first_name;
    }

    const std::string &User::get_last_name() const
    {
        return _last_name;
    }

    const std::string &User::get_email() const
    {
        return _email;
    }

    const std::string &User::get_title() const
    {
        return _title;
    }

    long &User::id()
    {
        return _id;
    }

    std::string &User::first_name()
    {
        return _first_name;
    }

    std::string &User::last_name()
    {
        return _last_name;
    }

    std::string &User::email()
    {
        return _email;
    }

    std::string &User::title()
    {
        return _title;
    }
}
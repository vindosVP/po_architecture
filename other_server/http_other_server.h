#ifndef HTTPOTHERWEBSERVER_H
#define HTTPOTHERWEBSERVER_H



#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/JSON/Object.h"
#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerParams.h"
#include "Poco/Net/HTMLForm.h"
#include "Poco/Net/ServerSocket.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/DateTimeFormat.h"
#include "Poco/Exception.h"
#include "Poco/ThreadPool.h"
#include "Poco/Util/ServerApplication.h"
#include "Poco/Util/Option.h"
#include "Poco/Util/OptionSet.h"
#include "Poco/Util/HelpFormatter.h"
#include "Poco/StreamCopier.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/SharedPtr.h"
#include "Poco/Net/SSLManager.h"
#include "Poco/Net/KeyConsoleHandler.h"
#include "Poco/Net/ConsoleCertificateHandler.h"
#include "Poco/Net/AcceptCertificateHandler.h"
#include "Poco/Net/HTTPSClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <Poco/JSON/Parser.h>
#include <Poco/Dynamic/Var.h>

#include <Poco/URIStreamFactory.h>
#include <Poco/URIStreamOpener.h>
#include <Poco/Net/HTTPSStreamFactory.h>
#include <Poco/Net/HTTPStreamFactory.h>
#include <Poco/Base64Encoder.h>
#include <Poco/Base64Decoder.h>

using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTTPRequestHandler;
using Poco::Net::HTTPRequestHandlerFactory;
using Poco::Net::HTTPServer;
using Poco::Net::HTTPServerParams;
using Poco::Net::HTTPServerRequest;
using Poco::Net::HTTPServerResponse;
using Poco::Net::ServerSocket;
using Poco::Util::Application;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;
using Poco::DateTimeFormat;
using Poco::DateTimeFormatter;
using Poco::ThreadPool;
using Poco::Timestamp;
using Poco::Net::HTMLForm;
using Poco::Net::NameValueCollection;
using Poco::Util::HelpFormatter;
using Poco::Util::Option;
using Poco::Util::OptionCallback;
using Poco::Util::OptionSet;
using Poco::Util::ServerApplication;


#include <iostream>
#include <string>
#include <fstream>
#include <optional>

#include "../helper.h"
#include "../database/message.h"
#include "../database/group.h"


class OtherHandler : public HTTPRequestHandler
{

public:
    OtherHandler(const std::string &format) : _format(format)
    {
    }

    std::optional<std::string> do_get(const std::string &url, const std::string &login, const std::string &password)
    {
        std::string string_result;
        try
        {
            std::string token = login + ":" + password;
            std::ostringstream os;
            Poco::Base64Encoder b64in(os);
            b64in << token;
            b64in.close();
            std::string identity = "Basic " + os.str();

            Poco::URI uri(url);
            Poco::Net::HTTPClientSession s(uri.getHost(), uri.getPort());
            Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.toString());
            request.setVersion(Poco::Net::HTTPMessage::HTTP_1_1);
            request.setContentType("application/json");
            request.set("Authorization", identity);
            request.set("Accept", "application/json");
            request.setKeepAlive(true);

            s.sendRequest(request);

            Poco::Net::HTTPResponse response;
            std::istream &rs = s.receiveResponse(response);

            while (rs)
            {
                char c{};
                rs.read(&c, 1);
                if (rs)
                    string_result += c;
            }

            if (response.getStatus() != 200)
                return {};
        }
        catch (Poco::Exception &ex)
        {
            std::cout << "exception:" << ex.what() << std::endl;
            return std::optional<std::string>();
        }

        return string_result;
    }

    void handleRequest(HTTPServerRequest &request,
                       HTTPServerResponse &response)
    {
        HTMLForm form(request, request.stream());
        std::string scheme;
        std::string info;
        std::string login, password;

        request.getCredentials(scheme, info);
        if (scheme == "Basic")
        {
            get_identity(info, login, password);
            std::cout << "login:" << login << std::endl;
            std::cout << "password:" << password << std::endl;
            std::string host = "localhost";
            std::string url;

            if(std::getenv("SERVICE_HOST")!=nullptr) host = std::getenv("SERVICE_HOST");
            url = "http://" + host+":8080/auth";

            auto auth_response = do_get(url, login, password);
            if (auth_response) // do authentificate
            {
                long user_id;
                Poco::JSON::Parser parser;
                Poco::Dynamic::Var result = parser.parse(*auth_response);
                Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
                user_id = object->getValue<long>("id");

                if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET &&
                    hasSubstr(request.getURI(), "/list") &&
                    form.has("user_id"))
                {
                    long other_id = std::atol(form.get("user_id").c_str());
                    auto results = database::Message::list_chat(user_id, other_id);
                    Poco::JSON::Array arr;
                    for (auto s : results)
                    {
                        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                        root->set("message", std::get<0>(s).toJSON());
                        root->set("from_requester", std::get<1>(s));
                        arr.add(root);
                    }
                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(arr, ostr);

                    return;
                }

                else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
                         hasSubstr(request.getURI(), "/send") &&
                         form.has("text") && form.has("to"))
                {
                    database::Message message;
                    message.text() = form.get("text");
                    message.author_id() = user_id;
                    message.recipient_id() = std::atol(form.get("to").c_str());
                    message.save_to_mysql();

                    response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                    response.setChunkedTransferEncoding(true);
                    response.setContentType("application/json");
                    std::ostream &ostr = response.send();
                    Poco::JSON::Stringifier::stringify(message.toJSON(), ostr);

                    return;
                }

                else if (hasSubstr(request.getURI(), "/groups"))
                {
                    if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
                        hasSubstr(request.getURI(), "/send") &&
                        form.has("text") && form.has("group_id"))
                    {
                        database::Message message;
                        message.text() = form.get("text");
                        message.author_id() = user_id;
                        message.group_id() = std::atol(form.get("group_id").c_str());
                        message.save_to_mysql(true);

                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(message.toJSON(), ostr);

                        return;
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST &&
                             hasSubstr(request.getURI(), "/add_user") &&
                             form.has("user_id") && form.has("group_id"))
                    {
                        long group_id = std::atol(form.get("group_id").c_str());
                        auto group = database::Group::get_by_id(group_id);
                        if (group && group->admin_id() == user_id)
                        {
                            group->add_user(std::atol(form.get("user_id").c_str()));
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                            root->set("status", "OK");
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(root, ostr);
                            return;
                        }
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_POST && form.has("title"))
                    {
                        std::cout << "[DEBUG] trying to create group" << std::endl;
                        database::Group group;
                        group.admin_id() = user_id;
                        group.title() = form.get("title");
                        group.save_to_mysql();

                        response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                        response.setChunkedTransferEncoding(true);
                        response.setContentType("application/json");
                        std::ostream &ostr = response.send();
                        Poco::JSON::Stringifier::stringify(group.toJSON(), ostr);
                    }
                    else if (request.getMethod() == Poco::Net::HTTPRequest::HTTP_GET && form.has("group_id"))
                    {
                        long group_id = std::atol(form.get("group_id").c_str());
                        auto group = database::Group::get_by_id(group_id);
                        if (group)
                        {
                            auto results = group->list_messages(user_id);
                            Poco::JSON::Array arr;
                            for (auto s : results)
                            {
                                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                                root->set("message", std::get<0>(s).toJSON());
                                root->set("from_requester", std::get<1>(s));
                                arr.add(root);
                            }
                            response.setStatus(Poco::Net::HTTPResponse::HTTP_OK);
                            response.setChunkedTransferEncoding(true);
                            response.setContentType("application/json");
                            std::ostream &ostr = response.send();
                            Poco::JSON::Stringifier::stringify(arr, ostr);

                            return;
                        }
                    }
                }

                response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_BAD_REQUEST);
                response.setChunkedTransferEncoding(true);
                response.setContentType("application/json");
                Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
                root->set("detail", "bad url/data");
                std::ostream &ostr = response.send();
                Poco::JSON::Stringifier::stringify(root, ostr);
                return;
            }
            
        }

        response.setStatus(Poco::Net::HTTPResponse::HTTPStatus::HTTP_UNAUTHORIZED);
        response.setChunkedTransferEncoding(true);
        response.setContentType("application/json");
        Poco::JSON::Object::Ptr root = new Poco::JSON::Object();
        root->set("type", "/errors/unauthorized");
        root->set("title", "Unauthorized");
        root->set("status", "401");
        root->set("detail", "invalid login or password");
        root->set("instance", "/user");
        std::ostream &ostr = response.send();
        Poco::JSON::Stringifier::stringify(root, ostr);
    }

private:
    std::string _format;
};

class HTTPOtherRequestFactory : public HTTPRequestHandlerFactory
{
public:
    HTTPOtherRequestFactory(const std::string &format) : _format(format)
    {
    }

    HTTPRequestHandler *createRequestHandler([[maybe_unused]] const HTTPServerRequest &request)
    {
        std::cout << "request:" << request.getURI()<< std::endl;
        if (hasSubstr(request.getURI(), "/list") ||
            hasSubstr(request.getURI(), "/send") ||
            hasSubstr(request.getURI(), "/groups"))        
            return new OtherHandler(_format);
        return 0;
    }

private:
    std::string _format;
};

class HTTPOtherWebServer : public Poco::Util::ServerApplication
{
public:
    HTTPOtherWebServer() : _helpRequested(false)
    {
    }

    ~HTTPOtherWebServer()
    {
    }

protected:
    void initialize(Application &self)
    {
        loadConfiguration();
        ServerApplication::initialize(self);
    }

    void uninitialize()
    {
        ServerApplication::uninitialize();
    }

    int main([[maybe_unused]] const std::vector<std::string> &args)
    {
        if (!_helpRequested)
        {
            database::Group::init();
            database::Message::init();
            ServerSocket svs(Poco::Net::SocketAddress("0.0.0.0", 8081));
            HTTPServer srv(new HTTPOtherRequestFactory(DateTimeFormat::SORTABLE_FORMAT), svs, new HTTPServerParams);
            srv.start();
            waitForTerminationRequest();
            srv.stop();
        }
        return Application::EXIT_OK;
    }

private:
    bool _helpRequested;
};
#endif // !HTTPOTHERWEBSERVER

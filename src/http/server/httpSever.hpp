#pragma once
#include<string>
#include<ixwebsocket/IXHttpServer.h>
#include<functional>

struct HttpRequest
{
	std::string method;
	std::string version;
	std::string body;
	std::map<std::string, std::string> headers;
};

struct HttpResponse
{
	int32_t status = 200;
	std::string msg;
};

class HttpServer
{
public:
	HttpServer();
	virtual ~HttpServer();

public:
	void setPort(int32_t value);
	bool listen(int32_t port);

public:
	void on(std::string url, std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)> func);
	void setOnMsgFunc(std::function < std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)> func);

public:
	void close();

private:
	ix::HttpResponsePtr onMsg(ix::HttpRequestPtr request, std::shared_ptr<ix::ConnectionState> connectionState);

private:
	std::map<std::string, std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)>> mRouters;
	std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)> mOnMsgFunc;
	ix::HttpServer* mServer=nullptr;
};
#include "httpSever.hpp"
#include<spdlog/spdlog.h>

HttpServer::HttpServer()
{

}

HttpServer::~HttpServer()
{
	this->close();
}

bool HttpServer::listen(int32_t port)
{
	if (this->mServer)
	{
		return false;
	}

	this->mServer = new ix::HttpServer(port);
	this->mServer->setOnConnectionCallback(std::bind(&HttpServer::onMsg, this, std::placeholders::_1, std::placeholders::_2));

	auto res = this->mServer->listen();
	if (!res.first)
	{
		spdlog::error("{}", res.second);
		return false;
	}

	this->mServer->start();

	spdlog::info("httpServer 127.0.0.1:{} start", port);

	return true;
}

void HttpServer::on(std::string url, std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)> func)
{
	this->mRouters[url] = func;
}

void HttpServer::setOnMsgFunc(std::function<std::shared_ptr<HttpResponse>(std::shared_ptr<HttpRequest>)> func)
{
	this->mOnMsgFunc = func;
}

void HttpServer::close()
{
	if (this->mServer)
	{
		this->mServer->stop();
		delete this->mServer;
		this->mServer = nullptr;
	}
}

ix::HttpResponsePtr HttpServer::onMsg(ix::HttpRequestPtr request, std::shared_ptr<ix::ConnectionState> connectionState)
{
	auto httpRequest = std::make_shared<HttpRequest>();
	httpRequest->method = request->method;
	httpRequest->version = request->version;
	httpRequest->body = request->body;
	for (auto it = request->headers.begin(); it != request->headers.end(); ++it)
	{
		httpRequest->headers.insert({ it->first,it->second });
	}

	if (this->mOnMsgFunc)
	{
		auto httpResponse=this->mOnMsgFunc(httpRequest);
		return std::make_shared<ix::HttpResponse>(httpResponse->status, "OK", ix::HttpErrorCode::Ok, ix::WebSocketHttpHeaders(), httpResponse->msg);
	}

	auto iter = this->mRouters.find(request->uri);
	if (iter == this->mRouters.end())
	{
		return std::make_shared<ix::HttpResponse>(404, "OK", ix::HttpErrorCode::Ok, ix::WebSocketHttpHeaders(), std::string());
	}

	auto httpResponse = iter->second(httpRequest);

	return std::make_shared<ix::HttpResponse>(httpResponse->status, "OK", ix::HttpErrorCode::Ok, ix::WebSocketHttpHeaders(), httpResponse->msg);
}

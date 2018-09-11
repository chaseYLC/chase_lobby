#include "stdafx.h"
#include "watchDogHandler.h"

#include <thread>
#include <conio.h>
#include <net/test_interface.h>
#include "Environment.h"


std::string watchDogHandler::inputLine(const std::string &inputMsg, boost::asio::ip::tcp::socket &sock)
{
	sock;

	const int byteSize = 4;

	static const char *endline = "\r\n";

	if (inputMsg == "help"
		|| inputMsg == "h")
	{
		std::string helpString = endline;
		helpString += "<< Command List >>";
		helpString += endline;
		helpString += "status";
		helpString += endline;
		helpString += "init DB";
		helpString += endline;
		helpString += endline;
		helpString += "u can close socket by typeing \'Ctrl + C\' or \'Ctrl + Z\'";
		helpString += endline;
		helpString += endline;

		return helpString;
	}
	else if (inputMsg == "status")
	{
		//auto env = Environment::instance();
		//std::string statusString = endline;
		//statusString += endline;
		//statusString += "[command] : status ==> show server's status";
		//statusString += endline;
		//statusString += endline;

		//statusString += "listen TCP-Port : ";
		//statusString += env->m_strListenPort;
		//statusString += endline;

		//return statusString;
	}

	return inputMsg;
}

void watchDogHandler::closedConsole()
{
	LOGI << "closed test-console.";
}

void watchDogHandler::registCallback(std::shared_ptr<boost::asio::io_service> ios, unsigned short port)
{
	m_ios = ios;

	MLN::Net::TestIntarface::instance()->setWatchDogEventCallback(
		std::bind(&watchDogHandler::inputLine
			, this
			, std::placeholders::_1
			, std::placeholders::_2
		)
		, std::bind(&watchDogHandler::closedConsole
			, this
		)
		, port);
}

#include "stdafx.h"
#include "keyEventHandler.h"

#include <Winuser.h>
#include <thread>
#include <conio.h>
#include <net/test_interface.h>
#include <net/logger.h>

#pragma warning( disable:4996)


void keyEventHandler::processKey(const unsigned int vKey)
{
	switch (vKey)
	{
	case VK_ESCAPE:
	{
					  LOGI << "pressed ESC key ...";

					  auto ios = m_ios.lock();
					  if (ios)
					  {
						  if (false == ios->stopped()){
							  LOGI << "terminating process...";
							  ios->stop();
						  }
					  }
	}
		break;
	case VK_F1:
	{
	}
		break;
	case VK_F2:
	{
	}
	break;

	}//switch (key)
}

void keyEventHandler::registCallback(std::shared_ptr<boost::asio::io_service> ios)
{
	m_ios = ios;

	MLN::Net::TestIntarface::instance()->setMyKeyEventCallback(
		std::bind(&keyEventHandler::processKey
		, this
		, std::placeholders::_1));
}

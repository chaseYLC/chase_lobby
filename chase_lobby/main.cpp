#include "stdafx.h"

#include <boost/filesystem.hpp>

#include <direct.h> // for _tchdir

#include <net/logger.h>
#include <net/test_interface.h>
#include <net/ExceptionCrashDump.h>
#include <net/NetServiceAcceptor.h>
#include <net/netServiceConnector.h>
#include <net/mssqlConsumer.h>
#include "connectorIndex.h"
#include "packetParserJson.h"
#include <net/packetEncType.h>

#include <thread>

#include "Environment.h"
#include "brokerAcceptorReceiver.h"
#include "LobbyAcceptorReceiver.h"
#include "brokerConnector.h"
#include "userBasis.h"
#include "lobbyUserManager.h"
#include "keyEventHandler.h"
#include "watchDogHandler.h"
#include "dataTable.h"



void initTestInterface()
{
	auto env = Environment::instance();
	watchDogHandler::instance()->registCallback(
		shared_ios
		, env->m_testInterfacePort);
	keyEventHandler::instance()->registCallback(shared_ios);

	MLN::Net::TestIntarface::instance()->start(
		shared_ios		// use nullptr if not using boost-asio
		, MLN::Net::TestIntarface::FUNC::wait_keyboardEvent
		, MLN::Net::TestIntarface::FUNC::watchdog
		);
}

void post_ios_start() 
{
	LOGI << "started";
}

void app_shutdown(const std::string &msg)
{
}

template <typename RECEIVER_TYPE>
std::shared_ptr<MLN::Net::NetServiceAcceptor> registAcceptorReceiver(const size_t acceptorIDX, Environment::MLN_SvrSetting &setting)
{
	if (false == setting.activate) {
		return std::shared_ptr<MLN::Net::NetServiceAcceptor>();
	}

	MLN::Net::EventReceiverAcceptorRegister<RECEIVER_TYPE> acceptorHandler;

	MLN::Net::NetService::ServiceParams serviceInitParams(
		*shared_ios.get()
		, acceptorHandler
		, JSON_PACKET_PARSING_SUPPORT::packetParser_notUseEncTest
		, JSON_PACKET_PARSING_SUPPORT::getMsgManipulatorTest()
		, setting.tick_time_ms
		, setting.keep_alive_ms
	);

	MLN::Net::NetService::AcceptorUserParams
		acceptorParams(
			""	// addr. null string is localhost.
			, setting.listenPort
			, (setting.workers != 0) ? (setting.workers) : (boost::thread::hardware_concurrency() * 2)
		);

	auto acceptor = MLN::Net::NetService::createAcceptor(
		acceptorIDX
		, serviceInitParams
		, acceptorParams
	);

	// Encryption 핸들러 등록
	chase_lobbysvr::UserBasis::m_encType = MLN::Net::EncType::GREETING;

	return acceptor;
};

template <typename RECEIVER_TYPE>
std::shared_ptr<MLN::Net::NetServiceConnector> registConnectorReceiver(const size_t connectorIDX, Environment::MLN_SvrSetting &setting)
{
	if (false == setting.activate) {
		return std::shared_ptr<MLN::Net::NetServiceConnector>();
	}
	MLN::Net::EventReceiverConnectorRegister<RECEIVER_TYPE> connectorHandler;

	MLN::Net::NetService::ServiceParams connectorSvcParams(
		*shared_ios.get()
		, connectorHandler
		, JSON_PACKET_PARSING_SUPPORT::packetParser_brokerTest
		, JSON_PACKET_PARSING_SUPPORT::getMsgManipulatorTest()
		, 1000
		, 0//, 1000 * 15		// keepAliveTimeMs
	);

	MLN::Net::NetService::ConnectorUserParams
		connectorParams(
			setting.addr
			, setting.listenPort
		);

	auto connector = MLN::Net::NetService::createConnector(
		connectorIDX
		, connectorSvcParams
		, connectorParams
	);
	BrokerConnector::tryConnect();

	return connector;
};


bool ioServiceThread()
{
	auto env = Environment::instance();

	auto brokerAcceptor = registAcceptorReceiver<BrokerAcceptorReceiver>(ACCPETOR_IDX::BROKER, env->m_svrBroker);
	auto lobbyAcceptor = registAcceptorReceiver<LobbyAcceptorReceiver>(ACCPETOR_IDX::LOBBY, env->m_svrLobby);

	/*if (lobbyAcceptor) {
		lobbyAcceptor->getMsgProcedure()->setCipher<UserManager>(
			MLN::Net::EncType::GREETING
			, &LobbyUserManager::createUser
			, &LobbyUserManager::decryptMessage
			, LobbyUserManager::instance());
	}*/
	if (lobbyAcceptor) {
		auto brokerConnector = registConnectorReceiver<BrokerConnector>(CONNECTOR_INDEX::BROKER_CONNECTOR, env->m_svrBroker);
	}

	// IO서비스 시작. runService() 에서 블러킹 됩니다.
	MLN::Net::NetService::runService(post_ios_start);

	return true;
}

bool initLogService(const std::string &Path)
{
	// 로그서비스 초기화
	std::cout << std::endl << "init Log Service" << std::endl;

	if (false == MLN::Log::Logger::instance()->init(Path)) {
		std::cout << "failed init Log Service.  " << std::endl;
		return false;
	}

	return true;
}


void setModuleDirToWorkingDir()
{
	TCHAR szPathBuffer[MAX_PATH];

	HMODULE hModule = GetModuleHandle(NULL);
	GetModuleFileName(hModule, szPathBuffer, MAX_PATH);

	int nIndex = 0;
	int nCnt = static_cast<int>(_tcslen(szPathBuffer));
	for (; nCnt > 0; nCnt--)
	{
		if (szPathBuffer[nCnt] == _T('\\'))
		{
			nIndex = nCnt;
			break;
		}
	}
	szPathBuffer[nIndex] = _T('\0');

	_tchdir(szPathBuffer);
}

void setRandom()
{
	auto curTime = std::chrono::system_clock::now();
	auto dur = curTime.time_since_epoch();
	auto mills = std::chrono::duration_cast<std::chrono::microseconds>(dur).count();

	g_rand = new std::mt19937((unsigned int)mills);
}

int _tmain(int argc, _TCHAR* argv[])
{
	setModuleDirToWorkingDir();

	namespace fs = boost::filesystem;
	fs::path full_path = fs::system_complete("chase_lobby.xml");
	std::string confPath = full_path.string();

	// set Random.
	setRandom();

	// init Logger
	if (false == initLogService(confPath)) {
		std::cout << "failed initLogService" << std::endl;
		return false;
	}

	MLN::CCrashHandler::Init(confPath.c_str());

	// init Configurations
	auto env = Environment::instance();
	assert(env && "failed Environment");
	if (false == env->Initialize(confPath.c_str())) {
		LOGE << "Config Error!";
		std::cout << "failed Environment Init" << std::endl;
		return false;
	}
	
	SetConsoleTitleA(env->m_title.c_str());

	// load data table.
	DataTable::instance()->load();

	// 
	MLN::MSSQL::DBConsumer::Open("Sudden64", "sa", "20934159", app_shutdown);

	// Test Interface.
	initTestInterface();

	return ioServiceThread();
}
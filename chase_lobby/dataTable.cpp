#include "stdafx.h"
#include "dataTable.h"
#include <fstream>
#include <boost/tokenizer.hpp>

bool DataTable::load()
{
	try {
		load_constant();
	}
	catch (std::exception &msg ) {
		LOGE << "failed file : " << msg.what();
		return false;
	}
	return true;
}


void DataTable::load_constant()
{
	std::string data("./dataTable/constant.csv");

	struct ColumnNo {
		enum {
			ID,
			Value,
		};
	};

	std::ifstream in(data.c_str());
	if (false == in.is_open()) {
		throw std::exception("faile open. Constant");
	}

	typedef boost::tokenizer< boost::escaped_list_separator<char> > Tokenizer;

	std::vector< std::string > vec;
	std::string line;

	size_t lineCnt = 0;
	while (std::getline(in, line))
	{
		if (2 >= (++lineCnt)) {
			continue;
		}

		Tokenizer tok(line);
		vec.assign(tok.begin(), tok.end());

		std::string id = vec.at(ColumnNo::ID);
		if (id == "MAX_VALUE1") {
			m_constant.MAX_VALUE1 = std::stoi(vec.at(ColumnNo::Value));
		}
		else if (id == "MAX_VALUE2") {
			m_constant.MAX_VALUE1 = std::stoi(vec.at(ColumnNo::Value));
		}
		else if (id == "MAX_VALUE3") {
			m_constant.MAX_VALUE1 = std::stoi(vec.at(ColumnNo::Value));
		}
	}//while (std::getline(in, line))
}


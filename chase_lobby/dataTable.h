#pragma once

#include <net/Singleton.h>

typedef struct tagMLNConstant
{
	int MAX_VALUE1 = 0;
	int MAX_VALUE2 = 0;
	int MAX_VALUE3 = 0;
}MLN_CONSTANT;

class DataTable
	: public MLN::Net::SingletonLight<DataTable>
{
public:
	bool load();

private:
	void load_constant();

public:
	MLN_CONSTANT m_constant;
};

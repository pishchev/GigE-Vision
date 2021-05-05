#pragma once
#include "GenTL.h"
#include "LibLoader.hpp"
#include "Logger.hpp"

class TransportLayer
{
public:

	void Open()
	{
		auto err = TLOpen(&hTL);
		elog(err, "TL_Handler::Open");
	}

	void UpdateInterfaceList()
	{
		bool8_t pbChanged = false;
		auto err = TLUpdateInterfaceList(hTL, &pbChanged, GENTL_INFINITE);
		elog(err, "TL_Handler::UpdateInterfaceList");
	}

	uint32_t GetNumInterfaces()
	{
		auto err = TLGetNumInterfaces(hTL, &num_interfaces);
		elog(err, "TL_Handler::GetNumInterfaces");
		return num_interfaces;
	}

	void ShowInterfaces()
	{
		GetNumInterfaces();
		std::cout << "Interfaces: " << std::endl;
		for (uint32_t i = 0; i < num_interfaces; i++)
		{
			Buffer sID(20);
			auto err = TLGetInterfaceID(hTL, i, (char*)sID.buffer, &sID.size);
			std::cout << i << ") ";
			print_as<char>(sID);
			elog(err, "TL_Handler::ShowInterfaces");
		}
	}

	GenTL::IF_HANDLE GetInterface(uint32_t num)
	{
		Buffer sID(20);
		auto err = TLGetInterfaceID(hTL, num, (char*)sID.buffer, &sID.size);
		elog(err, "TL_Handler::GetInterface");
		GenTL::IF_HANDLE hIF = nullptr;
		err = TLOpenInterface(hTL, (char*)sID.buffer, &hIF);
		elog(err, "TL_Handler::GetInterface");
		return hIF;
	}

	GenTL::TL_HANDLE GetTL()
	{
		return hTL;
	}


private:
	GenTL::TL_HANDLE hTL = nullptr;
	uint32_t num_interfaces = -1;
};
#pragma once
#include "GenTL.h"
#include "LibLoader.hpp"
#include "Logger.hpp"

class IF_Handler
{
public:
	IF_Handler(GenTL::IF_HANDLE hIF) : hIF(hIF) {}

	void UpdateDeviceList()
	{
		bool8_t pbChanged = false;
		auto err = IFUpdateDeviceList(hIF, &pbChanged, GENTL_INFINITE);
		elog(err, "IF_Handler::UpdateInterfaceList");
	}

	uint32_t GetNumDevices()
	{
		auto err = IFGetNumDevices(hIF, &num_devices);
		elog(err, "IF_Handler::GetNumDevices");

		return num_devices;
	}

	void ShowDevices()
	{
		GetNumDevices();
		std::cout << "Devices: " << std::endl;
		for (uint32_t i = 0; i < num_devices; i++)
		{
			Buffer dID(20);
			auto err = IFGetDeviceID(hIF, i, (char*)dID.buffer, &dID.size);
			std::cout << i << ") ";
			print_as<char>(dID);
			elog(err, "IF_Handler::ShowDevices");
		}
	}

	GenTL::DEV_HANDLE GetDevice(uint32_t num)
	{
		Buffer dID(20);
		auto err = IFGetDeviceID(hIF, num, (char*)dID.buffer, &dID.size);
		elog(err, "IF_Handler::GetDevice");
		GenTL::DEV_HANDLE hDevice = nullptr;
		err = IFOpenDevice(hIF, (char*)dID.buffer, GenTL::DEVICE_ACCESS_READONLY, &hDevice);
		elog(err, "IF_Handler::GetDevice");
		return hDevice;
	}

	GenTL::IF_HANDLE GetInterface()
	{
		return hIF;
	}


private:
	GenTL::IF_HANDLE hIF = nullptr;
	uint32_t num_devices = -1;
};
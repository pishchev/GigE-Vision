#pragma once
#include "GenTL.h"
#include "LibLoader.hpp"
#include "Logger.hpp"

class DEV_Handler
{
public:

	DEV_Handler(GenTL::DEV_HANDLE hDevice) : hDevice(hDevice) {}


	uint32_t GetNumStreams()
	{
		auto err = DevGetNumDataStreams(hDevice, &num_streams);
		elog(err, "DEV_Handler::GetNumStreams");
		return num_streams;
	}

	void ShowStreams()
	{
		GetNumStreams();
		std::cout << "Streams: " << std::endl;
		for (uint32_t i = 0; i < num_streams; i++)
		{
			Buffer dDSID(20);
			auto err = DevGetDataStreamID(hDevice, i, (char*)dDSID.buffer, &dDSID.size);
			std::cout << i << ") ";
			print_as<char>(dDSID);
			elog(err, "DEV_Handler::ShowStreams");
		}
	}

	GenTL::DS_HANDLE GetStream(uint32_t num)
	{
		Buffer dDSID(20);
		auto err = DevGetDataStreamID(hDevice, num, (char*)dDSID.buffer, &dDSID.size);
		elog(err, "DEV_Handler::GetStream");
		GenTL::DS_HANDLE hDS = nullptr;
		err = DevOpenDataStream(hDevice, (char*)dDSID.buffer, &hDS);
		elog(err, "DEV_Handler::GetStream");
		return hDS;
	}

	GenTL::PORT_HANDLE GetPort()
	{
		GenTL::PORT_HANDLE port = nullptr;

		auto err = DevGetPort(hDevice, &port);
		elog(err, "DEV_Handler::GetPort");
		return port;
	}

	GenTL::DEV_HANDLE GetDevice()
	{
		return hDevice;
	}


private:
	GenTL::DEV_HANDLE hDevice = nullptr;
	uint32_t num_streams = -1;
};
#include <iostream>
#include <assert.h>

#include <windows.h> 
#include <stdio.h> 

#include <string>

#include "Buffer.hpp"
#include "Logger.hpp"
#include "LibLoader.hpp"

#include "GenTL.h"

// TLSimu.cti
// bgapi2_gige.cti

class TL_Handler
{
public:

	void Open()
	{
		auto err = TLOpen(&hTL);

		if (log_flag)
			log(err, "TL_Handler::Open");
	}

	void UpdateInterfaceList()
	{
		bool8_t pbChanged = false;
		auto err = TLUpdateInterfaceList(hTL, &pbChanged, GENTL_INFINITE);

		if (log_flag)
			log(err, "TL_Handler::UpdateInterfaceList");
	}

	uint32_t GetNumInterfaces()
	{
		auto err = TLGetNumInterfaces(hTL, &num_interfaces);

		if (log_flag)
			log(err, "TL_Handler::GetNumInterfaces");

		return num_interfaces;
	}

	void ShowInterfaces()
	{
		GetNumInterfaces();

		std::cout << "Inferfaces: " << std::endl;

		for (uint32_t i = 0; i < num_interfaces; i++)
		{
			Buffer sID(20);
			auto err = TLGetInterfaceID(hTL, i, (char*)sID.buffer, &sID.size);

			std::cout << i << ") ";
			print_as<char>(sID);

			if (log_flag)
				log(err, "TL_Handler::ShowInterfaces");
		}
	}

	GenTL::IF_HANDLE GetInterface(uint32_t num)
	{
		if (num > num_interfaces)
			return nullptr;

		Buffer sID(20);
		TLGetInterfaceID(hTL, num, (char*)sID.buffer, &sID.size);

		GenTL::IF_HANDLE hIF = nullptr;

		auto err = TLOpenInterface(hTL, (char*)sID.buffer, &hIF);

		if (log_flag)
			log(err, "TL_Handler::GetInterface");
	
		return hIF;
	}


private:
	bool log_flag = true;

	GenTL::TL_HANDLE hTL = nullptr;

	uint32_t num_interfaces = -1;
};

int main()
{
	Init_Lib("TLSimu.cti");	
	

	GCInitLib();

	bool8_t pbChanged = false;

	{
		Buffer info_buffer(10);
		int a = 1;
		log(GCGetInfo(1, &a, info_buffer.buffer, &info_buffer.size),
			"GCGetInfo");
		print_as<char>(info_buffer);
	}
	

	//GenTL::TL_HANDLE hTL = nullptr;

	//log(TLOpen(&hTL),
	//	"TLOpen");


	//log(TLUpdateInterfaceList(hTL , &pbChanged, GENTL_INFINITE),
	//	"TLUpdateInterfaceList") ;


	//uint32_t num_interface = -1;
	//log(TLGetNumInterfaces(hTL, &num_interface),
	//	"TLGetNumInterfaces");
	//std::cout << "Number of interfaces: " << num_interface << std::endl;

	////проверка на существование интерфейсов

	//Buffer sID(20);
	//log(TLGetInterfaceID(hTL, 0, (char*)sID.buffer, &sID.size),
	//	"TLGetInterfaceID");
	//print_as<char>(sID);


	//GenTL::IF_HANDLE hIF;

	//log(TLOpenInterface(hTL , (char*)sID.buffer , &hIF),
	//	"TLOpenInterface");

	TL_Handler tl_handler;
	tl_handler.Open();
	tl_handler.UpdateInterfaceList();
	tl_handler.ShowInterfaces();

	GenTL::IF_HANDLE hIF = tl_handler.GetInterface(0);

	log(IFUpdateDeviceList(hIF, &pbChanged, GENTL_INFINITE),
		"IFUpdateDeviceList");


	uint32_t num_device = -1;
	log(IFGetNumDevices(hIF, &num_device),
		"IFGetNumDevices");
	std::cout << "Number of diveces: " << num_device << std::endl;

	//проверка на существование девайсов

	Buffer dID(20);
	log(IFGetDeviceID(hIF, 1, (char*)dID.buffer, &dID.size),
		"IFGetDeviceID");
	print_as<char>(dID);


	GenTL::DEV_HANDLE hDevice;

	log(IFOpenDevice(hIF, (char*)dID.buffer, GenTL::DEVICE_ACCESS_READONLY, &hDevice),
		"IFOpenDevice");



	uint32_t num_stream = -1;
	log(DevGetNumDataStreams(hDevice, &num_stream),
		"DevGetNumDataStreams");

	std::cout << "Number of streams: " << num_stream << std::endl;

	//проверка на существование stream


	Buffer dDSID(20);
	log(DevGetDataStreamID(hDevice, 0, (char*)dDSID.buffer, &dDSID.size),
		"DevGetDataStreamID");
	print_as<char>(dDSID);



	GenTL::DS_HANDLE hDS;

	log(DevOpenDataStream(hDevice, (char*)dDSID.buffer, &hDS),
		"DevOpenDataStream");


	
	GCCloseLib();

	system("pause");
	return 0;
}

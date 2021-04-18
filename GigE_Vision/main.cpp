#include <iostream>
#include <assert.h>

#include <windows.h> 
#include <stdio.h> 

#include <string>
#include <vector>

#include "Buffer.hpp"
#include "Logger.hpp"
#include "LibLoader.hpp"
#include "Port.hpp"

#include "TL_Handler.hpp"
#include "IF_Handler.hpp"
#include "DEV_Handler.hpp"

#include "GenTL.h"

#include "GenICam.h"


#include <thread>

void Introduce_Lib()
{
	Buffer info_buffer(10);
	int a = 1;
	elog(GCGetInfo(1, &a, info_buffer.buffer, &info_buffer.size),
		"GCGetInfo");

	std::cout << "Library producer: ";
	print_as<char>(info_buffer);
}


void ArqFunction(GenTL::EVENT_HANDLE hEvent)
{
	Buffer data_buffer(10000);
	while (true)
	{
		auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, GENTL_INFINITE);
		if (err == 0)
		{
			std::cout << "DataSize: " << data_buffer.size<<std::endl;
			//std::cout << "Data: "; print_as<char>(data_buffer);
			std::cout << "GOT IT!";
		}
		else if (err != GenTL::GC_ERR_TIMEOUT)
		{
			elog(err, "ArqFunction");
		}
	}
}

int main()
{
	// TLSimu.cti
	// bgapi2_gige.cti
	Init_Lib("TLSimu.cti");

	GCInitLib();

	Introduce_Lib();

	TL_Handler tl_handler;
	tl_handler.Open();
	tl_handler.UpdateInterfaceList();
	tl_handler.ShowInterfaces();

	IF_Handler if_handler(tl_handler.GetInterface(0));
	if_handler.UpdateDeviceList();
	if_handler.ShowDevices();

	DEV_Handler dev_handler(if_handler.GetDevice(0));
	dev_handler.ShowStreams();

	GenTL::DS_HANDLE hDS = dev_handler.GetStream(0);

	//-------------------------------------------------------------

	//Требуется для дальнейшей подкачки XML файла!

	GenTL::PORT_HANDLE port = dev_handler.GetPort();

	uint32_t num_urls = -1;

	elog(GCGetNumPortURLs(port, &num_urls), "GCGetNumPortURLs");
	std::cout << "Num urls: " << num_urls << std::endl;

	Buffer info(40);
	int32_t iInfoCmd = GenTL::INFO_DATATYPE_STRING;
	elog(GCGetPortURLInfo(port, 0, GenTL::URL_INFO_FILE_REGISTER_ADDRESS, &iInfoCmd, info.buffer, &info.size), "GCGetPortURLInfo");

	uint64_t addres = read_as<uint64_t>(info);
	std::cout << "Adress: " << addres << std::endl;

	Buffer info2(40);
	int32_t iInfoCmd2 = GenTL::INFO_DATATYPE_STRING;

	elog(GCGetPortURLInfo(port, 0, GenTL::URL_INFO_FILE_SIZE, &iInfoCmd2, info2.buffer, &info2.size), "GCGetPortURLInfo");

	std::cout << "Filesize: ";
	print_as<uint64_t>(info2);

	Buffer read_port_buffer(read_as<uint64_t>(info2));
	elog(GCReadPort(port, addres, read_port_buffer.buffer, &read_port_buffer.size), "GCReadPort");
	//print_as<char>(read_port_buffer); //вывод XML

	//--------------------------------------------------------------------------

	int type = GenTL::INFO_DATATYPE_STRING;
	Buffer pay_load_size(100);
	elog(DSGetInfo(hDS, GenTL::STREAM_INFO_DEFINES_PAYLOADSIZE, &type, pay_load_size.buffer, &pay_load_size.size), "DSGetInfo");
	std::cout << "Does payloadsize defines: ";
	print_as<bool8_t>(pay_load_size);

	std::vector<Buffer> buf_reserv = { Buffer(10000),Buffer(10000) ,Buffer(10000) ,Buffer(10000) ,Buffer(10000) };

	std::vector<GenTL::BUFFER_HANDLE> ds_buffers = { nullptr ,nullptr ,nullptr ,nullptr ,nullptr };


	/*for (int i = 0; i < ds_buffers.size(); ++i)
	{
		elog(DSAnnounceBuffer(hDS, buf_reserv[i].buffer, buf_reserv[i].size,  nullptr, &ds_buffers[i]), "DSAllocAndAnnounceBuffer");
	}*/
	// or
	for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
	{
		elog(DSAllocAndAnnounceBuffer(hDS, 1000, nullptr, &(*it)), "DSAllocAndAnnounceBuffer");
	}

	for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
	{
		elog(DSQueueBuffer(hDS, *it), "DSQueueBuffer");
	}

	elog(DSStartAcquisition(hDS, GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE), "DSStartAcquisition");


	//----------------DataCapture----------------------------------------------------------

	GenTL::EVENT_HANDLE hEvent = nullptr;
	elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");

	std::thread thr(ArqFunction, hEvent);
	thr.detach();

	//-----------------Trying to use GenApi-------------------------------------------------

	using namespace GENAPI_NAMESPACE;
	using namespace GENICAM_NAMESPACE;

	CNodeMapRef Camera;

	gcstring xml_str((char*)read_port_buffer.buffer);

	Camera._LoadXMLFromString(xml_str);
	Port p;
	p.UsePort(dev_handler.GetPort());
	Camera._Connect((IPort*)&p);

	CCommandPtr ptrAcquisitionStart = Camera._GetNode("AcquisitionStart");
	if (IsWritable(ptrAcquisitionStart))
	{
		ptrAcquisitionStart->Execute();
		std::cout << "AcquisitionStart:" << ptrAcquisitionStart->IsDone() << std::endl;
	}

	//GCCloseLib();

	system("pause");
	return 0;
}

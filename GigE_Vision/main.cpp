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

#include "Camera.hpp"

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
	Buffer data_buffer(16);
	while (true)
	{
		auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
		if (err == 0)
		{
			std::cout << "DataSize: " << data_buffer.size<<std::endl;
			std::cout << "Data: "; print_as<uint32_t>(data_buffer);
			data_buffer = Buffer(16);
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

	//-----------------Trying to use GenApi-------------------------------------------------

	Port p;
	p.UsePort(dev_handler.GetPort());

	Camera camera;
	camera.LoadXML(Port::GetXML(dev_handler.GetPort()));
	camera.Connect((IPort*)&p);
	camera.SetWidthMin();
	camera.SetHeightMin();

	auto payloadSize = camera.PayloadSize();

	std::cout << "PayloadSize: " << payloadSize << std::endl;
	std::cout << "PixelFormat: " << camera.PixelFormat() << std::endl;

	//-----------------------Buffer preparing--------------------------------------

	int type = GenTL::INFO_DATATYPE_STRING;
	std::vector<Buffer> buf_reserv = { Buffer(payloadSize),Buffer(payloadSize) ,Buffer(payloadSize) ,Buffer(payloadSize) ,Buffer(payloadSize) };

	std::vector<GenTL::BUFFER_HANDLE> ds_buffers = { nullptr ,nullptr ,nullptr ,nullptr ,nullptr};

	/*for (int i = 0; i < ds_buffers.size(); ++i)
	{
		elog(DSAnnounceBuffer(hDS, buf_reserv[i].buffer, buf_reserv[i].size,  nullptr, &ds_buffers[i]), "DSAllocAndAnnounceBuffer");
	}*/
	// or
	for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
	{
		elog(DSAllocAndAnnounceBuffer(hDS, payloadSize, nullptr, &(*it)), "DSAllocAndAnnounceBuffer");
	}

	for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
	{
		elog(DSQueueBuffer(hDS, *it), "DSQueueBuffer");
	}

	elog(DSStartAcquisition(hDS, GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE), "DSStartAcquisition");

	//-----------------StartAcquisition---------------------------------------------------

	camera.StartAcquisition();

	//----------------DataCapture----------------------------------------------------------

	GenTL::EVENT_HANDLE hEvent = nullptr;
	elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");

	std::thread thr(ArqFunction, hEvent);
	thr.detach();

	//----------------------Chunks------------------------------------------------------------

	//Buffer buffer_payload(100);
	//while (true)
	//{
	//	for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
	//	{		
	//		elog(DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_PAYLOADTYPE, &type, buffer_payload.buffer, &buffer_payload.size) ,"DSGetBufferInfo");
	//		print_as<size_t>(buffer_payload);// == GenTL::PAYLOAD_TYPE_CHUNK_DATA)
	//		buffer_payload = Buffer(100);
	//		/*{
	//			buffer_payload = Buffer(100);
	//			std::cout << "!";
	//		}*/
	//	}
	//}


	system("pause");
	return 0;
}

#include <iostream>
#include <fstream>

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
	Buffer data_buffer(64);
	while (true)
	{
		auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
		if (err == 0)
		{
			std::cout << "DataSize: " << data_buffer.size<<std::endl;
			std::cout << "Data: "<< std::hex << read_as<int64_t>(data_buffer)<<std::endl;
			data_buffer = Buffer(64);
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
	int width = camera.SetWidth(640);
	int height = camera.SetHeight(640);

	auto payloadSize = camera.PayloadSize();

	std::cout << "PayloadSize: " << payloadSize << std::endl;
	std::cout << "PixelFormat: " << camera.PixelFormat() << std::endl;

	//-----------------------Buffer preparing--------------------------------------

	int type = GenTL::INFO_DATATYPE_STRING;
	
	std::vector<GenTL::BUFFER_HANDLE> ds_buffers = { nullptr ,nullptr ,nullptr ,nullptr ,nullptr ,nullptr };

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

	//std::thread thr(ArqFunction, hEvent);
	//thr.detach();

	//--------------------------------------------------------------------------------------

	Buffer data_buffer(64);

	while (true)
	{
		auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
		if (err == 0)
		{
			//std::cout << "Adress: " << read_as<int64_t>(data_buffer) << std::endl;

			for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
			{
				if (read_as<int64_t>(data_buffer) == (int64_t)(*it))
				{
					/*{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_SIZE, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "Size:   "; print_as<size_t>(buffer_info);}

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_DATA_LARGER_THAN_BUFFER, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "Data larger than buffer: "; print_as<bool8_t>(buffer_info);}

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_DATA_SIZE, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "Data size: "; print_as<size_t>(buffer_info); }

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_DATA_LARGER_THAN_BUFFER, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "Data larger than buffer: "; print_as<bool8_t>(buffer_info);}*/
					/*{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_XPADDING, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "XPadding: "; print_as<size_t>(buffer_info); }

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_YPADDING, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "YPadding: "; print_as<size_t>(buffer_info); }

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_XOFFSET, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "XOffset: "; print_as<size_t>(buffer_info); }

					{Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_YOFFSET, &type, buffer_info.buffer, &buffer_info.size);
					std::cout << "YOffset: "; print_as<size_t>(buffer_info); }*/

					Buffer buffer_info(20);
					DSGetBufferInfo(hDS, *it, GenTL::BUFFER_INFO_BASE, &type, buffer_info.buffer, &buffer_info.size);
					//std::cout << "Base:   "; print_as<size_t>(buffer_info); 					

					unsigned char* buf = nullptr;
					buf = read_as<unsigned char*>(buffer_info);

					{
						std::ofstream fstream;
						fstream.open("D:\\source\\repos\\Python\\pictures\\mono8_640_640.txt");

						fstream << (int)buf[0];
						for (int i = 1; i < payloadSize; i++)
						{
							fstream <<' '<< (int)buf[i];
						}

						system("pause");
						exit(1);
					}
					

					//elog(DSQueueBuffer(hDS, *it), "DSQueueBuffer");
				}
			}

			data_buffer = Buffer(64);
		}
		else if (err != GenTL::GC_ERR_TIMEOUT)
		{
			elog(err, "ArqFunction");
		}
	}

	system("pause");
	return 0;
}

#include <iostream>
#include <fstream>
#include <assert.h>
#include <windows.h> 
#include <stdio.h> 
#include <string>
#include <vector>
#include <thread>

#include "Buffer.hpp"
#include "Logger.hpp"
#include "LibLoader.hpp"
#include "Port.hpp"
#include "ImageAcquirer.hpp"
#include "TransportLayer.hpp"
#include "Interface.hpp"
#include "Device.hpp"
#include "Camera.hpp"

#include "GenTL.h"
#include "GenICam.h"

#define IMAGE_TO_FILE true

void Introduce_Lib()
{
	Buffer info_buffer(20);
	int a = 1;
	elog(GCGetInfo(1, &a, info_buffer.buffer, &info_buffer.size),
		"GCGetInfo");

	std::cout << "Library producer: ";
	print_as<char>(info_buffer);
}

int main()
{
	//формат вывода некоторых функций
	int type = GenTL::INFO_DATATYPE_STRING;

	Init_Lib("TLSimu.cti");
	GCInitLib();
	Introduce_Lib();

	TransportLayer tl_handler;
	tl_handler.Open();
	tl_handler.UpdateInterfaceList();
	tl_handler.ShowInterfaces();

	Interface if_handler(tl_handler.GetInterface(0));
	if_handler.UpdateDeviceList();
	if_handler.ShowDevices();

	Device dev_handler(if_handler.GetDevice(1));
	dev_handler.ShowStreams();

	GenTL::DS_HANDLE hDS = dev_handler.GetStream(0);

	//-----------------GenApi-------------------------------------------------------

	Port p;
	p.UsePort(dev_handler.GetPort());

	Camera camera;
	camera.LoadXML(Port::GetXML(dev_handler.GetPort()));
	camera.Connect((IPort*)&p);
	camera.SetWidth(640);
	camera.SetHeight(640);

	auto payloadSize = camera.PayloadSize();

	std::cout << "PayloadSize: " << payloadSize << std::endl;
	std::cout << "PixelFormat: " << camera.PixelFormat() << std::endl;

	//-----------------------Buffer preparing--------------------------------------

	ImageAcquirer imageAcq;
	imageAcq.AnnounceBuffers(hDS, payloadSize);
	imageAcq.StartAcquisition(hDS);
	std::vector<GenTL::BUFFER_HANDLE> ds_buffers = imageAcq.GetBuffers();

	//-----------------StartAcquisition---------------------------------------------------

	camera.StartAcquisition();

	//----------------DataCapture----------------------------------------------------------

	GenTL::EVENT_HANDLE hEvent = nullptr;
	elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");

	//Можно попробовать асинхронно сделать
	//std::thread thr(func() , events , buffers)
	//thr.detach();

	//--------------------------------------------------------------------------------------

	Buffer data_buffer(64);

	while (true)
	{
		auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
		if (err == 0)
		{
			//Тут можно по-человечески потом сделать
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
					//по-хорошему обработать нужно отступы
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

					unsigned char* buf = nullptr;
					buf = read_as<unsigned char*>(buffer_info);

					if (IMAGE_TO_FILE)
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
					else
					{
						std::cout << *it << " is ready!" << std::endl;
					}
					
					//обновление буферов - непрерывный поток данных
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

	//при закрытии все открывающиеся штуки нужно позакрывать

	system("pause");
	return 0;
}

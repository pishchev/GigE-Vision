#pragma once

#include <fstream>
#include <assert.h>
#include <windows.h> 
#include <stdio.h> 
#include <string>
#include <vector>
#include <thread>
#include <queue>

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


class GigeManager
{
public:

	GigeManager(){}

	void Init(std::string cti)
	{
		Init_Lib(cti);
		GCInitLib();

		tl_handler.Open();
		tl_handler.UpdateInterfaceList();
	}

	void useInterface(int int_num)
	{
		if_handler.setInterfaces(tl_handler.GetInterface(int_num));
		if_handler.UpdateDeviceList();
	}

	void useDevice(int dev_num)
	{
		dev_handler.setDevice(if_handler.GetDevice(dev_num));
		hDS = dev_handler.GetStream(0);
		p.UsePort(dev_handler.GetPort());
	}

	void cameraInit()
	{
		camera.LoadXML(Port::GetXML(dev_handler.GetPort()));
		camera.Connect((IPort*)&p);
		camera.SetWidth(8);
		camera.SetHeight(8);

		payloadSize = camera.PayloadSize();
	}

	void acquirerPreparing()
	{
		imageAcq.AnnounceBuffers(hDS, payloadSize);
		imageAcq.StartAcquisition(hDS);
		ds_buffers = imageAcq.GetBuffers();
	}

	void startAcquisition()
	{
		camera.StartAcquisition();
		elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");
	}

	void getImage()
	{
		while (true)
		{
			auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
			if (err == 0)
			{
				Buffer buffer_info(20);
				DSGetBufferInfo(hDS, read_as<void*>(data_buffer), GenTL::BUFFER_INFO_BASE, &type, buffer_info.buffer, &buffer_info.size);

				unsigned char* buf = nullptr;
				buf = read_as<unsigned char*>(buffer_info);

				std::cout << std::endl << read_as<int64_t>(data_buffer) << " is ready!" << std::endl;

				for (int i = 0; i < payloadSize; i++)
				{
					std::cout << ' ' << (int)buf[i];
				}

				elog(DSQueueBuffer(hDS, read_as<GenTL::BUFFER_HANDLE>(data_buffer)), "DSQueueBuffer");

				data_buffer = Buffer(64);
			}
			else if (err != GenTL::GC_ERR_TIMEOUT)
			{
				elog(err, "ArqFunction");
			}
		}
	}

private:

	TransportLayer tl_handler;
	Interface if_handler;
	Device dev_handler;
	Port p;
	Camera camera;
	ImageAcquirer imageAcq;

	std::vector<GenTL::BUFFER_HANDLE> ds_buffers;
	std::queue<GenTL::BUFFER_HANDLE>ready_buffers{};

	Buffer data_buffer{64};

	GenTL::EVENT_HANDLE hEvent = nullptr;

	int type = GenTL::INFO_DATATYPE_STRING;

	GenTL::DS_HANDLE hDS;

	int64_t payloadSize = 0;
};

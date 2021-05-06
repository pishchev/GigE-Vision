#pragma once

#include <fstream>
#include <assert.h>
#include <windows.h> 
#include <stdio.h> 
#include <string>
#include <vector>
#include <thread>
#include <queue>
#include <memory>

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
		image = new int[payloadSize];

		imageAcq.AnnounceBuffers(hDS, payloadSize);
		imageAcq.StartAcquisition(hDS);
		ds_buffers = imageAcq.GetBuffers();
	}

	void startAcquisition()
	{
		camera.StartAcquisition();
		elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");
		asyncAcquisition();
	}

	void getImageFull()
	{
		while (true)
		{
			auto err = EventGetData(hEvent, data_buffer1.buffer, &data_buffer1.size, 10000);
			if (err == 0)
			{
				Buffer buffer_info(20);
				DSGetBufferInfo(hDS, read_as<void*>(data_buffer1), GenTL::BUFFER_INFO_BASE, &type, buffer_info.buffer, &buffer_info.size);

				unsigned char* buf = nullptr;
				buf = read_as<unsigned char*>(buffer_info);

				//std::cout << std::endl << read_as<int64_t>(data_buffer1) << " is ready!" << std::endl;

				for (int i = 0; i < payloadSize; i++)
				{
					image[i] = (int)buf[i];
				}

				elog(DSQueueBuffer(hDS, read_as<GenTL::BUFFER_HANDLE>(data_buffer1)), "DSQueueBuffer");

			}
			else if (err != GenTL::GC_ERR_TIMEOUT)
			{
				elog(err, "ArqFunction");
			}
		}
	}

	size_t imageSize()
	{
		return payloadSize;
	}
	

	bool getImage(int* buffer , size_t size)
	{
		if (!payloadSize>size || !is_ready)
		{
			return false;
		}

		memcpy(buffer, image, sizeof(int) * size);

		return true;
	}

	void timer()
	{
		next = true;
	}

private:

	void asyncAcquisition()
	{
		std::thread thr(asyncCapture, std::ref(*this));
		thr.detach();
	}

	static void asyncCapture(GigeManager& manager)
	{
		while (true)
		{
			if (!manager.next) continue;

			Buffer data_buffer1(64);
			auto err = EventGetData(manager.hEvent, data_buffer1.buffer, &data_buffer1.size, 10000);
			if (err == 0)
			{
				Buffer buffer_info(20);
				DSGetBufferInfo(manager.hDS, read_as<void*>(data_buffer1), GenTL::BUFFER_INFO_BASE, &manager.type, buffer_info.buffer, &buffer_info.size);

				unsigned char* buf = nullptr;
				buf = read_as<unsigned char*>(buffer_info);


				for (int i = 0; i < manager.payloadSize; i++)
				{
					manager.image[i] = (int)buf[i];
				}
				manager.is_ready = true;

				elog(DSQueueBuffer(manager.hDS, read_as<GenTL::BUFFER_HANDLE>(data_buffer1)), "DSQueueBuffer");

				manager.next = false;

			}
			else if (err != GenTL::GC_ERR_TIMEOUT)
			{
				elog(err, "ArqFunction");
			}
		}
	}

	TransportLayer tl_handler;
	Interface if_handler;
	Device dev_handler;
	Port p;
	Camera camera;
	ImageAcquirer imageAcq;

	std::vector<GenTL::BUFFER_HANDLE> ds_buffers;

	bool next = false;
	bool is_ready = false;
	int* image = nullptr;

	Buffer data_buffer1{64};

	Buffer data_buffer{64};

	GenTL::EVENT_HANDLE hEvent = nullptr;

	int type = GenTL::INFO_DATATYPE_STRING;

	GenTL::DS_HANDLE hDS;

	int64_t payloadSize = 0;
};

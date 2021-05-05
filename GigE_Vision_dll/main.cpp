#include <iostream>
#include "GigeManager.hpp"


int main()
{
	////формат вывода некоторых функций
	//int type = GenTL::INFO_DATATYPE_STRING;

	//Init_Lib("TLSimu.cti");
	//GCInitLib();

	//TransportLayer tl_handler;
	//tl_handler.Open();
	//tl_handler.UpdateInterfaceList();

	//Interface if_handler;
	//if_handler.setInterfaces(tl_handler.GetInterface(0));
	//if_handler.UpdateDeviceList();

	//Device dev_handler;
	//dev_handler.setDevice(if_handler.GetDevice(1));

	//GenTL::DS_HANDLE hDS = dev_handler.GetStream(0);

	////-----------------GenApi-------------------------------------------------------

	//Port p;
	//p.UsePort(dev_handler.GetPort());

	//Camera camera;
	//camera.LoadXML(Port::GetXML(dev_handler.GetPort()));
	//camera.Connect((IPort*)&p);
	//camera.SetWidth(8);
	//camera.SetHeight(8);

	//auto payloadSize = camera.PayloadSize();

	////-----------------------Buffer preparing--------------------------------------

	//ImageAcquirer imageAcq;
	//imageAcq.AnnounceBuffers(hDS, payloadSize);
	//imageAcq.StartAcquisition(hDS);
	//std::vector<GenTL::BUFFER_HANDLE> ds_buffers = imageAcq.GetBuffers();

	////-----------------StartAcquisition---------------------------------------------------

	//camera.StartAcquisition();

	////----------------DataCapture----------------------------------------------------------

	//GenTL::EVENT_HANDLE hEvent = nullptr;
	//elog(GCRegisterEvent(hDS, GenTL::EVENT_NEW_BUFFER, &hEvent), "GCRegisterEvent");

	////--------------------------------------------------------------------------------------

	//Buffer data_buffer(64);

	//while (true)
	//{
	//	auto err = EventGetData(hEvent, data_buffer.buffer, &data_buffer.size, 10000);
	//	if (err == 0)
	//	{
	//		Buffer buffer_info(20);
	//		DSGetBufferInfo(hDS, read_as<void*>(data_buffer), GenTL::BUFFER_INFO_BASE, &type, buffer_info.buffer, &buffer_info.size);

	//		unsigned char* buf = nullptr;
	//		buf = read_as<unsigned char*>(buffer_info);

	//		std::cout << std::endl << read_as<int64_t>(data_buffer) << " is ready!" << std::endl;

	//		for (int i = 0; i < payloadSize; i++)
	//		{
	//			std::cout << ' ' << (int)buf[i];
	//		}

	//		elog(DSQueueBuffer(hDS, read_as<GenTL::BUFFER_HANDLE>(data_buffer)), "DSQueueBuffer");

	//		data_buffer = Buffer(64);
	//	}
	//	else if (err != GenTL::GC_ERR_TIMEOUT)
	//	{
	//		elog(err, "ArqFunction");
	//	}
	//}

	GigeManager gige;
	gige.Init("TLSimu.cti");
	gige.useInterface(0);
	gige.useDevice(1);
	gige.cameraInit();
	gige.acquirerPreparing();
	gige.startAcquisition();
	gige.getImage();

	system("pause");
	return 0;
}

#pragma once
#include <iostream>

#include "Port.hpp"
#include "GenTL.h"
#include "GenICam.h"

using namespace GENAPI_NAMESPACE;
using namespace GENICAM_NAMESPACE;

class Camera
{
public:
	void LoadXML(gcstring xml_str)
	{
		cam._LoadXMLFromString(xml_str);
	}

	void Connect(IPort* port)
	{
		cam._Connect(port);
	}

	void StartAcquisition()
	{
		CCommandPtr ptrAcquisitionStart = cam._GetNode("AcquisitionStart");
		if (IsWritable(ptrAcquisitionStart))
		{
			ptrAcquisitionStart->Execute();
			std::cout << "AcquisitionStart:" << ptrAcquisitionStart->IsDone() << std::endl;
		}
	}

	void SetWidthMin()
	{
		CIntegerPtr ptrWidth = cam._GetNode("Width");
		if (IsWritable(ptrWidth))
		{
			*ptrWidth = ptrWidth->GetMin();
			std::cout << "Width: "<< ptrWidth->GetValue() <<std::endl;
		}
	}
	void SetHeightMin()
	{
		CIntegerPtr ptrHeight = cam._GetNode("Height");
		if (IsWritable(ptrHeight))
		{
			*ptrHeight = ptrHeight->GetMin();
			std::cout << "Height: " << ptrHeight->GetValue() << std::endl;
		}
	}

	int64_t PayloadSize()
	{
		CIntegerPtr ptrPayloadSize = cam._GetNode("PayloadSize");
		if (IsReadable(ptrPayloadSize))
		{
			return ptrPayloadSize->GetValue();
		}
		return -1;
	}

	gcstring PixelFormat()
	{
		CEnumerationPtr ptrPixelFormat = cam._GetNode("PixelFormat");
		if (IsReadable(ptrPixelFormat))
		{
			return ptrPixelFormat->GetCurrentEntry()->GetSymbolic();
		}
		return "Not readable node:(";
	}

private:
	CNodeMapRef cam;
};
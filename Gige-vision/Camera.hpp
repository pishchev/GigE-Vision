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
		}
	}
	
	int SetWidthMin()
	{
		CIntegerPtr ptrWidth = cam._GetNode("Width");
		if (IsWritable(ptrWidth))
		{
			*ptrWidth = ptrWidth->GetMin();
		}
		return ptrWidth->GetValue();
	}
	int SetHeightMin()
	{
		CIntegerPtr ptrHeight = cam._GetNode("Height");
		if (IsWritable(ptrHeight))
		{
			*ptrHeight = ptrHeight->GetMin();
		}
		return ptrHeight->GetValue();
	}

	int SetHeight(size_t h)
	{
		CIntegerPtr ptrHeight = cam._GetNode("Height");
		if (IsWritable(ptrHeight))
		{
			*ptrHeight = h;
		}
		return ptrHeight->GetValue();
	}
	int SetWidth(size_t w)
	{
		CIntegerPtr ptrWidth = cam._GetNode("Width");
		if (IsWritable(ptrWidth))
		{
			*ptrWidth = w;
		}
		return ptrWidth->GetValue();
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
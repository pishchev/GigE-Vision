#pragma once

#include "LibLoader.hpp"
#include "GenICam.h"
#include "Buffer.hpp"

GENICAM_INTERFACE GENAPI_DECL_ABSTRACT Port : public GENAPI_NAMESPACE::IPort
{

	GenTL::PORT_HANDLE hPort;


	void UsePort(GenTL::PORT_HANDLE port)
	{
		hPort = port;
	}

	//! Reads a chunk of bytes from the port
	virtual void Read(void* pBuffer, int64_t Address, int64_t Length)
	{
		size_t s = Length;
		auto err = GCReadPort(hPort , Address, pBuffer , &s);
		Length = s;
	}

	//! Writes a chunk of bytes to the port
	virtual void Write(const void* pBuffer, int64_t Address, int64_t Length)
	{
		size_t s = Length;
		auto err = GCWritePort(hPort, Address, pBuffer, &s);
		Length = s;
	}

	//! Get the access mode of the node
	virtual GENAPI_NAMESPACE::EAccessMode GetAccessMode() const
	{
		Buffer read(10);
		int type = 1;
		auto err = GCGetPortInfo(hPort, GenTL::PORT_INFO_ACCESS_READ, &type, read.buffer , &read.size);
		elog(err, "GetAccessMode");

		Buffer write(10);
		err = GCGetPortInfo(hPort, GenTL::PORT_INFO_ACCESS_WRITE, &type, write.buffer, &write.size);
		elog(err, "GetAccessMode");

		Buffer na(10);
		err = GCGetPortInfo(hPort, GenTL::PORT_INFO_ACCESS_NA, &type, na.buffer, &na.size);
		elog(err, "GetAccessMode");

		Buffer ni(10);
		err = GCGetPortInfo(hPort, GenTL::PORT_INFO_ACCESS_NI, &type, ni.buffer, &ni.size);
		elog(err, "GetAccessMode");

		if (read_as<bool8_t>(read) && read_as<bool8_t>(write))return GENAPI_NAMESPACE::EAccessMode::RW;
		if (read_as<bool8_t>(read))return GENAPI_NAMESPACE::EAccessMode::RO;
		if (read_as<bool8_t>(write))return GENAPI_NAMESPACE::EAccessMode::WO;
		if (read_as<bool8_t>(na))return GENAPI_NAMESPACE::EAccessMode::NA;
		if (read_as<bool8_t>(ni))return GENAPI_NAMESPACE::EAccessMode::NI;
	}

};
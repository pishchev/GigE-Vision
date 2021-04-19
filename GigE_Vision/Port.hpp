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

	static GENICAM_NAMESPACE::gcstring GetXML(GenTL::PORT_HANDLE port)
	{
		uint32_t num_urls = -1;

		elog(GCGetNumPortURLs(port, &num_urls), "GCGetNumPortURLs");

		Buffer info(40);
		int32_t iInfoCmd = GenTL::INFO_DATATYPE_STRING;
		elog(GCGetPortURLInfo(port, 0, GenTL::URL_INFO_FILE_REGISTER_ADDRESS, &iInfoCmd, info.buffer, &info.size), "GCGetPortURLInfo");

		uint64_t addres = read_as<uint64_t>(info);

		Buffer info2(40);
		int32_t iInfoCmd2 = GenTL::INFO_DATATYPE_STRING;

		elog(GCGetPortURLInfo(port, 0, GenTL::URL_INFO_FILE_SIZE, &iInfoCmd2, info2.buffer, &info2.size), "GCGetPortURLInfo");

		Buffer read_port_buffer(read_as<uint64_t>(info2));
		elog(GCReadPort(port, addres, read_port_buffer.buffer, &read_port_buffer.size), "GCReadPort");
		//print_as<char>(read_port_buffer); //вывод XML

		return GENICAM_NAMESPACE::gcstring((char*)read_port_buffer.buffer);
	}
};

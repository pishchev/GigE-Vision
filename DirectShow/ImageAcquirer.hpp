#pragma once

#include "Buffer.hpp"
#include <vector>
#include "GenTL.h"
#include "Logger.hpp"

class ImageAcquirer
{
public:
	void AnnounceBuffers(GenTL::DS_HANDLE hDS , size_t payloadSize)
	{
		int type = 1;
		Buffer num_buf_announce(20);
		elog(DSGetInfo(hDS, GenTL::STREAM_INFO_BUF_ANNOUNCE_MIN, &type, num_buf_announce.buffer, &num_buf_announce.size), "DSGetInfo");
		size_t num_buf = read_as<size_t>(num_buf_announce);

		ds_buffers = std::vector<GenTL::BUFFER_HANDLE>(num_buf, nullptr);

		for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
		{
			elog(DSAllocAndAnnounceBuffer(hDS, payloadSize, nullptr, &(*it)), "DSAllocAndAnnounceBuffer");
		}

		for (auto it = ds_buffers.begin(); it != ds_buffers.end(); ++it)
		{
			elog(DSQueueBuffer(hDS, *it), "DSQueueBuffer");
		}
	}

	void StartAcquisition(GenTL::DS_HANDLE hDS)
	{
		elog(DSStartAcquisition(hDS, GenTL::ACQ_START_FLAGS_DEFAULT, GENTL_INFINITE), "DSStartAcquisition");
	}

	std::vector<GenTL::BUFFER_HANDLE> GetBuffers()
	{
		return ds_buffers;
	}

private:
	std::vector<GenTL::BUFFER_HANDLE> ds_buffers;
};
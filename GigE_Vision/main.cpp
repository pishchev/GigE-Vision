#include <iostream>
#include <assert.h>

#include <windows.h> 
#include <stdio.h> 

#include <string>

#include "Buffer.hpp"
#include "Logger.hpp"

#include "GenTL.h"


int main()
{

	HMODULE gentl = LoadLibraryA("TLSimu.cti");
	//HMODULE gentl = LoadLibraryA("bgapi2_gige.cti");
	assert(gentl != nullptr);

	GenTL::PGCInitLib GCInitLib;
	GCInitLib = (GenTL::PGCInitLib)GetProcAddress(gentl, "GCInitLib");
	assert(GCInitLib != nullptr);

	GenTL::PGCCloseLib GCCloseLib;
	GCCloseLib = (GenTL::PGCCloseLib)GetProcAddress(gentl, "GCCloseLib");
	assert(GCCloseLib != nullptr);

	GenTL::PGCGetInfo GCGetInfo;
	GCGetInfo = (GenTL::PGCGetInfo)GetProcAddress(gentl, "GCGetInfo");
	assert(GCGetInfo != nullptr);

	GenTL::PTLOpen TLOpen;
	TLOpen = (GenTL::PTLOpen)GetProcAddress(gentl, "TLOpen");
	assert(TLOpen != nullptr);
	

	Logger log(gentl);
	

	GCInitLib();

	{
		Buffer info_buffer(10);
		int a = 1;
		log(	GCGetInfo(1, &a, info_buffer.buffer, &info_buffer.size)
				,"GCGetInfo");
		print_as<char>(info_buffer);
	}
	
	GenTL::TL_HANDLE phTL;
	(TLOpen)(&phTL);



	GCCloseLib();
	system("pause");
	return 0;
}

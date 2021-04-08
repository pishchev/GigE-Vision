#pragma once

#include <iostream>
#include <windows.h> 
#include <assert.h>

#include "GenTL.h"

#include "LibLoader.hpp"


static void log(GenTL::GC_ERROR err, std::string oper)
{
	size_t err_size = 100;
	char* err_ch = new char[err_size];

	auto log_error = GCGetLastError(&err, err_ch, &err_size);

	if (log_error != 0)
	{
		std::cout << "Logging error! Error descriptor: " << log_error << std::endl;

		delete[] err_ch;
		return;
	}

	std::cout << "log -> Operation " << oper << ". Status: " << err_ch << std::endl;
	delete[] err_ch;

	if (err != 0)
	{
		std::cout << "Error code: " << err << std::endl;
		//system("pause");
		//exit(-1);
	}
}



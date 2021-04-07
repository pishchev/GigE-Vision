#pragma once

class Buffer
{
public:
	Buffer(size_t size) :size(size)
	{
		buffer = new char[size];
	}
	~Buffer()
	{
		delete[] buffer;
	}

	size_t size;
	void* buffer = nullptr;

};

template<typename T>
void print_as(const Buffer& buffer)
{
	std::cout << (T*)buffer.buffer << std::endl;
}
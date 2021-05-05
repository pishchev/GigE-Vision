#pragma once
#include <typeinfo>

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


template<typename T , class = typename std::enable_if<std::is_same<T,char>::value> ::type>
void print_as(const Buffer& buffer)
{
	std::cout << (T*)buffer.buffer << std::endl;
}

template<typename T, class = typename std::enable_if<!std::is_same<T, char>::value>::type , int = 0>
void print_as(const Buffer& buffer)
{
	std::cout << *((T*)buffer.buffer) << std::endl;
}

template<typename T>
T read_as(const Buffer& buffer)
{
	return *((T*)buffer.buffer);
}
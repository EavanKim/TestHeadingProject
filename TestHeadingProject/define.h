#pragma once

#pragma pack(push, 1)
struct Header
{
	uint64_t type = 0;
	uint64_t length = 0;
};

template<uint64_t _type, uint64_t _buffersize>
struct SendStruct : public Header
{
	char buffer[ _buffersize ];

	SendStruct()
	{
		type = _type;
		length = sizeof( Header ) + _buffersize;
	}
};

typedef SendStruct<1, 0> SessionKey;
typedef SendStruct<2, 0> Shutdown;
typedef SendStruct<100, 43> TestBuffer;
#pragma pack(pop)
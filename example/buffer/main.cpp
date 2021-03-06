/*
* --*main*--
* @Author: Payton
* @Last  : Payton
*/
#include <Buffer.h>
#include <memory>
#include <cstring>
#include <Log.h>
#include "LogThread.h"


using namespace sc;
using namespace std;

void write_buff(std::shared_ptr<Buffer>& buffer)
{
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
	buffer->writeInt64(uint64_t(64));
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
	buffer->writeInt16(uint16_t(16));
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
	buffer->writeString("string");
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
	buffer->writeInt32(uint32_t(32));
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
	buffer->writeInt8(uint8_t(8));
	LOG_INFO("%d,%d",buffer->readableBytes(),buffer->writeableBytes());
}

void read_buff(std::shared_ptr<Buffer>& buffer)
{
	LOG_INFO("");
	LOG_INFO("%d",buffer->readableBytes());
	int64_t num64;
	buffer->peekInt64(num64);
	LOG_INFO("peekInt64,%d,%d",num64,buffer->readableBytes());
	buffer->readInt64(num64);
	LOG_INFO("num64,%d,%d",num64,buffer->readableBytes());

	int16_t num16;
	buffer->peekInt16(num16);
	LOG_INFO("peekInt16,%d,%d",num16,buffer->readableBytes());
	buffer->readInt16(num16);
	LOG_INFO("num16,%d,%d",num16,buffer->readableBytes());

	std::string str;
	buffer->peekString(str);
	LOG_INFO("peekString,%s,%d",str.c_str(),buffer->readableBytes());
	buffer->readString(str);
	LOG_INFO("str,%s,%d",str.c_str(),buffer->readableBytes());

	int32_t num32;
	buffer->peekInt32(num32);
	LOG_INFO("peekInt32,%d,%d",num32,buffer->readableBytes());
	buffer->readInt32(num32);
	LOG_INFO("num32,%d,%d",num32,buffer->readableBytes());

	int8_t num8;
	buffer->peekInt8(num8);
	LOG_INFO("peekInt8,%d,%d",num8,buffer->readableBytes());
	buffer->readInt8(num8);
	LOG_INFO("num8,%d,%d",num8,buffer->readableBytes());
}

int main()
{
	LOG_INSTANCE->start();
	std::shared_ptr<Buffer> buffer(new Buffer(8));
	write_buff(buffer);
	read_buff(buffer);

	LOG_INFO("can_read %d",buffer->is_readable(10));

	std::shared_ptr<Buffer> buffer_1024(new Buffer(1024));
	buffer_1024->swap(*buffer);
	LOG_INFO("writeableBytes %d",buffer->writeableBytes());
	LOG_INFO("writeableBytes %d",buffer_1024->is_readable(10));

	LOG_INSTANCE->stop();
	return 0;
}
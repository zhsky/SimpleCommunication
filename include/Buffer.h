/*
* --*Buffer*--
* @Author: Payton
* @Last  : Payton
*/

#include <vector>
#include <algorithm>
#include <stdint.h>
#include <string.h>
#include <string>
#include <assert.h>

namespace sc
{
class Buffer
{
public:
	explicit Buffer(size_t init_size = 1024):buff_(init_size),read_index_(0),write_index_(0){}

	void writeInt8(int8_t data)
	{
		append(&data,sizeof(int8_t));
	}
	void writeInt16(int16_t data)
	{
		int16_t v = htobe16(data);
		append(&v,sizeof(int16_t));
	}
	void writeInt32(int32_t data)
	{
		int32_t v = htobe32(data);
		append(&v,sizeof(int32_t));
	}
	void writeInt64(int64_t data)
	{
		int64_t v = htobe64(data);
		append(&v,sizeof(int64_t));
	}
	void writeString(const std::string& data)
	{
		uint16_t len = data.length();
		writeInt16(len);
		if(len <= 0) return;
		append(data.c_str(),len);
	}

	void peekInt8(int8_t& data)
	{
		assert(is_readable(sizeof(int8_t)));
		memcpy(&data,readptr(),sizeof(int8_t));
		return;
	}
	void peekInt16(int16_t& data)
	{
		assert(is_readable(sizeof(int16_t)));
		memcpy(&data,readptr(),sizeof(int16_t));
		data = be16toh(data);
		return;
	}
	void peekInt32(int32_t& data)
	{
		assert(is_readable(sizeof(int32_t)));
		memcpy(&data,readptr(),sizeof(int32_t));
		data = be32toh(data);
		return;
	}
	void peekInt64(int64_t& data)
	{
		assert(is_readable(sizeof(int64_t)));
		memcpy(&data,readptr(),sizeof(int64_t));
		data = be64toh(data);
		return;
	}
	void peekString(std::string& data)
	{
		int16_t len = 0;
		peekInt16(len);
		if(len <= 0) return;
		assert(is_readable(sizeof(int16_t) + len));
		data.resize(len);
		memcpy(const_cast<char*>(data.c_str()),readptr() + sizeof(int16_t),len);
	}

	void readInt8(int8_t& data)
	{
		peekInt8(data);
		read_index_ += sizeof(int8_t);
	}
	void readInt16(int16_t& data)
	{
		peekInt16(data);
		read_index_ += sizeof(int16_t);
	}
	void readInt32(int32_t& data)
	{
		peekInt32(data);
		read_index_ += sizeof(int32_t);
	}
	void readInt64(int64_t& data)
	{
		peekInt64(data);
		read_index_ += sizeof(int64_t);
	}
	void readString(std::string& data)
	{
		peekString(data);
		read_index_ += sizeof(int16_t) + data.length();
	}

public:
	void append(const char* data, size_t len)
	{
		ensureWriteable(len);
		std::copy(data,data + len,writeptr());
		write_index_ += len;
	}
	void append(const void* data, size_t len)
	{
		append(static_cast<const char*>(data),len);
	}

	size_t writeableBytes()
	{
		return buff_.size() - write_index_;
	}
	size_t readableBytes()
	{
		return write_index_ - read_index_;
	}
	bool is_readable(size_t len)
	{
		return readableBytes() >= len;
	}
	char* writeptr()
	{
		return &(buff_[write_index_]);
	}
	char* readptr()
	{
		return &(buff_[read_index_]);
	}

	void set_write_index(size_t index)
	{
		assert(index <= buff_.size());
		write_index_ = index;
	}
	size_t get_write_index()
	{
		return write_index_;
	}
	void set_read_index(size_t index)
	{
		assert(index <= write_index_);
		read_index_ = index;
	}
	size_t get_read_index()
	{
		return read_index_;
	}
	void swap(Buffer& oth)
	{
		std::swap(read_index_,oth.read_index_);
		std::swap(write_index_,oth.write_index_);
		buff_.swap(oth.buff_);
	}
private:
	void ensureWriteable(size_t len)
	{
		if(writeableBytes() < len)
		{
			buff_.resize(write_index_ + len);
		}
	}
private:
	std::vector<char> buff_;
	size_t read_index_;
	size_t write_index_;
};

}//namespace sc
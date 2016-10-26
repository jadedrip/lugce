#pragma once
#include <boost/iostreams/concepts.hpp>  // sink
#include <boost/iostreams/stream_buffer.hpp>
#include "buffer.hpp"

namespace lugce{
	class buffer_sink : public boost::iostreams::sink
	{
	public:
		buffer_sink( lugce::buffer& buf ) : _buf(buf){}
		std::streamsize write(const char* s, std::streamsize n)
		{
			_buf.append( s, (size_t)n );
			return n;
		}
	private:
		lugce::buffer &_buf;
	};
};

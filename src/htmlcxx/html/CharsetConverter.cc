#include <iconv.h>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include "CharsetConverter.h"

using namespace std;
using namespace htmlcxx;

CharsetConverter::CharsetConverter(const string &from, const string &to)
	: mIconvDescriptor(iconv_open(to.c_str(), from.c_str()))
{	
}

CharsetConverter::~CharsetConverter()
{
	if(isOk())
	{
		iconv_close(mIconvDescriptor);
	}
}

bool CharsetConverter::isOk()
{
	return (mIconvDescriptor != (iconv_t)(-1));
}

string CharsetConverter::convert(const string &input)
{
	const char *inbuf = input.c_str();
	size_t inbytesleft = input.length();

	size_t outbuf_len = 2 * input.length();
	char *outbuf_start = new char[outbuf_len];
	char *outbuf = outbuf_start;
	size_t outbytesleft = outbuf_len;

	size_t ret;
	while (1) {
		ret = iconv(mIconvDescriptor, const_cast<char**>(&inbuf), &inbytesleft, &outbuf, &outbytesleft);
		if (ret == 0) break;
		if (ret == (size_t)-1 && errno == E2BIG) return string();

		//				fprintf(stderr, "invalid byte: %d\n", inbuf - input.c_str());

		inbuf++; inbytesleft--;
	}

	string out(outbuf_start, outbuf_len - outbytesleft);

	delete [] outbuf_start;

	return out;
}

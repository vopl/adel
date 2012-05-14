#include "pch.hpp"
#include "http/impl/inputMessage.hpp"
#include "http/log.hpp"
#include "http/method.hpp"
#include "http/headerName.hpp"

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_string.hpp>
#include <boost/spirit/include/qi_char.hpp>
#include <boost/spirit/include/qi_lit.hpp>

#include <boost/spirit/include/phoenix_core.hpp>

namespace http { namespace impl
{

	InputMessage::InputMessage(const net::Channel &channel, size_t granula)
		: _channel(channel)
		, _granula(granula)
		, _bufferAccumuler(new ContentFilterBufferAccumuler)
		, _contentFilter(_bufferAccumuler)
		, _em(em_firstLine)
	{
	}

	//////////////////////////////////////////////////////////////////////////
	InputMessage::~InputMessage()
	{
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::isConnected() const
	{
		return _channel.isOpen();
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readFirstLine()
	{
		if(em_firstLine == _em)
		{
			if(!readUntil("\r\n", _firstLine))
			{
				return false;
			}

			_readedPos = _firstLine.end() + 2;
			_em = em_headers;
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readHeaders()
	{
		if(em_headers > _em)
		{
			if(!readFirstLine())
			{
				return false;
			}
		}

		if(em_headers == _em)
		{
			std::pair<size_t, SHeader> hdr;

			using namespace boost::spirit::qi;
			namespace qi = boost::spirit::qi;
			namespace px = boost::phoenix;

			rule<InputMessage::Iterator> parser =
				raw[
				    *(char_ - ':')
				][px::ref(hdr.second._name_) = qi::_1] >>

				':' >> *space
			;

			Iterator begin(_readedPos);

			for(;;)
			{
				hdr.second._header_ = Segment(_readedPos, _bufferAccumuler->end());
				if(!readUntil("\r\n", hdr.second._header_))
				{
					return false;
				}
				_readedPos = hdr.second._header_.end() + 2;

				if(hdr.second._header_.empty())
				{
					break;
				}

				Iterator piter = hdr.second._header_.begin();
				if(!parse(piter, hdr.second._header_.end(), parser))
				{
					return false;
				}
				hdr.second._value_ = Segment(piter, hdr.second._header_.end());
				hdr.first = http::hn::key(hdr.second._name_);
				_headersMap.insert(hdr);
			}

			begin--; begin++;
			_headers = Segment(begin, _readedPos-2);
			_em = em_body;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readBody()
	{
		assert(!"not impl");

		if(em_body > _em)
		{
			if(!readHeaders())
			{
				return false;
			}
		}

		if(em_body == _em)
		{
			assert(!"not impl");
			/*if(!readUntil("\r\n"))
			{
				return false;
			}*/
			_em = em_done;
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::ignoreBody()
	{
		assert(0);
		return false;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::firstLine() const
	{
		return _firstLine;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::headers() const
	{
		return _headers;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const HeaderName &name) const
	{
		return header(name.key);
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(size_t key) const
	{
		TMHeaders::const_iterator iter = _headersMap.find(key);
		if(_headersMap.end() == iter)
		{
			return NULL;
		}

		return &iter->second._value_;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const std::string &name) const
	{
		return header(http::hn::key(name));
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *name, size_t nameSize) const
	{
		return header(http::hn::key(name, nameSize));
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *namez) const
	{
		return header(http::hn::key(namez));
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::body() const
	{
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void InputMessage::reinit()
	{
		_em = em_firstLine;
		_bufferAccumuler->dropFront(_readedPos);
		_contentFilter = _bufferAccumuler;
		_readedPos = Iterator();
		_firstLine = Segment();
		_headers = Segment();
		_body = Segment();
		_headersMap.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readBuffer(Segment *segment)
	{
		InputMessageBuffer 	*lastBuffer = _bufferAccumuler->lastBuffer();

		do
		{
			async::Future2<boost::system::error_code, net::Packet> res =
				_channel.receive(_granula);

			res.wait();

			if(res.data1NoWait())
			{
				return false;
			}

			if(!_contentFilter->filterPush(res.data2NoWait(), 0))
			{
				return false;
			}
		}
		while(lastBuffer == _bufferAccumuler->lastBuffer());

		if(lastBuffer)
		{
			assert(lastBuffer->next());
			if(segment)
			{
				*segment = Segment(
					Iterator(lastBuffer->next(), lastBuffer->next()->begin()),
					Iterator(_bufferAccumuler->lastBuffer(), _bufferAccumuler->lastBuffer()->end()));
			}
		}
		else
		{
			if(segment)
			{
				*segment = Segment(
					Iterator(_bufferAccumuler->firstBuffer(), _bufferAccumuler->firstBuffer()->begin()),
					Iterator(_bufferAccumuler->lastBuffer(), _bufferAccumuler->lastBuffer()->end()));
			}
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	namespace
	{
		class TokFinder
		{
			const char	*_tokenz;
			size_t		_tokenPos;
			InputMessage::Iterator _begin;
		public:
			TokFinder(const char *tokenz)
				: _tokenz(tokenz)
				, _tokenPos(0)
			{
			}
			bool find(InputMessage::Segment &segment)
			{
				InputMessage::Iterator iter(segment.begin());
				InputMessage::Iterator end(segment.end());
				for(; iter!=end; ++iter)
				{
					if(_tokenz[_tokenPos] == *iter)
					{
						if(!_tokenPos)
						{
							_begin = iter;
						}
						_tokenPos++;
						if(!_tokenz[_tokenPos])
						{
							segment = InputMessage::Segment(_begin, ++iter);
							return true;
						}
					}
					else
					{
						_tokenPos = 0;
					}
				}
				segment = InputMessage::Segment(segment.end(), segment.end());
				return false;
			}
		};
	}

	//////////////////////////////////////////////////////////////////////////
	bool InputMessage::readUntil(const char *tokenz, Segment &segment)
	{
		TokFinder tf(tokenz);

		if(!segment.empty())
		{
			Segment lseg(segment);
			if(tf.find(lseg))
			{
				segment = Segment(segment.begin(), lseg.begin());
				return true;
			}
		}

		for(;;)
		{
			Segment lseg;
			if(!readBuffer(&lseg))
			{
				return false;
			}
			if(segment.empty())
			{
				segment = lseg;
			}
			if(tf.find(lseg))
			{
				segment = Segment(segment.begin(), lseg.begin());
				return true;
			}
		}

		assert(!"never here");
		return true;
	}

}}

#include "pch.hpp"
#include "http/impl/inputMessage.hpp"
#include "http/log.hpp"

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

			TLOG("firstLine: _" <<std::string(_firstLine.begin(), _firstLine.end())<<"_");
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
			//TODO: заголовки брать поштучно и распихивать в _headersMap
			_headers = Segment(
				_firstLine.end()+2,
				Iterator(
					_bufferAccumuler->lastBuffer(),
					_bufferAccumuler->lastBuffer()->end()
				)
			);

			TLOG("headers pre: _" <<std::string(_headers.begin(), _headers.end())<<"_");
			if(!readUntil("\r\n\r\n", _headers))
			{
				return false;
			}
			_headers = Segment(_headers.begin(), _headers.end()+2);
			TLOG("headers: _" <<std::string(_headers.begin(), _headers.end())<<"_");
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
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment &InputMessage::headers() const
	{
		assert(0);
		return *(const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const HeaderName &name) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(size_t key) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const std::string &name) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *name, size_t nameSize) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
	}

	//////////////////////////////////////////////////////////////////////////
	const InputMessage::Segment *InputMessage::header(const char *namez) const
	{
		assert(0);
		return (const InputMessage::Segment *)NULL;
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
		assert(0);
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
			size_t		_pos;
			InputMessage::Iterator _iter;
		public:
			TokFinder(const char *tokenz)
				: _tokenz(tokenz)
				, _pos(0)
			{
			}
			bool find(InputMessage::Segment &segment)
			{
				InputMessage::Iterator iter(segment.begin());
				InputMessage::Iterator end(segment.end());
				for(; iter!=end; ++iter)
				{
					if(_tokenz[_pos] == *iter)
					{
						if(!_pos)
						{
							_iter = iter;
						}
						_pos++;
						if(!_tokenz[_pos])
						{
							segment = InputMessage::Segment(_iter, segment.end());
							return true;
						}
					}
					else
					{
						_pos = 0;
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

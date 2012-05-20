#include "pch.hpp"
#include "http/impl/inputMessage.hpp"
#include "http/log.hpp"
#include "http/method.hpp"
#include "http/headerName.hpp"
#include "http/headerValue.hpp"
#include "http/error.hpp"

#include "http/impl/contentDecoderChunked.hpp"

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
		, _accumuler(new ContentDecoderAccumuler)
		, _contentDecoder(_accumuler)
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
	boost::system::error_code InputMessage::readFirstLine()
	{
		if(em_firstLine == _em)
		{
			boost::system::error_code ec;
			if((ec = readUntil("\r\n", _firstLine)))
			{
				return ec;
			}

			_readedPos = _firstLine.end() + 2;
			_em = em_headers;
		}
		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readHeaders()
	{
		boost::system::error_code ec;
		if(em_headers > _em)
		{
			if((ec = readFirstLine()))
			{
				return ec;
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
				hdr.second._header_ = Segment(_readedPos, _accumuler->end());
				if((ec = readUntil("\r\n", hdr.second._header_)))
				{
					return ec;
				}
				_readedPos = hdr.second._header_.end() + 2;

				if(hdr.second._header_.empty())
				{
					break;
				}

				Iterator piter = hdr.second._header_.begin();
				if(!parse(piter, hdr.second._header_.end(), parser))
				{
					return http::error::make(http::error::invalid_message);
				}
				hdr.second._value_ = Segment(piter, hdr.second._header_.end());
				hdr.first = http::hn::key(hdr.second._name_);
				_headersMap.insert(hdr);
			}

			begin--; begin++;
			_headers = Segment(begin, _readedPos-2);
			_em = em_body;
		}

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readBody()
	{
		boost::system::error_code ec;
		if(em_body > _em)
		{
			if((ec = readHeaders()))
			{
				return ec;
			}
		}

		if(em_body == _em)
		{
			bool readed = false;

			BodyExtractorPtr bodyExtractor;
			if((ec = prepareBodyExtractor(bodyExtractor)))
			{
				return ec;
			}
			assert(bodyExtractor);

			///////////////////////////////
			//сначала пихнуть что уже начитано в _accumuler
			if((ec = bodyExtractor->read(_accumuler, _readedPos)))
			{
				return ec;
			}

			///////////////////////////////
			//вырезать это из _accumuler
			_accumuler->dropTail(_readedPos);

			///////////////////////////////
			//теперь вычитывать из канала
			if((ec = bodyExtractor->read(_channel, _granula)))
			{
				return ec;
			}

			///////////////////////////////
			//вылить хвост обратно в _accumuler
			bodyExtractor->flush(_accumuler);

			///////////////////////////////
			//нормализовать позицию чтения, ато после нее дыра теперь
			_readedPos.normalize();

			_body = Segment(_accumulerBody->begin(), _accumulerBody->end());

			_em = em_done;
		}

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::ignoreBody()
	{
		assert(0);
		return http::error::make(http::error::not_implemented);
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
		return _body;
	}
	
	//////////////////////////////////////////////////////////////////////////
	void InputMessage::reinit()
	{
		_version = Version();
		_em = em_firstLine;
		_accumuler->dropFront(_readedPos);
		_contentDecoder = _accumuler;
		_readedPos = Iterator();
		_firstLine = Segment();
		_headers = Segment();
		_body = Segment();
		_headersMap.clear();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readBuffer(Segment *segment)
	{
		InputMessageBuffer 	*lastBuffer = _accumuler->lastBuffer();

		boost::system::error_code ec;
		do
		{
			async::Future2<boost::system::error_code, net::Packet> res =
				_channel.receive(_granula);

			res.wait();

			if(res.data1NoWait())
			{
				return res.data1NoWait();
			}

			if((ec = _contentDecoder->decoderPush(res.data2NoWait(), 0)))
			{
				return ec;
			}
		}
		while(lastBuffer == _accumuler->lastBuffer());

		if(lastBuffer)
		{
			assert(lastBuffer->next());
			if(segment)
			{
				*segment = Segment(
					Iterator(lastBuffer->next(), lastBuffer->next()->begin()),
					Iterator(_accumuler->lastBuffer(), _accumuler->lastBuffer()->end()));
			}
		}
		else
		{
			if(segment)
			{
				*segment = Segment(
					Iterator(_accumuler->firstBuffer(), _accumuler->firstBuffer()->begin()),
					Iterator(_accumuler->lastBuffer(), _accumuler->lastBuffer()->end()));
			}
		}

		return http::error::make();
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
	boost::system::error_code InputMessage::readUntil(const char *tokenz, Segment &segment)
	{
		TokFinder tf(tokenz);

		if(!segment.empty())
		{
			Segment lseg(segment);
			if(tf.find(lseg))
			{
				segment = Segment(segment.begin().normalize(), lseg.begin());
				return http::error::make();
			}
		}

		boost::system::error_code ec;
		for(;;)
		{
			Segment lseg;
			if((ec = readBuffer(&lseg)))
			{
				return ec;
			}
			if(segment.empty())
			{
				segment = lseg;
			}
			if(tf.find(lseg))
			{
				segment = Segment(segment.begin().normalize(), lseg.begin());
				return http::error::make();
			}
		}

		assert(!"never here");
		return http::error::make(http::error::unexpected);
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::prepareBodyExtractor(BodyExtractorPtr &result)
	{
		ContentDecoderPtr bodyDecoder = _accumulerBody;

		HeaderValue<ContentEncoding> hvContentEncoding(header(http::hn::contentEncoding));
		if(hvContentEncoding.isCorrect())
		{
			if(hvContentEncoding.value() & ece_deflate)
			{
				bodyDecoder.reset(new ContentEncoderZlib(bodyDecoder, ece_deflate));
			}
			else if(hvContentEncoding.value() & ece_gzip)
			{
				bodyDecoder.reset(new ContentEncoderZlib(bodyDecoder, ece_gzip));
			}
			else if(hvContentEncoding.value() == ece_identity)
			{
				//nothing
			}
			else
			{
				return http::error::make(http::error::invalid_message);
			}
		}
		else
		{
			//no content encoding
		}

		HeaderValue<Unsigned> hvContentLength(header(http::hn::contentLength));
		if(hvContentLength.isCorrect())
		{
			result.reset(new BodyExtractorSized(bodyDecoder));
			return http::error::make();
		}

		HeaderValue<TransferEncoding> hvTransferEncoding(header(http::hn::transferEncoding));
		if(hvTransferEncoding.isCorrect() && (hvTransferEncoding.value()&ete_chunked))
		{
			result.reset(new BodyExtractorChunked(bodyDecoder));
			return http::error::make();
		}


		HeaderValue<Connection> hvConnection(header(http::hn::connection));

		if(
			(hvConnection.isCorrect() && hvConnection.value()==ec_close) ||
			(!hvConnection.isCorrect() && _version < Version(1,1)))
		{
			if(hvConnection.value()==ec_close)
			{
				result.reset(new BodyExtractorUntilClose(bodyDecoder));
				return http::error::make();
			}
		}

		return http::error::make(http::error::invalid_message);
	}

	/*
	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readBodySized(size_t size)
	{
		size_t alreadyReaded = _accumuler->end() - _readedPos;
		if(alreadyReaded >= size)
		{
			_body = Segment(_readedPos, _readedPos + size);
			_readedPos = _body.end();

			return http::error::make();
		}
		size_t toRead = size - alreadyReaded;

		boost::system::error_code ec;
		while(toRead)
		{
			async::Future2<boost::system::error_code, net::Packet> res =
				_channel.receive(_granula);
			res.wait();
			if(res.data1NoWait())
			{
				return res.data1NoWait();
			}

			net::Packet &p = res.data2NoWait();
			if((ec = _contentDecoder->decoderPush(p, 0)))
			{
				return ec;
			}

			if(p._size >= toRead)
			{
				toRead = 0;
				break;
			}
			else
			{
				toRead -= p._size;
			}
		}
		_body = Segment(_readedPos, _readedPos + size);
		_readedPos = _body.end();

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readBodyChunked()
	{
		boost::system::error_code ec;
		ContentDecoderChunked cdc(_accumuler);

		///////////////////////////////
		{
			InputMessageBuffer* buf = _readedPos.buffer();
			assert(buf);

			const char *pos = _readedPos.position();
			assert(pos >= buf->begin() && pos <= buf->end());

			if(pos == buf->end())
			{
				buf = buf->next();
				if(buf)
				{
					pos = buf->begin();
				}
			}

			while(buf)
			{
				size_t offset;
				net::Packet p = buf->asPacket(offset);

				offset += pos - buf->begin();
				assert(offset < p._size);

				if((ec = cdc.decoderPush(p, offset)))
				{
					return ec;
				}

				buf = buf->next();
				if(buf)
				{
					pos = buf->begin();
				}
			}
		}

		///////////////////////////////
		{
			while(!cdc.isDone())
			{
				async::Future2<boost::system::error_code, net::Packet> res =
					_channel.receive(_granula);
				res.wait();
				if(res.data1NoWait())
				{
					return res.data1NoWait();
				}
				if((ec = cdc.decoderPush(res.data2NoWait(), 0)))
				{
					return ec;
				}
			}
		}

		_accumuler->dropTail(_readedPos);
		if((ec = cdc.decoderFlush()))
		{
			return ec;
		}

		_readedPos.normalize();
		_body = Segment(_readedPos, _readedPos + cdc.contentLength());
		_readedPos += _body.end();

		return http::error::make();
	}

	//////////////////////////////////////////////////////////////////////////
	boost::system::error_code InputMessage::readBodyAll()
	{
		size_t readed = _accumuler->end() - _readedPos;

		boost::system::error_code ec;
		for(;;)
		{
			async::Future2<boost::system::error_code, net::Packet> res =
				_channel.receive(_granula);
			res.wait();
			if(res.data1NoWait())
			{
				break;
			}

			net::Packet &p = res.data2NoWait();
			if((ec = _contentDecoder->decoderPush(p, 0)))
			{
				return ec;
			}

			readed -= p._size;
		}
		_body = Segment(_readedPos.normalize(), _readedPos + readed);
		_readedPos = _body.end();

		return http::error::make();
	}*/

}}

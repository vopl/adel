#include "pch.hpp"
#include "http/impl/inputMessage.hpp"
#include "http/log.hpp"
#include "http/method.hpp"
#include "http/headerName.hpp"
#include "http/headerValue.hpp"
#include "http/error.hpp"

#include "http/impl/bodyExtractorChunked.hpp"
#include "http/impl/bodyExtractorSized.hpp"
#include "http/impl/bodyExtractorUntilClose.hpp"

#include "http/impl/contentDecoderZlib.hpp"

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
			//сначала пихнуть что уже начитано в _accumuler и вырезать оттуда
			if((ec = bodyExtractor->read(_accumuler, _readedPos)))
			{
				return ec;
			}

			///////////////////////////////
			//теперь вычитывать из канала
			if((ec = bodyExtractor->read(_channel, _granula)))
			{
				return ec;
			}

			///////////////////////////////
			//вылить хвост обратно в _accumuler
			if((ec = bodyExtractor->flush()))
			{
				return ec;
			}

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
		_accumulerBody.reset();
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
		async::Future2<boost::system::error_code, net::Packet> res =
			_channel.receive(_granula);

		res.wait();

		if(res.data1NoWait())
		{
			return res.data1NoWait();
		}

		if((ec = _accumuler->push(res.data2NoWait(), 0)))
		{
			return ec;
		}

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
		_accumulerBody.reset(new ContentDecoderAccumuler());
		ContentDecoderPtr bodyDecoder = _accumulerBody;

		HeaderValue<ContentEncoding> hvContentEncoding(header(http::hn::contentEncoding));
		if(hvContentEncoding.isCorrect())
		{
			if(hvContentEncoding.value() & ece_deflate)
			{
				bodyDecoder.reset(new ContentDecoderZlib(bodyDecoder, ece_deflate));
			}
			else if(hvContentEncoding.value() & ece_gzip)
			{
				bodyDecoder.reset(new ContentDecoderZlib(bodyDecoder, ece_gzip));
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
			result.reset(new BodyExtractorSized(bodyDecoder, _accumuler, hvContentLength.value()));
			return http::error::make();
		}

		HeaderValue<TransferEncoding> hvTransferEncoding(header(http::hn::transferEncoding));
		if(hvTransferEncoding.isCorrect() && (hvTransferEncoding.value()&ete_chunked))
		{
			result.reset(new BodyExtractorChunked(bodyDecoder, _accumuler));
			return http::error::make();
		}


		HeaderValue<Connection> hvConnection(header(http::hn::connection));

		if(
			(hvConnection.isCorrect() && hvConnection.value()==ec_close) ||
			(!hvConnection.isCorrect() && _version < Version(1,1)))
		{
			if(hvConnection.value()==ec_close)
			{
				result.reset(new BodyExtractorUntilClose(bodyDecoder, _accumuler));
				return http::error::make();
			}
		}

		return http::error::make(http::error::invalid_message);
	}

}}

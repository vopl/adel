#ifndef _HTTP_IMPL_INPUTMESSAGE_HPP_
#define _HTTP_IMPL_INPUTMESSAGE_HPP_

#include "net/channel.hpp"
#include "http/inputMessage.hpp"
#include "http/impl/contentFilterBufferAccumuler.hpp"
#include <boost/enable_shared_from_this.hpp>
#include <boost/unordered_map.hpp>

namespace http { namespace impl
{
	///////////////////////////////////////////////////////////////
	class InputMessage
		: public boost::enable_shared_from_this<InputMessage>
	{
	public:
		typedef http::InputMessage::Iterator Iterator;
		typedef http::InputMessage::Segment Segment;

	public:
		InputMessage(const net::Channel &channel, size_t granula);
		virtual ~InputMessage();

		bool isConnected() const;

		bool readFirstLine();
		bool readHeaders();
		bool readBody();
		bool ignoreBody();


		//requestLine, responseLine
		const Segment &firstLine() const;

		//headers
		const Segment &headers() const;
		const Segment *header(const HeaderName &name) const;
		const Segment *header(size_t key) const;
		const Segment *header(const std::string &name) const;
		const Segment *header(const char *name, size_t nameSize) const;
		const Segment *header(const char *namez) const;

		const Segment &body() const;

	public:
		void reinit();

	protected:
		net::Channel	_channel;
		size_t			_granula;

		enum EMode
		{
			em_firstLine=1,
			em_headers=2,
			em_body=3,
			em_done=4,
		} _em;

	protected:
		bool readBuffer(Segment *segment);
		ContentFilterBufferAccumulerPtr	_bufferAccumuler;
		ContentFilterPtr				_contentFilter;

		bool readUntil(const char *tokenz, Segment &segment);

	protected:
		Iterator _readedPos;
		Segment	_firstLine;
		Segment	_headers;
		Segment	_body;



		struct SHeader
		{
			Segment _header_;
			Segment _name_;
			Segment _value_;
		};
		typedef std::map<size_t, SHeader> TMHeaders;
		TMHeaders _headersMap;
	};

	typedef boost::shared_ptr<InputMessage> InputMessagePtr;

}}

#endif

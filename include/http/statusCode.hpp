#ifndef _HTTP_STATUSCODE_HPP_
#define _HTTP_STATUSCODE_HPP_







/*
		100 Continue
		101 Switching Protocols
		200 OK
		201 Created
		202 Accepted
		203 Non-Authoritative Information
		204 No Content
		205 Reset Content
		206 Partial Content
		300 Multiple Choices
		301 Moved Permanently
		302 Found
		303 See Other
		304 Not Modified
		305 Use Proxy
		307 Temporary Redirect
		400 Bad Request
		401 Unauthorized
		402 Payment Required
		403 Forbidden
		404 Not Found
		405 Method Not Allowed
		406 Not Acceptable
		407 Proxy Authentication Required
		408 Request Timeout
		409 Conflict
		410 Gone
		411 Length Required
		412 Precondition Failed
		413 Request Entity Too Large
		414 Request-URI Too Long
		415 Unsupported Media Type
		416 Requested Range Not Satisfiable
		417 Expectation Failed
		500 Internal Server Error
		501 Not Implemented
		502 Bad Gateway
		503 Service Unavailable
		504 Gateway Timeout
		505 HTTP Version Not Supported
*/



namespace http
{

	enum EStatusCode
	{
		esc_100=100,// Continue
		esc_101=101,// Switching Protocols
		esc_200=200,// OK
		esc_201=201,// Created
		esc_202=202,// Accepted
		esc_203=203,// Non-Authoritative Information
		esc_204=204,// No Content
		esc_205=205,// Reset Content
		esc_206=206,// Partial Content
		esc_300=300,// Multiple Choices
		esc_301=301,// Moved Permanently
		esc_302=302,// Found
		esc_303=303,// See Other
		esc_304=304,// Not Modified
		esc_305=305,// Use Proxy
		esc_307=306,// Temporary Redirect
		esc_400=400,// Bad Request
		esc_401=401,// Unauthorized
		esc_402=402,// Payment Required
		esc_403=403,// Forbidden
		esc_404=404,// Not Found
		esc_405=405,// Method Not Allowed
		esc_406=406,// Not Acceptable
		esc_407=407,// Proxy Authentication Required
		esc_408=408,// Request Timeout
		esc_409=409,// Conflict
		esc_410=410,// Gone
		esc_411=411,// Length Required
		esc_412=412,// Precondition Failed
		esc_413=413,// Request Entity Too Large
		esc_414=414,// Request-URI Too Long
		esc_415=415,// Unsupported Media Type
		esc_416=416,// Requested Range Not Satisfiable
		esc_417=417,// Expectation Failed
		esc_500=500,// Internal Server Error
		esc_501=501,// Not Implemented
		esc_502=502,// Bad Gateway
		esc_503=503,// Service Unavailable
		esc_504=504,// Gateway Timeout
		esc_505=505,// HTTP Version Not Supported
	};

	inline const char *reasonPhrase(const EStatusCode &statusCode)
	{
		switch(statusCode)
		{
		case esc_100: return "Continue";
		case esc_101: return "Switching Protocols";
		case esc_200: return "OK";
		case esc_201: return "Created";
		case esc_202: return "Accepted";
		case esc_203: return "Non-Authoritative Information";
		case esc_204: return "No Content";
		case esc_205: return "Reset Content";
		case esc_206: return "Partial Content";
		case esc_300: return "Multiple Choices";
		case esc_301: return "Moved Permanently";
		case esc_302: return "Found";
		case esc_303: return "See Other";
		case esc_304: return "Not Modified";
		case esc_305: return "Use Proxy";
		case esc_307: return "Temporary Redirect";
		case esc_400: return "Bad Request";
		case esc_401: return "Unauthorized";
		case esc_402: return "Payment Required";
		case esc_403: return "Forbidden";
		case esc_404: return "Not Found";
		case esc_405: return "Method Not Allowed";
		case esc_406: return "Not Acceptable";
		case esc_407: return "Proxy Authentication Required";
		case esc_408: return "Request Timeout";
		case esc_409: return "Conflict";
		case esc_410: return "Gone";
		case esc_411: return "Length Required";
		case esc_412: return "Precondition Failed";
		case esc_413: return "Request Entity Too Large";
		case esc_414: return "Request-URI Too Long";
		case esc_415: return "Unsupported Media Type";
		case esc_416: return "Requested Range Not Satisfiable";
		case esc_417: return "Expectation Failed";
		case esc_500: return "Internal Server Error";
		case esc_501: return "Not Implemented";
		case esc_502: return "Bad Gateway";
		case esc_503: return "Service Unavailable";
		case esc_504: return "Gateway Timeout";
		case esc_505: return "HTTP Version Not Supported";
		default:
			assert(0);
			return "Unknown";
		}

		assert(!"never here");
		return "Unknown";
	}

}
#endif

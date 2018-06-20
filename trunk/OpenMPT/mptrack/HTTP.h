/*
 * HTTP.h
 * ------
 * Purpose: Simple HTTP client interface.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

OPENMPT_NAMESPACE_BEGIN


struct URI
{
	mpt::ustring scheme;
	mpt::ustring username;
	mpt::ustring password;
	mpt::ustring host;
	mpt::ustring port;
	mpt::ustring path;
	mpt::ustring query;
	mpt::ustring fragment;
};

class bad_uri
	: public std::runtime_error
{
public:
	bad_uri(const std::string &msg)
		: std::runtime_error(msg)
	{
	}
};

URI ParseURI(mpt::ustring str);


namespace HTTP
{


class exception
	: public std::runtime_error
{
private:
	mpt::ustring message;
public:
	exception(const mpt::ustring &m);
	mpt::ustring GetMessage() const;
};


struct NativeHandle;


class Handle
{
private:
	std::unique_ptr<NativeHandle> handle;
public:
	Handle();
	Handle(const Handle &) = delete;
	Handle & operator=(const Handle &) = delete;
	explicit operator bool() const;
	bool operator!() const;
	Handle(NativeHandle h);
	Handle & operator=(NativeHandle h);
	operator NativeHandle ();
	~Handle();
};


class InternetSession
{
private:
	Handle internet;
public:
	InternetSession(mpt::ustring userAgent);
	operator NativeHandle ();
	template <typename TRequest>
	auto operator()(const TRequest &request) -> decltype(request(*this))
	{
		return request(*this);
	}
	template <typename TRequest>
	auto Request(const TRequest &request) -> decltype(request(*this))
	{
		return request(*this);
	}
};


enum class Protocol
{
	HTTP,
	HTTPS,
};


using Port = uint16;
constexpr Port PortDefault = 0;
constexpr Port PortHTTP = 80;
constexpr Port PortHTTPS = 443;


enum class Method
{
	Get,
	Head,
	Post,
	Put,
	Delete,
	Trace,
	Options,
	Connect,
	Patch,
};


using Query = std::vector<std::pair<mpt::ustring, mpt::ustring>>;


using AcceptMimeTypes = std::vector<std::string>;
static inline AcceptMimeTypes AcceptMimeTypesText()
{
	return {"text/*"};
}
static inline AcceptMimeTypes AcceptMimeTypesJSON()
{
	return {"application/json"};
}
static inline AcceptMimeTypes AcceptMimeTypesBinary()
{
	return {"application/octet-stream"};
}


using Headers = std::vector<std::pair<std::string, std::string>>;


enum Flags
{
	None    = 0x00u,
	NoCache = 0x01u,
};


struct Result
{
	uint64 Status;
	std::string Data;
};


struct Request
{
	Protocol protocol = Protocol::HTTPS;
	mpt::ustring host;
	Port port = PortDefault;
	mpt::ustring username;
	mpt::ustring password;
	Method method = Method::Get;
	mpt::ustring path = MPT_USTRING("/");
	Query query;
	mpt::ustring referrer;
	AcceptMimeTypes acceptMimeTypes;
	Flags flags = None;
	Headers headers;
	std::string dataMimeType;
	mpt::const_byte_span data;

	Request &SetURI(const URI &uri);
	Request &InsecureTLSDowngradeWindowsXP();

	Result operator()(InternetSession &internet) const;
};


Result SimpleGet(InternetSession &internet, Protocol protocol, const mpt::ustring &host, const mpt::ustring &path);


} // namespace HTTP


OPENMPT_NAMESPACE_END

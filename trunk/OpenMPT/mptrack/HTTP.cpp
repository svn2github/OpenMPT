/*
 * HTTP.cpp
 * --------
 * Purpose: Simple HTTP client interface.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "HTTP.h"
#include "../common/mptIO.h"
#include <WinInet.h>


OPENMPT_NAMESPACE_BEGIN


URI ParseURI(mpt::ustring str)
{
	URI uri;
	std::size_t scheme_delim_pos = str.find(':');
	if(scheme_delim_pos == mpt::ustring::npos)
	{
		throw bad_uri("no scheme delimiter");
	}
	if(scheme_delim_pos == 0)
	{
		throw bad_uri("no scheme");
	}
	uri.scheme = str.substr(0, scheme_delim_pos);
	str = str.substr(scheme_delim_pos + 1);
	if(str.substr(0, 2) == U_("//"))
	{
		str = str.substr(2);
		std::size_t authority_delim_pos = str.find_first_of(U_("/?#"));
		if(authority_delim_pos == mpt::ustring::npos)
		{
			throw bad_uri("no path");
		}
		mpt::ustring authority = str.substr(0, authority_delim_pos);
		std::size_t userinfo_delim_pos = authority.find(U_("@"));
		if(userinfo_delim_pos != mpt::ustring::npos)
		{
			mpt::ustring userinfo = authority.substr(0, userinfo_delim_pos);
			authority = authority.substr(userinfo_delim_pos + 1);
			std::size_t username_delim_pos = userinfo.find(U_(":"));
			uri.username = userinfo.substr(0, username_delim_pos);
			if(username_delim_pos != mpt::ustring::npos)
			{
				uri.password = userinfo.substr(username_delim_pos + 1);
			}
		}
		std::size_t beg_bracket_pos = authority.find(U_("["));
		std::size_t end_bracket_pos = authority.find(U_("]"));
		std::size_t port_delim_pos = authority.find_last_of(U_(":"));
		if(beg_bracket_pos != mpt::ustring::npos && end_bracket_pos != mpt::ustring::npos)
		{
			if(port_delim_pos != mpt::ustring::npos && port_delim_pos > end_bracket_pos)
			{
				uri.host = authority.substr(0, port_delim_pos);
				uri.port = authority.substr(port_delim_pos + 1);
			} else
			{
				uri.host = authority;
			}
		} else
		{
			uri.host = authority.substr(0, port_delim_pos);
			if(port_delim_pos != mpt::ustring::npos)
			{
				uri.port = authority.substr(port_delim_pos + 1);
			}
		}
		str = str.substr(authority_delim_pos);
	}
	std::size_t path_delim_pos = str.find_first_of(U_("?#"));
	uri.path = str.substr(0, path_delim_pos);
	if(uri.path.empty())
	{
		throw bad_uri("empty path");
	}
	str = str.substr(path_delim_pos + 1);
	if(path_delim_pos != mpt::ustring::npos)
	{
		std::size_t fragment_delim_pos = str.find(U_("#"));
		uri.query = str.substr(0, fragment_delim_pos);
		if(fragment_delim_pos != mpt::ustring::npos)
		{
			uri.fragment = str.substr(fragment_delim_pos + 1);
		}
	}
	return uri;
}


namespace HTTP
{


static mpt::ustring LastErrorMessage(DWORD errorCode)
{
	void *lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		GetModuleHandle(TEXT("wininet.dll")),
		errorCode,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL);
	mpt::ustring message = mpt::ToUnicode(mpt::winstring((LPTSTR)lpMsgBuf));
	LocalFree(lpMsgBuf);
	return message;
}


exception::exception(const mpt::ustring &m)
	: std::runtime_error(std::string("HTTP error: ") + mpt::ToCharset(mpt::CharsetASCII, m))
{
	message = m;
}

mpt::ustring exception::GetMessage() const
{
	return message;
}


class LastErrorException
	: public exception
{
public:
	LastErrorException()
		: exception(LastErrorMessage(GetLastError()))
	{
	}
};


struct NativeHandle
{
	HINTERNET native_handle;
	NativeHandle(HINTERNET h)
		: native_handle(h)
	{
	}
	operator HINTERNET() const
	{
		return native_handle;
	}
};


Handle::Handle()
	: handle(mpt::make_unique<NativeHandle>(HINTERNET(NULL)))
{
}

Handle::operator bool() const
{
	return handle->native_handle != HINTERNET(NULL);
}

bool Handle::operator!() const
{
	return handle->native_handle == HINTERNET(NULL);
}

Handle::Handle(NativeHandle h)
	: handle(mpt::make_unique<NativeHandle>(HINTERNET(NULL)))
{
	handle->native_handle = h.native_handle;
}

Handle & Handle::operator=(NativeHandle h)
{
	if(handle->native_handle)
	{
		InternetCloseHandle(handle->native_handle);
		handle->native_handle = HINTERNET(NULL);
	}
	handle->native_handle = h.native_handle;
	return *this;
}

Handle::operator NativeHandle ()
{
	return *handle;
}

Handle::~Handle()
{
	if(handle->native_handle)
	{
		InternetCloseHandle(handle->native_handle);
		handle->native_handle = HINTERNET(NULL);
	}
}


InternetSession::InternetSession(mpt::ustring userAgent)
{
	internet = NativeHandle(InternetOpen(mpt::ToWin(userAgent).c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0));
	if(!internet)
	{
		throw HTTP::LastErrorException();
	}
}

InternetSession::operator NativeHandle ()
{
	return internet;
}


static mpt::winstring Verb(Method method)
{
	mpt::winstring result;
	switch(method)
	{
	case Method::Get:
		result = _T("GET");
		break;
	case Method::Head:
		result = _T("HEAD");
		break;
	case Method::Post:
		result = _T("POST");
		break;
	case Method::Put:
		result = _T("PUT");
		break;
	case Method::Delete:
		result = _T("DELETE");
		break;
	case Method::Trace:
		result = _T("TRACE");
		break;
	case Method::Options:
		result = _T("OPTIONS");
		break;
	case Method::Connect:
		result = _T("CONNECT");
		break;
	case Method::Patch:
		result = _T("PATCH");
		break;
	}
	return result;
}

static bool IsCachable(Method method)
{
	return method == Method::Get || method == Method::Head;
}


namespace
{
class AcceptMimeTypesWrapper
{
private:
	std::vector<mpt::winstring> strings;
	std::vector<LPCTSTR> array;
public:
	AcceptMimeTypesWrapper(AcceptMimeTypes acceptMimeTypes)
	{
		for(const auto &mimeType : acceptMimeTypes)
		{
			strings.push_back(mpt::ToWin(mpt::CharsetASCII, mimeType));
		}
		array.resize(strings.size() + 1);
		for(std::size_t i = 0; i < strings.size(); ++i)
		{
			array[i] = strings[i].c_str();
		}
		array[strings.size()] = NULL;
	}
	operator LPCTSTR*()
	{
		return strings.empty() ? NULL : array.data();
	}
};
}


Result Request::operator()(InternetSession &internet) const
{
	Port actualPort = port;
	if(actualPort == PortDefault)
	{
		actualPort = (protocol != Protocol::HTTP) ? PortHTTPS : PortHTTP;
	}
	Handle connection = NativeHandle(InternetConnect(
		NativeHandle(internet),
		mpt::ToWin(host).c_str(),
		actualPort,
		!username.empty() ? mpt::ToWin(username).c_str() : NULL,
		!password.empty() ? mpt::ToWin(password).c_str() : NULL,
		INTERNET_SERVICE_HTTP,
		0,
		0));
	if(!connection)
	{
		throw HTTP::LastErrorException();
	}
	mpt::ustring queryPath = path;
	if(!query.empty())
	{
		std::vector<mpt::ustring> arguments;
		for(const auto &argument : query)
		{
			if(!argument.second.empty())
			{
				arguments.push_back(mpt::format(U_("%1=%2"))(argument.first, argument.second));
			} else
			{
				arguments.push_back(mpt::format(U_("%1"))(argument.first));
			}
		}
		queryPath += U_("?") + mpt::String::Combine(arguments, U_("&"));
	}
	Handle request = NativeHandle(HttpOpenRequest(
		NativeHandle(connection),
		Verb(method).c_str(),
		mpt::ToWin(path).c_str(),
		NULL,
		!referrer.empty() ? mpt::ToWin(referrer).c_str() : NULL,
		AcceptMimeTypesWrapper(acceptMimeTypes),
		0
			| ((protocol != Protocol::HTTP) ? (INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_REDIRECT_TO_HTTP) : 0)
			| (IsCachable(method) ? 0 : INTERNET_FLAG_DONT_CACHE)
			| ((flags & NoCache) ? (INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE) : 0)
		,
		NULL));
	if(!request)
	{
		throw HTTP::LastErrorException();
	}
	{
		std::string headersString;
		if(!dataMimeType.empty())
		{
			headersString += mpt::format("Content-type: %1\r\n")(dataMimeType);
		}
		if(!headers.empty())
		{
			for(const auto &header : headers)
			{
				headersString += mpt::format("%1: %2\r\n")(header.first, header.second);
			}
		}
		if(HttpSendRequest(
			NativeHandle(request),
			!headersString.empty() ? mpt::ToWin(mpt::CharsetASCII, headersString).c_str() : NULL,
			!headersString.empty() ? mpt::saturate_cast<DWORD>(mpt::ToWin(mpt::CharsetASCII, headersString).length()) : 0,
			!data.empty() ? (LPVOID)data.data() : NULL,
			!data.empty() ? mpt::saturate_cast<DWORD>(data.size()) : 0)
			== FALSE)
		{
			throw HTTP::LastErrorException();
		}
	}
	Result result;
	{
		DWORD statusCode = 0;
		DWORD length = sizeof(statusCode);
		if(HttpQueryInfo(NativeHandle(request), HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &length, NULL) == FALSE)
		{
			throw HTTP::LastErrorException();
		}
		result.Status = statusCode;
	}
	{
		std::string resultBuffer;
		DWORD bytesRead = 0;
		do
		{
			char downloadBuffer[mpt::IO::BUFFERSIZE_TINY];
			DWORD availableSize = 0;
			if(InternetQueryDataAvailable(NativeHandle(request), &availableSize, 0, NULL) == FALSE)
			{
				throw HTTP::LastErrorException();
			}
			availableSize = mpt::clamp(availableSize, DWORD(0), mpt::saturate_cast<DWORD>(mpt::IO::BUFFERSIZE_TINY));
			if(InternetReadFile(NativeHandle(request), downloadBuffer, availableSize, &bytesRead) == FALSE)
			{
				throw HTTP::LastErrorException();
			}
			resultBuffer.append(downloadBuffer, downloadBuffer + bytesRead);
		} while(bytesRead != 0);
		result.Data = std::move(resultBuffer);
	}
	return result;		
}


Request &Request::SetURI(const URI &uri)
{
	if(uri.scheme == U_(""))
	{
		throw bad_uri("no scheme");
	} else if(uri.scheme == U_("http"))
	{
		protocol = HTTP::Protocol::HTTP;
	} else if(uri.scheme == U_("https"))
	{
		protocol = HTTP::Protocol::HTTPS;
	} else
	{
		throw bad_uri("wrong scheme");		
	}
	host = uri.host;
	if(!uri.port.empty())
	{
		port = ConvertStrTo<HTTP::Port>(uri.port);
	} else
	{
		port = HTTP::PortDefault;
	}
	username = uri.username;
	password = uri.password;
	if(uri.path.empty())
	{
		path = U_("/");
	} else
	{
		path = uri.path;
	}
	query.clear();
	auto keyvals = mpt::String::Split<mpt::ustring>(uri.query, U_("&"));
	for(const auto &keyval : keyvals)
	{
		std::size_t delim_pos = keyval.find(U_("="));
		mpt::ustring key = keyval.substr(0, delim_pos);
		mpt::ustring val;
		if(delim_pos != mpt::ustring::npos)
		{
			val = keyval.substr(delim_pos + 1);
		}
		query.push_back(std::make_pair(key, val));
	}
	// ignore fragment
	return *this;
}


Request &Request::InsecureTLSDowngradeWindowsXP()
{
	if(mpt::Windows::IsOriginal() && mpt::Windows::Version::Current().IsBefore(mpt::Windows::Version::WinVista))
	{
		// TLS 1.0 is not enabled by default until IE7. Since WinInet won't let us override this setting, we cannot assume that HTTPS
		// is going to work on older systems. Besides... Windows XP is already enough of a security risk by itself. :P
		if(protocol == Protocol::HTTPS)
		{
			protocol = Protocol::HTTP;
		}
		if(port == PortHTTPS)
		{
			port = PortHTTP;
		}
	}
	return *this;
}


Result SimpleGet(InternetSession &internet, Protocol protocol, const mpt::ustring &host, const mpt::ustring &path)
{
	HTTP::Request request;
	request.protocol = protocol;
	request.host = host;
	request.method = HTTP::Method::Get;
	request.path = path;
	return internet(request);
}


} // namespace HTTP


OPENMPT_NAMESPACE_END

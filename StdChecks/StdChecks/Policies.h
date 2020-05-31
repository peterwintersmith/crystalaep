// Copyright (c) Peter Winter-Smith [peterwintersmith@gmail.com]

#pragma once

namespace UrlFilteringPolicies
{
	extern "C" __declspec(dllexport) EAllowableOperation CheckInvalidUrlEncoding(IConnectionPool *pConnPool, IConnection *pConn);
}

namespace SSLFilteringPolicies
{
}

namespace RequestFilteringPolicies
{
}

namespace ResponseFilteringPolicies
{
	// media content filtering
	extern "C" EAllowableOperation IsStreamingProtocolFormat(IConnectionPool *pConnPool, IConnection *pConn);
	extern "C" __declspec(dllexport) EAllowableOperation CheckPngFileValidity(IConnectionPool *pConnPool, IConnection *pConn);
	extern "C" __declspec(dllexport) EAllowableOperation CheckJpegJFIFValidity(IConnectionPool *pConnPool, IConnection *pConn);
	extern "C" __declspec(dllexport) EAllowableOperation CheckGifFileValidity(IConnectionPool *pConnPool, IConnection *pConn);

	// streaming media content filtering
	extern "C" __declspec(dllexport) EAllowableOperation CheckFlvFileValidityChunked(IConnectionPool *pConnPool, IConnection *pConn);

	// webapp response filtering
	extern "C" __declspec(dllexport) EAllowableOperation CheckCrossSiteScriptingUrlParams(IConnectionPool *pConnPool, IConnection *pConn);
	extern "C" __declspec(dllexport) EAllowableOperation CheckCrossSiteScriptingPostBody(IConnectionPool *pConnPool, IConnection *pConn);
}
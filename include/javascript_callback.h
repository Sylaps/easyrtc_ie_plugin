#ifndef PEERCONNECTION_JAVASCRIPT_CALLBACK_CLIENT_H_
#define PEERCONNECTION_JAVASCRIPT_CALLBACK_CLIENT_H_
#pragma once

#include "stdafx.h"
#include <string>

class JavaScriptCallback		// aka BrowserCallback
{
public:
	virtual void SendToBrowser(const std::string& json) = 0;

protected:
	virtual ~JavaScriptCallback() { }
};

#endif
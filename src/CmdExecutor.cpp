/*
 * CmdExecutor.cpp
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#include "CmdExecutor.h"

namespace gw {

CmdExecutor::CmdExecutor(const std::string & host,int port) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(5,0));
}

CmdExecutor::~CmdExecutor() {
	// TODO Auto-generated destructor stub
}

void CmdExecutor::run() {
}

} /* namespace gw */

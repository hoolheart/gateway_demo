/*
 * CmdFetcher.cpp
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#include "CmdFetcher.h"

namespace gw {

CmdFetcher::CmdFetcher(const std::string & host,int port) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(5,0));
}

CmdFetcher::~CmdFetcher() {
	// TODO Auto-generated destructor stub
}

void CmdFetcher::run() {
}

} /* namespace gw */

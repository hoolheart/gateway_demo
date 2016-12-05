/*
 * CmdFetcher.h
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#ifndef CMDFETCHER_H_
#define CMDFETCHER_H_

#include "Poco/Runnable.h"
#include "Poco/Net/HTTPClientSession.h"
#include <boost/scoped_ptr.hpp>
#include "Poco/Dynamic/Struct.h"

typedef boost::scoped_ptr<Poco::Net::HTTPClientSession> HTTPClientSession_ptr;

namespace gw {

class CmdFetcher : public Poco::Runnable {
private:
	HTTPClientSession_ptr session;

public:
	CmdFetcher(const std::string & host,int port);
	virtual ~CmdFetcher();

	//Runnable implementation
	void run();

private:
	//handle json data
	void handleJSON(Poco::Dynamic::Var &result);
};

} /* namespace gw */

#endif /* CMDFETCHER_H_ */

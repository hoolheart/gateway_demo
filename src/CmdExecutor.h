/*
 * CmdExecutor.h
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#ifndef CMDEXECUTOR_H_
#define CMDEXECUTOR_H_

#include "Poco/Runnable.h"
#include "Poco/Net/HTTPClientSession.h"
#include <boost/scoped_ptr.hpp>

typedef boost::scoped_ptr<Poco::Net::HTTPClientSession> HTTPClientSession_ptr;

namespace gw {

class CmdExecutor : public Poco::Runnable {
private:
	HTTPClientSession_ptr session;

public:
	CmdExecutor(const std::string & host,int port);
	virtual ~CmdExecutor();

	//Runnable implementation
	void run();
};

} /* namespace gw */

#endif /* CMDEXECUTOR_H_ */

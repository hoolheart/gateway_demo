/*
 * HTTPReporter.h
 *
 *  Created on: 2016年11月20日
 *      Author: hzhou
 */

#ifndef HTTPREPORTER_H_
#define HTTPREPORTER_H_

#include "Poco/Runnable.h"
#include "Poco/Net/HTTPClientSession.h"
#include <boost/scoped_ptr.hpp>

typedef boost::scoped_ptr<Poco::Net::HTTPClientSession> HTTPClientSession_ptr;

namespace gw {

class HTTPReporter : public Poco::Runnable {
private:
	HTTPClientSession_ptr session;

public:
	HTTPReporter(const std::string & host,int port);
	virtual ~HTTPReporter();

	//Runnable implementation
	void run();
};

} /* namespace gw */

#endif /* HTTPREPORTER_H_ */

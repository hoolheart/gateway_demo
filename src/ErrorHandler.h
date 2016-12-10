/*
 * ErrorHandler.h
 *
 *  Created on: 2016年12月10日
 *      Author: hzhou
 */

#ifndef ERRORHANDLER_H_
#define ERRORHANDLER_H_

#include "Poco/ErrorHandler.h"

namespace gw {

class ErrorHandler: public Poco::ErrorHandler {
public:
	ErrorHandler();
	virtual ~ErrorHandler();

	virtual void exception(const Poco::Exception& exc);
	virtual void exception(const std::exception& exc);
	virtual void exception();
};

} /* namespace gw */

#endif /* ERRORHANDLER_H_ */

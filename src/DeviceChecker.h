/*
 * DeviceChecker.h
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#ifndef DEVICECHECKER_H_
#define DEVICECHECKER_H_

#include "Poco/Runnable.h"
#include "CommInterface.h"

namespace gw {

class DeviceChecker: public Poco::Runnable, public CommErrHandler, public CommDataHandler {
public:
	explicit DeviceChecker();
	~DeviceChecker();

	//Runnable implementation
	void run();

	//CommErrHandler implementation
	void onCommError(unsigned char chl,COMM_ERROR err,std::string what);

	//CommDataHandler implementation
	void onCommData(unsigned char chl,COMM_DATA_FRAME &frame);
};

} /* namespace gw */

#endif /* DEVICECHECKER_H_ */

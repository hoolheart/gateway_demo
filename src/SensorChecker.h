/*
 * SensorChecker.h
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#ifndef SENSORCHECKER_H_
#define SENSORCHECKER_H_

#include "Poco/Runnable.h"
#include "CommInterface.h"

namespace gw {

class SensorChecker: public Poco::Runnable, public CommErrHandler, public CommDataHandler {
public:
	explicit SensorChecker();
	~SensorChecker();

	//Runnable implementation
	void run();

	//CommErrHandler implementation
	void onCommError(unsigned char chl,COMM_ERROR err,std::string what);

	//CommDataHandler implementation
	void onCommData(unsigned char chl,COMM_DATA_FRAME &frame);
};

} /* namespace gw */

#endif /* SENSORCHECKER_H_ */

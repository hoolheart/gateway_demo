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
#include "CommInterface.h"
#include "DataManage.h"

typedef boost::scoped_ptr<Poco::Net::HTTPClientSession> HTTPClientSession_ptr;

namespace gw {

class CmdExecutor : public Poco::Runnable, public CommDataHandler {
private:
	HTTPClientSession_ptr session;
	TASK task;
	Controller_ptr dev;
	IComm_ptr comm;
	CONTROLLER_CMD cmd;
	bool received, result;

public:
	CmdExecutor(const std::string & host,int port);
	virtual ~CmdExecutor();

	//set next task
	void setTask(TASK _task) {task = _task;}

	//Runnable implementation
	void run();

	//CommDataHandler implementation
	void onCommData(unsigned char chl,COMM_DATA_FRAME &frame);
};

} /* namespace gw */

#endif /* CMDEXECUTOR_H_ */

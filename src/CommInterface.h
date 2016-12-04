/*
 * CommInterface.h
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#ifndef COMMINTERFACE_H_
#define COMMINTERFACE_H_
#include <string>
#include "Poco/Thread.h"
#include "Poco/Runnable.h"
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace gw {

struct COMM_DATA_FRAME {
	unsigned char type;
	unsigned char addr;
	unsigned char data[8];
};

enum CHANNEL_STATUS {
	CHANNEL_CLOSED = 1,
	CHANNEL_OPEN,
	CHANNEL_BUSY,
	CHANNEL_ERROR
};

enum COMM_ERROR {
	CHL_FAILED_OPEN,
	CHL_FAILED_SEND,
	CHL_ERROR,
	CHL_ACCIDENT_CLOSE
};

class CommErrHandler {
public:
	virtual ~CommErrHandler() {}

	virtual void onCommError(unsigned char chl,COMM_ERROR err,std::string what)=0;
};

class CommDataHandler {
public:
	virtual ~CommDataHandler() {}

	virtual void onCommData(unsigned char chl,COMM_DATA_FRAME &frame)=0;
};

/** interface of communicate channel to the sensors */
class IComm {
public:
	virtual ~IComm() {}

	virtual unsigned char getChannel() const=0;/**< get channel index */
	virtual CHANNEL_STATUS getStatus() const=0;/**< get channel status */
	virtual void openChannel()=0;/**< open channel */
	virtual void closeChannel()=0;/**< close channel */
	virtual void sendData(COMM_DATA_FRAME &frame)=0;/**< send data */
	virtual void setCommErrHandler(CommErrHandler* handler)=0;/**< set error handler */
	virtual void setCommDataHandler(CommDataHandler* handler)=0;/**< set data handler */
};

typedef boost::shared_ptr<IComm> IComm_ptr;

class SerialComm : public Poco::Runnable, public IComm {

private:
	unsigned char chl;
	int file;
	int baudRate;
	CHANNEL_STATUS status;
	enum {HEADER1,HEADER2,TYPE,ADDR,DATA} step;
	COMM_DATA_FRAME rxBuff;
	char txBuff[12];
	int dataIndex;
	boost::scoped_ptr<Poco::Thread> thd;
	bool runSignal;
	CommErrHandler* errHandler;
	CommDataHandler* dataHandler;

public:
	explicit SerialComm(unsigned char _chl,int _baudRate);
	~SerialComm();

	// implementation of Runnable
	void run();/**< running process to handle received data */

	//implementation of IComm
	virtual unsigned char getChannel() const;/**< get channel index */
	virtual CHANNEL_STATUS getStatus() const;/**< get channel status */
	virtual void openChannel();/**< open channel */
	virtual void closeChannel();/**< close channel */
	virtual void sendData(COMM_DATA_FRAME &frame);/**< send data */
	virtual void setCommErrHandler(CommErrHandler* handler);/**< set error handler */
	virtual void setCommDataHandler(CommDataHandler* handler);/**< set data handler */

private:
	int map_speed_to_unix(int speed);
};

} /* namespace gw */

#endif /* COMMINTERFACE_H_ */

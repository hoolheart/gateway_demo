/*
 * CommInterface.cpp
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#include "CommInterface.h"
#include <unistd.h>
#include <termios.h>
#include <sstream>
#include <fcntl.h>
#include <iostream>
#include <string.h>

namespace gw {

SerialComm::SerialComm(unsigned char _chl,int _baudRate):
	chl(_chl),file(0),baudRate(_baudRate),status(CHANNEL_CLOSED),step(HEADER1),dataIndex(0),runSignal(false),errHandler(0),dataHandler(0) {
	thd.reset(new Poco::Thread());
	txBuff[0] = 0x5a; txBuff[1] = 0x5a;
}

SerialComm::~SerialComm() {
	closeChannel();
}

void SerialComm::run() {
	unsigned char byte,cs;
	int cnt;
	while(runSignal) {
		//read data
		cnt = read(file,&byte,1);
		if(cnt<=0) {
			usleep(5*1000);
		}
		else {
			//std::cout<<"[UART] Receive data: 0x"<<std::hex<<(int)byte<<std::endl;
			//std::cout<<"[UART] STEP BEFORE PROCESS: "<<step<<std::endl;
			switch (step) {
			case HEADER1://header1
				if(byte==0x5a) {
					step = HEADER2;
				}
				break;
			case HEADER2://header2
				if(byte==0x5a) {
					step = TYPE;
				}
				else {
					step = HEADER1;
				}
				break;
			case TYPE://type
				rxBuff.type = byte;
				step = ADDR;
				break;
			case ADDR://address
				rxBuff.addr = byte;
				step = DATA;
				dataIndex = 0;
				break;
			case DATA://data
				rxBuff.data[dataIndex] = byte;//record data
				if(dataIndex<7) {
					dataIndex++;
				}
				else {
					//checksum
					cs = 0;
					for(int i=0;i<8;i++) {
						cs += rxBuff.data[i];
					}
					//std::cout<<"[UART] checksum: 0x"<<std::hex<<(int)cs<<std::endl;
					if(cs==0 && dataHandler) {
						//success
						dataHandler->onCommData(chl,rxBuff);
					}
					//return to header
					step = HEADER1;
				}
				break;
			default:
				step = HEADER1;
				break;
			}
			//std::cout<<"[UART] STEP AFTER PROCESS: "<<step<<std::endl;
		}
	}
}

unsigned char SerialComm::getChannel() const {
	return chl;
}

CHANNEL_STATUS SerialComm::getStatus() const {
	return status;
}

void SerialComm::openChannel() {
	//check status
	if(status==CHANNEL_CLOSED) {
		//prepare baud rate
		int baudFactor = map_speed_to_unix(baudRate);
		if (baudFactor < 0) {
			std::cout<<"Invalid baud rate: "<<baudRate<<std::endl;
			if(errHandler) {
				errHandler->onCommError(chl,CHL_FAILED_OPEN,std::string("Invalid baud rate"));
			}
			return;
		}

		std::ostringstream fName; fName<<"/dev/ttyO"<<(unsigned int)chl;//prepare file name
		char device[64]; strcpy(device,fName.str().c_str());
		std::cout<<"Try to open file: "<<device<<std::endl;
		file = open(device, O_RDWR | O_NOCTTY | O_NDELAY);//try open
		if (file<0) {
			std::cout<<"Failed to open file: "<<fName.str()<<std::endl;
			if(errHandler) {
				errHandler->onCommError(chl,CHL_FAILED_OPEN,std::string("failed to open file")+fName.str());
			}
			return;
		}

		// no delay
		fcntl(file, F_SETFL, 0);

		//get file options
		struct termios opts;
		tcgetattr(file, &opts);            //Sets the parameters associated with file

		// Set up the communications options:
		opts.c_cflag &= ~CSIZE;
		opts.c_cflag &= ~CSTOPB;
		opts.c_cflag &= ~PARENB;
		opts.c_cflag |= CLOCAL | CREAD | CS8;
		opts.c_iflag = IGNPAR;
		opts.c_oflag = 0;
		opts.c_lflag = 0;
		opts.c_cc[VTIME] = 5;
		opts.c_cc[VMIN] = 0;
		cfsetispeed(&opts, baudFactor);
		cfsetospeed(&opts, baudFactor);

		//apply option
		tcflush(file, TCIFLUSH);
		tcsetattr(file, TCSANOW, &opts);

		//start receiving thread
		if(!thd->isRunning()) {
			runSignal = true;
			thd->start(*this);
		}

		//set status
		status = CHANNEL_OPEN;
		std::cout<<"Succeed to open file: "<<fName.str()<<std::endl;
	}
}

void SerialComm::closeChannel() {
	if(status!=CHANNEL_CLOSED) {//check status
		//stop thread
		runSignal = false;//clear flag
		thd->join();//wait to finish
		//close file
		close(file);
		file = 0;
		status = CHANNEL_CLOSED;
		std::cout<<"Succeed to close UART CHANNEL "<<(int)chl<<std::endl;
	}
}

void SerialComm::sendData(COMM_DATA_FRAME& frame) {
	if(status==CHANNEL_OPEN) {//check status
		//prepare buffer
		txBuff[2] = frame.type;
		txBuff[3] = frame.addr;
		unsigned char cs = 0;
		for(int i=0;i<7;i++) {
			txBuff[4+i] = frame.data[i];
			cs -= frame.data[i];
		}
		txBuff[11] = cs;
		//send
		int cnt = write(file,txBuff,12);
		if((cnt<0) && errHandler) {
			//error
			std::ostringstream err; err<<"failed to send data to address: "<<frame.addr;
			errHandler->onCommError(chl,CHL_FAILED_SEND,err.str());
		}
		else {
			std::cout<<"Succeed to send command by UART"<<std::endl;
		}
	}
}

void SerialComm::setCommErrHandler(CommErrHandler* handler) {
	errHandler = handler;
}

void SerialComm::setCommDataHandler(CommDataHandler* handler) {
	dataHandler = handler;
}

int SerialComm::map_speed_to_unix(int speed) {
	int unix_speed;

	switch (speed) {
	case 50: unix_speed = B50; break;
	case 75: unix_speed = B75; break;
	case 110: unix_speed = B110; break;
	case 134: unix_speed = B134; break;
	case 150: unix_speed = B150; break;
	case 300: unix_speed = B300; break;
	case 600: unix_speed = B600; break;
	case 1200: unix_speed = B1200; break;
	case 1800: unix_speed = B1800; break;
	case 2400: unix_speed = B2400; break;
	case 4800: unix_speed = B4800; break;
	case 9600: unix_speed = B9600; break;
	case 19200: unix_speed = B19200; break;
	case 38400: unix_speed = B38400; break;
	case 57600: unix_speed = B57600; break;
	case 115200: unix_speed = B115200; break;
	case 230400: unix_speed = B230400; break;
	case 460800: unix_speed = B460800; break;
	case 500000: unix_speed = B500000; break;
	case 576000: unix_speed = B576000; break;
	case 921600: unix_speed = B921600; break;
	case 1000000: unix_speed = B1000000; break;
	case 1152000: unix_speed = B1152000; break;
	case 1500000: unix_speed = B1500000; break;
	case 2000000: unix_speed = B2000000; break;
	case 2500000: unix_speed = B2500000; break;
	case 3000000: unix_speed = B3000000; break;
	case 3500000: unix_speed = B3500000; break;
	case 4000000: unix_speed = B4000000; break;
	default: unix_speed = -1; break;
	}

	return unix_speed;
}

} /* namespace gw */

/*
 * SensorChecker.cpp
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#include "SensorChecker.h"
#include "DataManage.h"
#include <iostream>
#include "protocol.h"

namespace gw {

SensorChecker::~SensorChecker() {
}

SensorChecker::SensorChecker() {
	// bind with communication channels
	// UART4
	IComm_ptr channel = DataManage::getInstance()->getChannel(4);
	channel->setCommErrHandler(this);
	channel->setCommDataHandler(this);
}

void SensorChecker::run() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//prepare data frame
	COMM_DATA_FRAME frame;
	frame.type = 0x0B;
	//devices on UART4
	IComm_ptr pChl = pDat->getChannel(4);//get channel
	std::list<Sensor_ptr> sensors = pDat->getSensorList(4);
	if(pChl && sensors.size()) {
		std::list<Sensor_ptr>::iterator i=sensors.begin();
		while(i!=sensors.end()) {
			Sensor_ptr pSensor = *i;//get sensor
			if(pSensor) {
				frame.addr = pSensor->getAddress();//set address
				pChl->sendData(frame);//send data frame
				std::cout<<"[SC] send command to check "<<pSensor->getName()<<std::endl;
				Poco::Thread::sleep(25);
			}
			i++;
		}
	}
}

void SensorChecker::onCommError(unsigned char chl, COMM_ERROR err,
		std::string what) {
	std::cout<<"[SC] ERROR ON CHANNEL "<<chl<<" with the code "<<(int)err<<std::endl;
	std::cout<<"[SC] ERROR CONTENT: "<<what<<std::endl;
}

void SensorChecker::onCommData(unsigned char chl, COMM_DATA_FRAME& frame) {
	//std::cout<<"[SC] handle frame chl: "<<(int)chl<<" type: 0x"<<std::hex<<(int)frame.type<<" addr:"<<(int)frame.addr<<std::endl;
	//check type
	if(frame.type==0xB0) {
		Sensor_ptr pSensor = DataManage::getInstance()->getSensor(chl,frame.addr);//get sensor
		if(pSensor) {
			SENSOR_REPORT *report = (SENSOR_REPORT*)frame.data;//apply protocol
			//std::cout<<"[SC] sensor name:"<<pSensor->getName()<<" status:"<<(int)report->status<<" value:"<<report->value<<std::endl;
			//check status
			if(report->status<=SENSOR_FAULT) {
				pSensor->setStatus((char)report->status);//record status
				if(report->status==SENSOR_OK || report->status==SENSOR_WARN) {
					pSensor->setValue(report->value);
				}
			}
		}
	}
}

} /* namespace gw */

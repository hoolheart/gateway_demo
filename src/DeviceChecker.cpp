/*
 * DeviceChecker.cpp
 *
 *  Created on: 2016年11月19日
 *      Author: hzhou
 */

#include "DataManage.h"
#include <iostream>

#include "DeviceChecker.h"
#include "protocol.h"

namespace gw {

DeviceChecker::DeviceChecker() {
}

DeviceChecker::~DeviceChecker() {
}

void DeviceChecker::run() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	std::list<int> chls = pDat->getChannels();//get all channels

	//prepare data frame
	COMM_DATA_FRAME frame;
	frame.type = 0x0B;
	//check devices
	for(std::list<int>::iterator iChl=chls.begin();iChl!=chls.end();iChl++) {
		IComm_ptr pChl = pDat->getChannel(*iChl);//get channel
		//check sensors
		std::list<Sensor_ptr> sensors = pDat->getSensors(*iChl);//get sensors
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
		//check controllers
		std::list<Controller_ptr> controllers = pDat->getControllers(*iChl);//get controllers
		if(pChl && controllers.size()) {
			std::list<Controller_ptr>::iterator i=controllers.begin();
			while(i!=controllers.end()) {
				Controller_ptr pController = *i;//get sensor
				if(pController) {
					frame.addr = pController->getAddress();//set address
					pChl->sendData(frame);//send data frame
					std::cout<<"[SC] send command to check "<<pController->getName()<<std::endl;
					Poco::Thread::sleep(25);
				}
				i++;
			}
		}
	}
}

void DeviceChecker::onCommError(unsigned char chl, COMM_ERROR err,
		std::string what) {
	std::cout<<"[SC] ERROR ON CHANNEL "<<chl<<" with the code "<<(int)err<<std::endl;
	std::cout<<"[SC] ERROR CONTENT: "<<what<<std::endl;
}

void DeviceChecker::onCommData(unsigned char chl, COMM_DATA_FRAME& frame) {
	//std::cout<<"[SC] handle frame chl: "<<(int)chl<<" type: 0x"<<std::hex<<(int)frame.type<<" addr:"<<(int)frame.addr<<std::endl;
	//check type
	if(frame.type==0xB0) {
		Sensor_ptr pSensor = DataManage::getInstance()->getSensor(chl,frame.addr);//get sensor
		Controller_ptr pController = DataManage::getInstance()->getController(chl,frame.addr);//get controller
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
		else if(pController) {
			CONTROLLER_REPORT *report = (CONTROLLER_REPORT*)frame.data;//apply protocol
			//check code
			if(report->code>0) {
				pController->setCurrentCmd(report->code,report->para);//record command
				pController->setStatus(DEVICE_OK);//status OK
			}
			else {
				pController->setStatus(DEVICE_FAULT);//status FAULT
			}
		}
	}
}

} /* namespace gw */

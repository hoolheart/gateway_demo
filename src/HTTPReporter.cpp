/*
 * HTTPReporter.cpp
 *
 *  Created on: 2016年11月20日
 *      Author: hzhou
 */

#include "HTTPReporter.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include <sstream>
#include "DataManage.h"
#include "Poco/DateTime.h"
#include "Poco/Timestamp.h"
#include "Poco/DateTimeFormatter.h"
#include "Poco/Timespan.h"
#include <iostream>

namespace gw {

HTTPReporter::HTTPReporter(const std::string & host,int port) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(1,0));
}

HTTPReporter::~HTTPReporter() {
	// TODO Auto-generated destructor stub
}

void HTTPReporter::run() {
	reportSensors();
	reportControllers();
}

void HTTPReporter::reportSensors() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//prepare request
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/sensorDetailData");
	request.setContentType("application/json");
	//prepare body
	std::ostringstream body;
	body<<"{\n";
	//gateway
	Gateway_ptr pGateway = pDat->getGateway();
	body<<"\"site\":\""<<pGateway->getName()<<"\",\n";
	body<<"\"index\":\""<<pGateway->getID()<<"\",\n";
	//time
	Poco::DateTime cur;
	cur += Poco::Timespan(3600*8,0);//+8 zone
	//Poco::DateTimeFormatter formatter;
	body<<"\"timestamp\":\""<<Poco::DateTimeFormatter::format(cur,"%Y-%m-%dT%H:%M:%S%z",3600*8)<<"\",\n";
	cur += Poco::Timespan(10,0);
	body<<"\"next_time\":\""<<Poco::DateTimeFormatter::format(cur,"%Y-%m-%dT%H:%M:%S%z",3600*8)<<"\",\n";
	//data
	body<<"\"data\":[\n";
	std::map<unsigned short,Sensor_ptr> sensors = pDat->getSensors();//get all sensors
	bool flagFirst = true;
	for(std::map<unsigned short,Sensor_ptr>::iterator it=sensors.begin();it!=sensors.end();it++) {
		if(flagFirst) {
			flagFirst = false;
		}
		else {
			body<<",\n";
		}
		Sensor_ptr pSensor = it->second;//get sensor
		std::string state("false");
		if(pSensor->getStatus()==DEVICE_OK) {
			state = std::string("true");
		}
		body<<"{\"name\":\""<<pSensor->getName()<<"\",\"index\":\""<<pSensor->getID()
				<<"\",\"value\":"<<pSensor->getValue()
				<<",\"max\":"<<pSensor->getMax()<<",\"min\":"<<pSensor->getMin()
				<<",\"unit\":\""<<pSensor->getUnit()<<"\","
				<<"\"state\":"<<state<<"}";
		pSensor->setStatus(DEVICE_UNKNOWN);
		pSensor->setValue(0.0);
	}
	body<<"\n]\n";
	body<<"}";
	std::cout<<"[POST] "<<body.str()<<std::endl;
	request.setContentLength(body.str().size());
	//send request
	try {
		session->sendRequest(request) << body.str();//send post
		Poco::Net::HTTPResponse response;
		std::istream& rs = session->receiveResponse(response);//get response
		if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
			std::cout<<"[HR] Succeed to post sensor data to HTTP Server"<<std::endl;
		}
		else {
			std::cout<<"[HR] Response status from HTTP Server isn't OK "<<response.getStatus()<<std::endl;
		}
	}
	catch(...) {
		std::cout<<"[HR] Failed to post sensor data to HTTP Server"<<std::endl;
	}
	session->reset();//close session
}

void HTTPReporter::reportControllers() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//prepare request
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/controllerStatus");
	request.setContentType("application/json");
	//prepare body
	std::ostringstream body;
	body<<"{\n";
	//gateway
	Gateway_ptr pGateway = pDat->getGateway();
	body<<"\"site\":\""<<pGateway->getName()<<"\",\n";
	body<<"\"gateway_id\":\""<<pGateway->getID()<<"\",\n";
	//time
	Poco::DateTime cur;
	cur += Poco::Timespan(3600*8,0);//+8 zone
	//Poco::DateTimeFormatter formatter;
	body<<"\"timestamp\":\""<<Poco::DateTimeFormatter::format(cur,"%Y-%m-%dT%H:%M:%S%z",3600*8)<<"\",\n";
	cur += Poco::Timespan(10,0);
	body<<"\"next_time\":\""<<Poco::DateTimeFormatter::format(cur,"%Y-%m-%dT%H:%M:%S%z",3600*8)<<"\",\n";
	//data
	body<<"\"data\":[\n";
	std::map<unsigned short,Controller_ptr> controllers = pDat->getControllers();//get all controllers
	bool flagFirst = true;
	for(std::map<unsigned short,Controller_ptr>::iterator it=controllers.begin();it!=controllers.end();it++) {
		if(flagFirst) {
			flagFirst = false;
		}
		else {
			body<<",\n";
		}
		Controller_ptr pController = it->second;//get sensor
		CONTROLLER_CMD cur = pController->getCurrentCmd();//get current command
		std::string state = cur.name;
		std::cout<<"[CTRL_STATUS] "<<pController->getName()<<(int)pController->getStatus()<<std::endl;
		if((pController->getStatus()==DEVICE_UNKNOWN) || (state.size()==0)) {
			state = std::string("invalid");
		}
		body<<"{\"device_id\":\""<<pController->getID()<<"\",\"name\":\""<<pController->getName()
				<<"\",\"status\":\""<<state<<"\"}";
		pController->setStatus(DEVICE_UNKNOWN);
	}
	body<<"\n]\n";
	body<<"}";
	std::cout<<"[POST] "<<body.str()<<std::endl;
	request.setContentLength(body.str().size());
	//send request
	try {
		session->sendRequest(request) << body.str();//send post
		Poco::Net::HTTPResponse response;
		std::istream& rs = session->receiveResponse(response);//get response
		if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
			std::cout<<"[HR] Succeed to post controller data to HTTP Server"<<std::endl;
		}
		else {
			std::cout<<"[HR] Response status from HTTP Server isn't OK "<<response.getStatus()<<std::endl;
		}
	}
	catch(...) {
		std::cout<<"[HR] Failed to post controller data to HTTP Server"<<std::endl;
	}
	session->reset();//close session
}

} /* namespace gw */

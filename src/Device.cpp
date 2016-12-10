/*
 * Device.cpp
 *
 *  Created on: 2016年11月17日
 *      Author: hzhou
 */

#include "Device.h"

namespace gw {

Device::Device():
		channel(0),addr(0),name(std::string()),status(DEVICE_UNKNOWN)
{}

Device::Device(unsigned char _chl,unsigned char _addr,const std::string &_id,const std::string &_name):
		channel(_chl),addr(_addr),id(_id),name(_name),status(DEVICE_UNKNOWN)
{}

Device::~Device() {}


unsigned char Device::getChannel() const {return channel;}

void Device::setChannel(unsigned char _chl) {channel = _chl;}

unsigned char Device::getAddress() const {return addr;}

void Device::setAddress(unsigned char _addr) {addr = _addr;}

std::string Device::getID() const {return id;}

void Device::setID(std::string _id) {id = _id;}

std::string Device::getName() const {return name;}

void Device::setName(std::string _name) {name = _name;}

char Device::getStatus() const {return status;}

void Device::setStatus(char _status) {
	Poco::Mutex::ScopedLock locker(mutex);
	if((_status>=DEVICE_UNKNOWN) && (_status<=DEVICE_FAULT)) {//check validation
		status = _status;
	}
}

Sensor::Sensor():
		Device(),value(0),min(0),max(0),unit(std::string())
{}

Sensor::Sensor(int _chl,unsigned char _addr,const std::string &_id,const std::string &_name):
		Device(_chl,_addr,_id,_name),value(0),min(0),max(0),unit(std::string())
{}

Sensor::~Sensor() {}

std::string Sensor::getType() const {
	return std::string("sensor");//type: sensor
}

float Sensor::getValue() const {return value;}

void Sensor::setValue(float _val) {Poco::Mutex::ScopedLock locker(mutex); value = _val;}

float Sensor::getMin() const {return min;}

float Sensor::getMax() const {return max;}

std::string Sensor::getUnit() const {return unit;}

void Sensor::setupParameter(float _min,float _max,std::string _unit) {
	min = _min; max = _max;
	unit = _unit;
}

Gateway::Gateway(): Device() {}

Gateway::Gateway(const std::string &_id,const std::string &_name):
		Device(0,0,_id,_name)
{}

Gateway::~Gateway() {}

std::string Gateway::getType() const {
	return std::string("gateway");
}

Controller::Controller():Device() {
}

Controller::Controller(int _chl, unsigned char _addr,const std::string& _id, const std::string& _name, const std::string &_type):
	Device(_chl,_addr,_id,_name),ctrlType(_type) {
}

Controller::~Controller() {
}

std::string Controller::getType() const {
	return "controller";
}

void Controller::appendCmd(unsigned char _code, const std::string& _name) {
	if(cmdHash.count(_code)==0) {
		cmdHash.insert(std::make_pair(_code,CONTROLLER_CMD(_code,_name,0)));
	}
}

CONTROLLER_CMD Controller::getCmdInfo(unsigned char _code) const {
	if(cmdHash.count(_code)) {
		return cmdHash.at(_code);
	}
	return CONTROLLER_CMD();
}

CONTROLLER_CMD Controller::getCmdInfo(const std::string& _name) const {
	for(std::map<unsigned char,CONTROLLER_CMD>::const_iterator i=cmdHash.begin();i!=cmdHash.end();i++) {
		const CONTROLLER_CMD &cmd = i->second;//get available command
		if(cmd.name==_name) {
			return cmd;
		}
	}
	return CONTROLLER_CMD();
}

void Controller::setCurrentCmd(unsigned char _code,unsigned int _para) {
	Poco::Mutex::ScopedLock locker(mutex);
	//check availability of code
	if(cmdHash.count(_code)) {
		cur = cmdHash[_code];//copy command from hash
		cur.para = _para;//record parameter
	}
}

CONTROLLER_CMD Controller::getCurrentCmd() const {
	return cur;
}

std::string Controller::getControllerType() const {
	return ctrlType;
}

void Controller::setControllerType(const std::string& _type) {
	ctrlType = _type;
}

} /* namespace gw */

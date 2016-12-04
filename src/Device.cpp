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
{
}

Device::Device(unsigned char _chl,unsigned char _addr,const std::string &_id,const std::string &_name):
		channel(_chl),addr(_addr),name(_name),id(_id),status(DEVICE_UNKNOWN)
{
}

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

void Device::setStatus(char _status)
{
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

std::string Sensor::getType() const
{
	return std::string("sensor");//type: sensor
}

float Sensor::getValue() const {return value;}

void Sensor::setValue(float _val) {value = _val;}

float Sensor::getMin() const {return min;}

float Sensor::getMax() const {return max;}

std::string Sensor::getUnit() const {return unit;}

void Sensor::setupParameter(float _min,float _max,std::string _unit)
{
	min = _min; max = _max;
	unit = _unit;
}

Gateway::Gateway():
		Device()
{}

Gateway::Gateway(const std::string &_id,const std::string &_name):
		Device(0,0,_id,_name)
{}

Gateway::~Gateway() {}

std::string Gateway::getType() const
{
	return std::string("gateway");
}

} /* namespace gw */

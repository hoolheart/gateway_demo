/*
 * DataManage.cpp
 *
 *  Created on: 2016年11月18日
 *      Author: hzhou
 */

#include "DataManage.h"

namespace gw {

DataManage::DataManage() {
	//build sensor map
	addSensor(4,0x01,"soiltemperature","土壤温度",0,60,"℃");
	addSensor(4,0x02,"soilhumidity","土壤湿度",0,100,"%RH");
	addSensor(4,0x03,"atmostemperature","大气温度",0,60,"℃");
	addSensor(4,0x04,"atmoshumidity","空气湿度",0,100,"%RH");
	addSensor(4,0x05,"illuminance","光照强度",0,100,"lux");
	addSensor(4,0x06,"co2concentration","CO2浓度",0,100,"ppm");
	//setup gateway
	gw.reset(new Gateway("1","1#连栋温室"));
	//build controller map
	std::map<unsigned char,std::string> cmds;//prepare command list
	cmds.insert(std::make_pair(1,"on")); cmds.insert(std::make_pair(2,"off"));
	addController(4,0x11,"engine_top1","天窗卷膜机1",cmds);
	addController(4,0x12,"engine_side1","侧窗卷膜机1",cmds);
	addController(4,0x13,"electromagnetic_valve","电磁阀",cmds);
	//setup channels
	channels.insert(std::make_pair(4,new SerialComm(4,19200)));//UART 4
	for(std::map<int,IComm_ptr>::iterator iChl = channels.begin();iChl!=channels.end();iChl++) {
		IComm_ptr pChl = iChl->second;//get channel
		if(pChl) {
			pChl->openChannel();//open channel
		}
	}
}

DataManage::~DataManage() {
	// TODO Auto-generated destructor stub
}

Gateway_ptr DataManage::getGateway() {
	return gw;
}

Sensor_ptr DataManage::getSensor(unsigned char chl, unsigned char addr) {
	//get united key
	unsigned short key = chl;
	key = (key<<8)+addr;
	//check key
	if(sensors.count(key)) {
		return sensors[key];
	}
	else {
		return Sensor_ptr();
	}
}

std::list<Sensor_ptr> DataManage::getSensors(unsigned char chl) {
	std::list<Sensor_ptr> list;//prepare list
	std::map<unsigned short,Sensor_ptr>::iterator i = sensors.begin();
	while(i!=sensors.end()) {
		Sensor_ptr &sensor = i->second;
		if(sensor->getChannel()==chl) {//check channel
			list.push_back(sensor);//add to list
		}
		i++;//increase iterator
	}
	return list;
}

std::map<unsigned short, Sensor_ptr> DataManage::getSensors() {
	return sensors;
}


IComm_ptr DataManage::getChannel(int chl) {
	//check channel
	if(channels.count(chl)) {
		return channels[chl];
	}
	return IComm_ptr();
}

std::list<int> DataManage::getChannels() {
	std::list<int> list;
	for(std::map<int,IComm_ptr>::iterator i=channels.begin();i!=channels.end();i++) {
		list.push_back(i->first);//record channel index
	}
	return list;
}

DataManage_ptr DataManage::instance;

DataManage_ptr DataManage::getInstance() {
	if(instance.get()==0) {
		instance.reset(new DataManage());//build instance
	}
	return instance;
}

Sensor_ptr DataManage::getSensor(std::string id) {
	std::map<unsigned short,Sensor_ptr>::iterator i = sensors.begin();
	while(i!=sensors.end()) {
		Sensor_ptr &sensor = i->second;
		if(sensor->getID()==id) {//check name
			return sensor;
		}
		i++;//increase iterator
	}
	return Sensor_ptr();
}

Controller_ptr DataManage::getController(unsigned char chl,unsigned char addr) {
	//get united key
	unsigned short key = chl;
	key = (key<<8)+addr;
	//check key
	if(controllers.count(key)) {
		return controllers[key];
	}
	else {
		return Controller_ptr();
	}
}

Controller_ptr DataManage::getController(std::string id) {
	std::map<unsigned short,Controller_ptr>::iterator i = controllers.begin();
	while(i!=controllers.end()) {
		Controller_ptr &dev = i->second;//get device
		if(dev->getID()==id) {//check name
			return dev;
		}
		i++;//increase iterator
	}
	return Controller_ptr();
}

std::list<Controller_ptr> DataManage::getControllers(unsigned char chl) {
	std::list<Controller_ptr> list;//prepare list
	std::map<unsigned short,Controller_ptr>::iterator i = controllers.begin();
	while(i!=controllers.end()) {
		Controller_ptr &dev = i->second;
		if(dev->getChannel()==chl) {//check channel
			list.push_back(dev);//add to list
		}
		i++;//increase iterator
	}
	return list;
}

std::map<unsigned short, Controller_ptr> DataManage::getControllers() {
	return controllers;
}

void DataManage::addSensor(unsigned char chl, unsigned char addr,std::string id,
		std::string name, float min, float max, std::string unit) {
	//get united key
	unsigned short key = chl;
	key = (key<<8)+addr;
	//check key availability
	if(sensors.count(key)==0) {
		Sensor_ptr sensor(new Sensor(chl,addr,id,name));//create sensor
		sensor->setupParameter(min,max,unit);
		sensors.insert(std::make_pair(key,sensor));//insert into map
	}
}

void DataManage::pushTask(TASK task) {
	tasks.push_back(task);
	while(tasks.size()>TASK_MAX_SIZE) {
		tasks.pop_front();
	}
}

bool DataManage::popTask(TASK& task) {
	if(tasks.size()) {
		task = tasks.front();//task first task
		tasks.pop_front();
		return true;
	}
	return false;
}

void DataManage::addController(unsigned char chl, unsigned char addr,
		std::string id, std::string name,
		std::map<unsigned char, std::string> cmds) {
	//get united key
	unsigned short key = chl;
	key = (key<<8)+addr;
	//check key availability
	if(controllers.count(key)==0) {
		Controller_ptr dev(new Controller(chl,addr,id,name));//create controller
		for(std::map<unsigned char, std::string>::iterator i=cmds.begin();i!=cmds.end();i++) {
			dev->appendCmd(i->first,i->second);//append available command
		}
		controllers.insert(std::make_pair(key,dev));//insert into map
	}
}

} /* namespace gw */

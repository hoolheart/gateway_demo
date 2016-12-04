/*
 * DataManage.cpp
 *
 *  Created on: 2016年11月18日
 *      Author: hzhou
 */

#include "DataManage.h"

namespace gw {

DataManage::DataManage() {
	// build sensor map
	addSensor(4,0x01,"soiltemperature","土壤温度",0,60,"℃");
	addSensor(4,0x02,"soilhumidity","土壤湿度",0,100,"%RH");
	addSensor(4,0x03,"atmostemperature","大气温度",0,60,"℃");
	addSensor(4,0x04,"atmoshumidity","空气湿度",0,100,"%RH");
	addSensor(4,0x05,"illuminance","光照强度",0,100,"lux");
	addSensor(4,0x06,"co2concentration","CO2浓度",0,100,"ppm");
	//setup gateway
	gw.reset(new Gateway("1","1#连栋温室"));
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

std::list<Sensor_ptr> DataManage::getSensorList(unsigned char chl) {
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

std::map<int, IComm_ptr> DataManage::getChannels() {
	return channels;
}

DataManage_ptr DataManage::instance;

DataManage_ptr DataManage::getInstance() {
	if(instance.get()==0) {
		instance.reset(new DataManage());//build instance
	}
	return instance;
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

} /* namespace gw */

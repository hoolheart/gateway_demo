/*
 * DataManage.h
 *
 *  Created on: 2016年11月18日
 *      Author: hzhou
 */

#ifndef DATAMANAGE_H_
#define DATAMANAGE_H_

#include "Device.h"
#include "CommInterface.h"
#include <map>
#include <list>
#include <boost/shared_ptr.hpp>

namespace gw {

class DataManage;
typedef boost::shared_ptr<DataManage> DataManage_ptr;

class DataManage {
private:
	std::map<unsigned short,Sensor_ptr> sensors;
	std::map<int,IComm_ptr> channels;
	Gateway_ptr gw;

private:
	explicit DataManage();
	static DataManage_ptr instance;

public:
	static DataManage_ptr getInstance();
	~DataManage();

	Gateway_ptr getGateway();
	Sensor_ptr getSensor(unsigned char chl,unsigned char addr);
	std::list<Sensor_ptr> getSensorList(unsigned char chl);
	std::map<unsigned short,Sensor_ptr> getSensors();
	IComm_ptr getChannel(int chl);
	std::map<int,IComm_ptr> getChannels();

private:
	void addSensor(unsigned char chl,unsigned char addr,std::string id,
			std::string name,float min,float max,std::string unit);
};

} /* namespace gw */

#endif /* DATAMANAGE_H_ */

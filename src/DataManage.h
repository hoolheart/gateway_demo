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

struct TASK {
	std::string device_id;
	std::string command;
	unsigned int para;

	TASK():device_id(std::string()),command(std::string()),para(0) {}
	TASK(const std::string &_id,const std::string &_cmd,unsigned int _para):device_id(_id),command(_cmd),para(_para) {}
};

class DataManage;
typedef boost::shared_ptr<DataManage> DataManage_ptr;

#define TASK_MAX_SIZE 10

class DataManage {
private:
	std::map<unsigned short,Sensor_ptr> sensors;
	std::map<unsigned short,Controller_ptr> controllers;
	std::map<int,IComm_ptr> channels;
	Gateway_ptr gw;

	std::list<TASK> tasks;

private:
	explicit DataManage();
	static DataManage_ptr instance;

public:
	static DataManage_ptr getInstance();
	~DataManage();

	Gateway_ptr getGateway();
	Sensor_ptr getSensor(unsigned char chl,unsigned char addr);
	Sensor_ptr getSensor(std::string id);
	std::list<Sensor_ptr> getSensors(unsigned char chl);
	std::map<unsigned short,Sensor_ptr> getSensors();
	Controller_ptr getController(unsigned char chl,unsigned char addr);
	Controller_ptr getController(std::string id);
	std::list<Controller_ptr> getControllers(unsigned char chl);
	std::map<unsigned short,Controller_ptr> getControllers();

	std::list<int> getChannels();
	IComm_ptr getChannel(int chl);

	void pushTask(TASK task);//push new task
	bool popTask(TASK &task);//pop existing task

private:
	void addSensor(unsigned char chl,unsigned char addr,std::string id,
			std::string name,float min,float max,std::string unit);
	void addController(unsigned char chl,unsigned char addr,std::string id,std::string name,
			std::map<unsigned char,std::string> cmds);
};

} /* namespace gw */

#endif /* DATAMANAGE_H_ */

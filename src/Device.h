/*
 * Device.h
 *
 *  Created on: 2016年11月17日
 *      Author: hzhou
 */

#ifndef DEVICE_H_
#define DEVICE_H_
#include <string>
#include <boost/shared_ptr.hpp>
#include <map>

namespace gw {

enum DEVICE_STATUS {
	DEVICE_UNKNOWN = -1,
	DEVICE_INIT = 0,
	DEVICE_OK = 1,
	DEVICE_WARN = 2,
	DEVICE_FAULT = 3
};

class Device {
private:
	unsigned char channel;
	unsigned char addr;
	std::string id;
	std::string name;
	char status;

public:
	explicit Device();
	explicit Device(unsigned char _chl,unsigned char _addr,const std::string &_id,const std::string &_name);
	virtual ~Device();

	unsigned char getChannel() const;
	void setChannel(unsigned char _chl);
	unsigned char getAddress() const;
	void setAddress(unsigned char _addr);
	std::string getID() const;
	void setID(std::string _id);
	std::string getName() const;
	void setName(std::string _name);
	char getStatus() const;
	void setStatus(char _status);

	virtual std::string getType() const=0;
};

class Sensor : public Device {
private:
	float value;
	float min,max;
	std::string unit;

public:
	explicit Sensor();
	explicit Sensor(int _chl,unsigned char _addr,const std::string &_id,const std::string &_name);
	~Sensor();

	virtual std::string getType() const;

	float getValue() const;
	void setValue(float _val);
	float getMin() const;
	float getMax() const;
	std::string getUnit() const;
	void setupParameter(float _min,float _max,std::string _unit);
};

class Gateway : public Device
{
private:

public:
	explicit Gateway();
	explicit Gateway(const std::string &_id,const std::string &_name);
	~Gateway();

	virtual std::string getType() const;
};

struct CONTROLLER_CMD {
	unsigned char code;
	std::string name;
	unsigned int para;

	CONTROLLER_CMD():code(0),name(std::string()),para(0) {}
	CONTROLLER_CMD(unsigned char _code,const std::string &_name,unsigned int _para):code(_code),name(_name),para(_para) {}
};

class Controller : public Device {
private:
	std::map<unsigned char,CONTROLLER_CMD> cmdHash;/**< hash of available commands */
	CONTROLLER_CMD cur;/**< current command status */

public:
	explicit Controller();
	explicit Controller(int _chl,unsigned char _addr,const std::string &_id,const std::string &_name);
	~Controller();

	virtual std::string getType() const;

	void appendCmd(unsigned char _code,const std::string &_name);
	CONTROLLER_CMD getCmdInfo(unsigned char _code) const;
	CONTROLLER_CMD getCmdInfo(const std::string &_name) const;
	void setCurrentCmd(unsigned char _code,unsigned int _para);
	CONTROLLER_CMD getCurrentCmd() const;
};

// smart pointers
typedef boost::shared_ptr<Device> Device_ptr;
typedef boost::shared_ptr<Sensor> Sensor_ptr;
typedef boost::shared_ptr<Gateway> Gateway_ptr;
typedef boost::shared_ptr<Controller> Controller_ptr;

} /* namespace gw */

#endif /* DEVICE_H_ */

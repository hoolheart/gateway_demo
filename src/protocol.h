/*
 * protocol.h
 *
 *  Created on: 2016年11月20日
 *      Author: hzhou
 */

#ifndef PROTOCOL_H_
#define PROTOCOL_H_

#pragma pack(push)
#pragma pack(1)

enum SENSOR_STATUS {
	SENSOR_INIT=0,
	SENSOR_OK=1,
	SENSOR_WARN=2,
	SENSOR_FAULT=3
};

/** status and value report from sensor */
struct SENSOR_REPORT {
	unsigned char status;
	float value;
	unsigned char bak[2];
	unsigned char checkSum;
};

/** status and command report from controller */
struct CONTROLLER_REPORT {
	unsigned char code;
	unsigned int para;
	unsigned char bak[2];
	unsigned char checkSum;
};

/** command to controller */
struct CONTROLLER_COMMAND {
	unsigned char code;
	unsigned int para;
	unsigned char bak[2];
	unsigned char checkSum;
};

/** command echo to controller */
struct CONTROLLER_CMD_ECHO {
	unsigned char code;
	unsigned int para;
	unsigned char success;
	unsigned char bak;
	unsigned char checkSum;
};

#pragma pack(pop)

#endif /* PROTOCOL_H_ */

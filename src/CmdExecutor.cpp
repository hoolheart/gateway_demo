/*
 * CmdExecutor.cpp
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#include "CmdExecutor.h"
#include "protocol.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <sstream>
#include <iostream>

namespace gw {

CmdExecutor::CmdExecutor(const std::string & host,int port):received(false),result(false) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(1,0));
}

CmdExecutor::~CmdExecutor() {
	// TODO Auto-generated destructor stub
}

void CmdExecutor::run() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//get controller
	dev = pDat->getController(task.device_id);
	if(dev && (comm = pDat->getChannel(dev->getChannel()))) {//get communicate channel
		cmd = dev->getCmdInfo(task.command);//get command
		if(cmd.code>0) {
			received = false;
			//prepare data frame
			COMM_DATA_FRAME frame;
			frame.addr = dev->getAddress();//address
			frame.type = 0x0A;//control
			CONTROLLER_COMMAND *command = (CONTROLLER_COMMAND*)frame.data;//apply protocol
			command->code = cmd.code;//code
			command->para = task.para;//parameter
			comm->sendData(frame);//send data
			for(int i=0;(i<100)&&(!received);i++) {
				Poco::Thread::sleep(10);
			}
			if(!received) {
				result = false;
			}
			//report
			//prepare request
			Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, "/commandEcho");
			request.setContentType("application/json");
			//prepare body
			std::ostringstream body;
			body<<"{\n";
			//gateway
			Gateway_ptr pGateway = pDat->getGateway();
			body<<"\"gateway_id\":\""<<pGateway->getID()<<"\",\n";
			//data
			body<<"\"device_id\":\""<<task.device_id<<"\",\n";
			body<<"\"command\":\""<<task.command<<"\",\n";
			std::string state("false");
			if(result) {
				state = std::string("true");
			}
			body<<"\"result\":"<<state<<"\n";
			body<<"}";
			std::cout<<"[POST] "<<body.str()<<std::endl;
			request.setContentLength(body.str().size());
			//send request
			try {
				session->sendRequest(request) << body.str();//send post
				Poco::Net::HTTPResponse response;
				std::istream& rs = session->receiveResponse(response);//get response
				if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
					std::cout<<"[HR] Succeed to post command echo to HTTP Server"<<std::endl;
				}
				else {
					std::cout<<"[HR] Response status from HTTP Server isn't OK "<<response.getStatus()<<std::endl;
				}
			}
			catch(...) {
				std::cout<<"[HR] Failed to post command echo to HTTP Server"<<std::endl;
			}
			session->reset();//close session
		}
	}
}

void CmdExecutor::onCommData(unsigned char chl, COMM_DATA_FRAME& frame) {
	if(frame.type==0xA0) {//check type
		if(dev && (dev->getChannel()==chl) && (dev->getAddress()==frame.addr)) {//check channel and address
			CONTROLLER_CMD_ECHO *msg = (CONTROLLER_CMD_ECHO*)frame.data;//apply protocol
			if(msg->code==cmd.code) {
				received = true;//mark received flag
				result = (msg->success>0);//record result
			}
		}
	}
}

} /* namespace gw */

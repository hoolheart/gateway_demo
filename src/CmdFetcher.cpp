/*
 * CmdFetcher.cpp
 *
 *  Created on: 2016年12月4日
 *      Author: hzhou
 */

#include "CmdFetcher.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/StreamCopier.h"
#include <sstream>
#include "DataManage.h"
#include <iostream>
#include "Poco/JSON/Parser.h"
#include "Poco/JSON/ParseHandler.h"
#include "Poco/JSON/JSONException.h"
#include "Poco/JSON/Object.h"
#include "Poco/Dynamic/Struct.h"

namespace gw {

CmdFetcher::CmdFetcher(const std::string & host,int port) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(1,0));
}

CmdFetcher::~CmdFetcher() {
	// TODO Auto-generated destructor stub
}

void CmdFetcher::run() {
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//prepare request
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, "/commands");
	request.setContentType("application/json");
	request.set("Accept","application/json");
	//prepare body
	std::ostringstream body;
	//gateway
	Gateway_ptr pGateway = pDat->getGateway();
	body<<"{\"gateway_id\":\""<<pGateway->getID()<<"\"}";
	std::cout<<"[GET_REQ] "<<body.str()<<std::endl;
	request.setContentLength(body.str().size());
	//send request
	try {
		session->sendRequest(request) << body.str();//send post
		Poco::Net::HTTPResponse response;
		std::istream& rs = session->receiveResponse(response);//get response
		if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
			if(response.getContentType().compare("application/json")==0) {
				std::cout<<"[CF] Succeed to get commands from HTTP Server"<<std::endl;
				Poco::JSON::Parser parser;//prepare parser
				Poco::Dynamic::Var result;
				try {
					result = parser.parse(rs);//parse data
					handleJSON(result);//handle json
				}
				catch(Poco::JSON::JSONException& jsone) {
					std::cout << "[CF] json parse error:" << jsone.message() << std::endl;
				}
			}
			else {
				std::cout<<"[CF] Wrong content type:"<<response.getContentType()<<std::endl;
			}
		}
		else {
			std::cout<<"[CF] Response status from HTTP Server isn't OK "<<response.getStatus()<<std::endl;
		}
	}
	catch(...) {
		std::cout<<"[CF] Failed to get commands from HTTP Server"<<std::endl;
	}
	session->reset();//close session
}

void CmdFetcher::handleJSON(Poco::Dynamic::Var &result) {
	if(result.type() == typeid(Poco::JSON::Object::Ptr)) {
		Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
		Poco::DynamicStruct ds = *object;//convert to dynamic structure
		//get DataManage
		DataManage_ptr pDat = DataManage::getInstance();
		//check gateway id
		Gateway_ptr gw = pDat->getGateway();
		if((!ds.contains("gateway_id")) || (gw->getID().compare(ds["gateway_id"].toString()))!=0) {
			std::cout<<"[CF] Wrong gateway ID"<<std::endl;
			return;
		}
		//fetch command
		if(ds.contains("commands") && ds["commands"].isArray() && ds["commands"].size()>0) {
			for(unsigned int i=0;i<ds["commands"].size();i++) {
				Poco::Dynamic::Var cmd = ds["commands"][i];//get command
				if(cmd["device_id"].isString() && cmd["command"].isString()) {
					pDat->pushTask(TASK(cmd["device_id"].toString(),cmd["command"].toString(),0));//push task into list
				}
			}
		}
	}
}

} /* namespace gw */

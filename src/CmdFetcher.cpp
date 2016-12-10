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
#include "Poco/Timestamp.h"

namespace gw {

CmdFetcher::CmdFetcher(const std::string & host,int port) {
	// create session
	session.reset(new Poco::Net::HTTPClientSession(host,port));
	session->setTimeout(Poco::Timespan(1,0));
	//time
	t = Poco::Timestamp().epochMicroseconds();
}

CmdFetcher::~CmdFetcher() {
	// TODO Auto-generated destructor stub
}

void CmdFetcher::run() {
	std::cout<<"!!!Command Fetcher!!!"<<std::endl;
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//prepare request
	std::ostringstream uri; uri<<"/deviceControl/"<<(t/1000);
	t = Poco::Timestamp().epochMicroseconds();//update time
	Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_GET, uri.str());
	//std::cout<<"[URI_FETCH] "<<uri.str()<<std::endl;
	request.setContentType("application/json");
	request.set("Accept","application/json");
	//prepare body
	std::ostringstream body;
	//gateway
	Gateway_ptr pGateway = pDat->getGateway();
	body<<"{\"gateway_id\":\""<<pGateway->getID()<<"\"}";
	std::cout<<"[GET_REQ] "<<uri.str()<<std::endl;
	request.setContentLength(body.str().size());
	//send request
	try {
		session->sendRequest(request) << body.str();//send post
		Poco::Net::HTTPResponse response;
		std::istream& rs = session->receiveResponse(response);//get response
		if(response.getStatus()==Poco::Net::HTTPResponse::HTTP_OK) {
			if(response.getContentType().find("application/json")!=std::string::npos) {
				std::cout<<"[CF] Succeed to get commands from HTTP Server"<<std::endl;
				Poco::JSON::Parser parser;//prepare parser
				Poco::Dynamic::Var result;
				//get content
				std::ostringstream res_content;
				Poco::StreamCopier::copyStream(rs, res_content);
				std::cout<<"[RES_CMD]"<<res_content.str()<<std::endl;
				if(res_content.str().size()>5) {
					try {
						result = parser.parse(res_content.str());//parse data
						if(result.type() == typeid(Poco::JSON::Array::Ptr)) {
							Poco::JSON::Array::Ptr arr = result.extract<Poco::JSON::Array::Ptr>();
							for(unsigned int i=0;i<arr->size();i++) {
								handleJSON(arr->get(i));//handle json
							}
						}
					}
					catch(Poco::JSON::JSONException& jsone) {
						std::cout << "[CF] json parse error:" << jsone.message() << std::endl;
					}
				}
			}
			else {
				std::cout<<"[CF] Wrong content type:"<<response.getContentType()<<std::endl;
			}
		}
		else {
			std::cout<<"[CF] Response status from HTTP Server isn't OK "<<response.getStatus()<<std::endl;
		}
		session->reset();//close session
	}
	catch(...) {
		std::cout<<"[CF] Failed to get commands from HTTP Server"<<std::endl;
	}
	Poco::Thread::sleep(10);
}

void CmdFetcher::handleJSON(Poco::Dynamic::Var result) {
	//std::cout<<result.toString()<<std::endl;
	Poco::JSON::Object::Ptr object = result.extract<Poco::JSON::Object::Ptr>();
	Poco::DynamicStruct ds = *object;//convert to dynamic structure
	//get DataManage
	DataManage_ptr pDat = DataManage::getInstance();
	//check gateway id
	//Gateway_ptr gw = pDat->getGateway();
	//if(gw->getID().compare(ds["gateway_id"].toString())!=0) {
		//std::cout<<"[CF] Wrong gateway ID"<<std::endl;
		//return;
	//}
	//fetch command
	std::cout<<"[CMD_SIZE]"<<ds["data"].size()<<std::endl;
	if(ds["data"].size()>0) {
		for(unsigned int i=0;i<ds["data"].size();i++) {
			std::cout<<ds["data"][i].toString()<<std::endl;
			if(ds["data"][i]["device_id"].isString() && ds["data"][i]["state"].isBoolean()) {
				std::string cmd_name = ds["data"][i]["state"].extract<bool>()?std::string("on"):std::string("off");
				std::cout<<"[CMD]"<<ds["data"][i]["device_id"].toString()<<cmd_name<<std::endl;
				pDat->pushTask(TASK(ds["data"][i]["device_id"].toString(),cmd_name,0));//push task into list
			}
		}
	}
}

} /* namespace gw */

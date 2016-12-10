/*
 * ErrorHandler.cpp
 *
 *  Created on: 2016年12月10日
 *      Author: hzhou
 */

#include "ErrorHandler.h"
#include <iostream>

namespace gw {

ErrorHandler::ErrorHandler():Poco::ErrorHandler() {
	// TODO Auto-generated constructor stub

}

ErrorHandler::~ErrorHandler() {
	// TODO Auto-generated destructor stub
}

void ErrorHandler::exception(const Poco::Exception& exc) {
	std::cout<<"[ERROR] Poco Exception:"<<exc.what()<<std::endl;
	std::cout<<"[ERROR] Poco Exception name:"<<exc.name()<<std::endl;
	std::cout<<"[ERROR] Poco Exception message:"<<exc.message()<<std::endl;
	std::cout<<"[ERROR] Poco Exception code:"<<exc.code()<<std::endl;
	std::cout<<"[ERROR] Poco Exception displayText:"<<exc.displayText()<<std::endl;
}

void ErrorHandler::exception(const std::exception& exc) {
	std::cout<<"[ERROR] System Exception:"<<exc.what()<<std::endl;
}

void ErrorHandler::exception() {
	std::cout<<"[ERROR] Unknown type of Exception"<<std::endl;
}

} /* namespace gw */

//============================================================================
// Name        : gateway_demo.cpp
// Author      : petit_kayak
// Version     :
// Copyright   : espace_de_petit_kayak
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>

#include "Poco/Timer.h"
#include "Poco/Thread.h"
#include "Poco/Stopwatch.h"
#include "Poco/Logger.h"

#include "DataManage.h"
#include <boost/shared_ptr.hpp>

#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"

#include "DeviceChecker.h"
#include "HTTPReporter.h"
#include "CmdFetcher.h"
#include "CmdExecutor.h"

#define REMOTE_ADDR "118.178.189.181"
#define REMOTE_PORT 3030

enum CUR_MODE {
	MODE_INIT=-1,
	MODE_CHECK,
	MODE_REPORT,
	MODE_FETCH,
	MODE_EXECUTE
};

class MainProcess
{
private:
	Poco::Stopwatch _sw;/**< watch to show time passed */
	int step;
	boost::shared_ptr<gw::DeviceChecker> checker;
	boost::shared_ptr<gw::HTTPReporter> reporter;
	boost::shared_ptr<gw::CmdFetcher> fetcher;
	boost::shared_ptr<gw::CmdExecutor> executor;
	gw::DataManage_ptr pDat;
	Poco::Thread thd;
	CUR_MODE mode;

public:
	explicit MainProcess():step(0),mode(MODE_INIT)
	{
		_sw.start();//start stopwatch]
		checker.reset(new gw::DeviceChecker());//prepare sensor checker
		reporter.reset(new gw::HTTPReporter(REMOTE_ADDR,REMOTE_PORT));//prepare http reporter
		fetcher.reset(new gw::CmdFetcher(REMOTE_ADDR,REMOTE_PORT));//prepare command fetcher
		executor.reset(new gw::CmdExecutor(REMOTE_ADDR,REMOTE_PORT));//prepare command executor

		//bind comm
		pDat = gw::DataManage::getInstance();
		std::list<int> chls = pDat->getChannels();//get channels
		for(std::list<int>::iterator iChl = chls.begin();iChl!=chls.end();iChl++) {
			int chl = *iChl;//get channel
			gw::IComm_ptr comm = pDat->getChannel(chl);//get channel
			if(comm) {
				//register checker
				comm->regCommDataHandler(gw::CommDataHandler_ptr(checker));
				comm->regCommErrHandler(gw::CommErrHandler_ptr(checker));
				//register executor
				comm->regCommDataHandler(gw::CommDataHandler_ptr(executor));
			}
		}
	}
	~MainProcess() {
		thd.join();
	}

	void onTimer(Poco::Timer& timer)
	{
		step++;//increase step

		if((step%10)==0) {//print heart beat every second
			std::cout << "[MP] Demo has run " << _sw.elapsed()/1e6 << " s." << std::endl;
		}

		if(thd.isRunning()) {
			if((step==50) && (mode!=MODE_CHECK)) {
				pDat->pushTask(gw::TASK("gateway","query",0));
			}
			else if((step==100) && (mode!=MODE_REPORT)) {
				pDat->pushTask(gw::TASK("gateway","report",0));
			}
		}
		else {
			gw::TASK task;
			while(pDat->popTask(task)) {
				if(task.device_id=="gateway") {
					if(task.command=="query") {
						thd.start(*checker);//start check
					}
					else if(task.command=="report") {
						thd.start(*reporter);//start report
					}
				}
				else if(task.device_id.size()>0){
					executor->setTask(task);
					thd.start(*executor);//start execute
				}
				if(thd.isRunning()) {
					break;
				}
			}
			if(!thd.isRunning()) {
				if(step==50) {
					thd.start(*checker);
				}
				else if(step==100) {
					thd.start(*reporter);
				}
				else {
					thd.start(*fetcher);
				}
			}
		}

		if(step==100) {
			step = 0;
		}
	}
};

int main() {
	std::cout << "Hello World" << std::endl; // prints !!!Hello World!!!
	std::cout << Poco::DateTimeFormatter::format(Poco::DateTime(),"%Y-%m-%dT%H:%M:%S%z") <<std::endl;

	//setup data
	gw::DataManage::getInstance();

	//setup timer
	Poco::Timer timer(100,100);
	MainProcess mainProc;
	timer.start(Poco::TimerCallback<MainProcess>(mainProc, &MainProcess::onTimer));

	//wait for break
	int exit_code;
	std::cin >> exit_code;
	timer.stop();

	return exit_code;
}

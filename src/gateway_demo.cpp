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

#include "ErrorHandler.h"

#define REMOTE_ADDR "192.168.0.101"
//#define REMOTE_ADDR "118.178.189.181"
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
	boost::shared_ptr<Poco::Thread> thd;
	CUR_MODE mode;

public:
	explicit MainProcess():step(0),mode(MODE_INIT)
	{
		_sw.start();//start stopwatch]
		checker.reset(new gw::DeviceChecker());//prepare sensor checker
		reporter.reset(new gw::HTTPReporter(REMOTE_ADDR,REMOTE_PORT));//prepare http reporter
		fetcher.reset(new gw::CmdFetcher(REMOTE_ADDR,REMOTE_PORT));//prepare command fetcher
		executor.reset(new gw::CmdExecutor(REMOTE_ADDR,REMOTE_PORT));//prepare command executor

		//prepare thread
		thd.reset(new Poco::Thread());

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
		thd->join();
	}

	void tryStartThd(CUR_MODE m) {
		//get runnable
		boost::shared_ptr<Poco::Runnable> runnable;
		switch (m) {
		case MODE_CHECK:
			runnable = checker;
			break;
		case MODE_REPORT:
			runnable = reporter;
			break;
		case MODE_FETCH:
			runnable = fetcher;
			break;
		case MODE_EXECUTE:
			runnable = executor;
			break;
		default:
			break;
		}
		if(runnable && (!thd->isRunning())) {
			try {
				thd->join();
				thd->start(*runnable);
				mode = m;
			}
			catch (...) {
				std::cout<<"Failed to start thread for mode: "<<m<<std::endl;
				thd.reset(new Poco::Thread());
				std::cout<<"Recreate thread"<<std::endl;
			}
		}
	}

	void onTimer(Poco::Timer& timer)
	{
		step++;//increase step

		if((step%100)==0) {//print heart beat every second
			std::cout << "[MP] Demo has run " << _sw.elapsed()/1e6 << " s." << std::endl;
		}

		if(thd->isRunning()) {
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
						tryStartThd(MODE_CHECK);
					}
					else if(task.command=="report") {
						tryStartThd(MODE_REPORT);
					}
				}
				else if(task.device_id.size()>0){
					executor->setTask(task);
					tryStartThd(MODE_EXECUTE);
				}
				if(thd->isRunning()) {
					break;
				}
			}
			if(!thd->isRunning()) {
				if(step==50) {
					tryStartThd(MODE_CHECK);
				}
				else if(step==100) {
					tryStartThd(MODE_REPORT);
				}
				else if((step%10)==0){
					tryStartThd(MODE_FETCH);
				}
			}
		}

		if(step>=100) {
			step = 0;
		}
	}
};

int main() {
	std::cout << "Hello World" << std::endl; // prints !!!Hello World!!!
	std::cout << Poco::DateTimeFormatter::format(Poco::DateTime(),"%Y-%m-%dT%H:%M:%S%z") <<std::endl;

	//prepare error handler
	gw::ErrorHandler handler;
	Poco::ErrorHandler *pre_handler = Poco::ErrorHandler::set(&handler);

	//setup data
	gw::DataManage::getInstance();

	//setup timer
	Poco::Timer timer(10,10);
	MainProcess mainProc;
	timer.start(Poco::TimerCallback<MainProcess>(mainProc, &MainProcess::onTimer));

	//wait for break
	int exit_code;
	std::cin >> exit_code;
	timer.stop();

	//restore error handler and exit
	Poco::ErrorHandler::set(pre_handler);
	return exit_code;
}

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
#include "SensorChecker.h"
#include "HTTPReporter.h"
#include <boost/scoped_ptr.hpp>

#include "Poco/DateTime.h"
#include "Poco/DateTimeFormatter.h"

class MainProcess
{
private:
	Poco::Stopwatch _sw;/**< watch to show time passed */
	int step;
	boost::scoped_ptr<gw::SensorChecker> sensorChecker;
	boost::scoped_ptr<gw::HTTPReporter> reporter;
	Poco::Thread chkThd,rptThd;

public:
	explicit MainProcess()
	{
		_sw.start();//start stopwatch
		step = 0;//initialize step
		sensorChecker.reset(new gw::SensorChecker());//prepare sensor checker
		reporter.reset(new gw::HTTPReporter("118.178.189.181",3030));//prepare http reporter
	}
	~MainProcess() {
		chkThd.join();
		rptThd.join();
	}

	void onTimer(Poco::Timer& timer)
	{
		step++;//increase step

		if((step%2)==0) {//print heart beat every second
			std::cout << "[MP] Demo has run " << _sw.elapsed()/1e6 << " s." << std::endl;
		}

		if(step==10) {
			//start check when 5s passed
			if(!chkThd.isRunning() && !rptThd.isRunning()) {
				chkThd.start(*sensorChecker);
			}
		}
		else if(step==20) {
			//start report
			if(!chkThd.isRunning() && !rptThd.isRunning()) {
				rptThd.start(*reporter);
			}
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
	Poco::Timer timer(500,500);
	MainProcess mainProc;
	timer.start(Poco::TimerCallback<MainProcess>(mainProc, &MainProcess::onTimer));

	//wait for break
	int exit_code;
	std::cin >> exit_code;
	timer.stop();

	return exit_code;
}

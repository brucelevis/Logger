#include "Logger.hpp"
#include <iostream>

//生产日志
void ProduceLog(IntLogger& logger) {
	static const int LOG_NUMS = 1000;
	for (int i = 0; i < LOG_NUMS / 10; ++i) {
		logger.Log(MilliSec() * LOG_NUMS + i, i);
	}
	std::cout << "one thread gen logs done!" << std::endl;
}

int main()
{
	std::string log_name = "test.csv";
	IntLogger logger(log_name, 100);

	std::cout << "begin gen logs!" << std::endl;

	//2个线程生产日志
	std::thread t1(ProduceLog, std::ref(logger));
	std::thread t2(ProduceLog, std::ref(logger));

	t1.join();
	t2.join();

	std::cout << "logs flush!" << std::endl;

	logger.Stop();
}
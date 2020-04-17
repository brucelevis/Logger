#pragma once

#include <string>
#include <queue>
#include <atomic>
#include <fstream>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <cstdint>

//��־��
struct LogItem
{
	uint64_t t;
	uint64_t v;
	LogItem(uint64_t time, uint64_t value)
		: t(time), v(value) {}

	bool operator < (const LogItem& item) const
	{
		//��Сֵ����
		return t > item.t;
	}
};

//΢��ʱ���
uint64_t MilliSec()
{
	auto now = std::chrono::system_clock::now();
	return std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();
}

class IntLogger {
public:
	IntLogger(std::string& filename, int window)
	{
		_window = window;
		_running = true;
		_file.open(filename);
		_process_thread = std::thread(&IntLogger::ProcessLog, this);
	};

	~IntLogger()
	{
		_process_thread.join();
		_file.close();
	}

	void Log(uint64_t time, uint64_t value)
	{
		std::unique_lock<std::mutex> ul(_mutex);

		//�����
		LogItem item{ time, value };
		_one_que.emplace(std::move(item));

		auto top = _one_que.top();
		auto now = MilliSec();
		//��ʱ��΢�룩���߳������г��ȣ������������
		if (now - top.t >= _window * 1000 * 1000 || _one_que.size() >= LOG_QUEUE_LEN)
		{
			//֪ͨд���߳�
			_process_cond.notify_one();
		}
	};

	void Stop()
	{
		_running = false;
	}

private:
	void ProcessLog()
	{
		while (_running)
		{
			std::unique_lock<std::mutex> ul(_mutex);
			_process_cond.wait(ul);

			//�����������
			_two_que.swap(_one_que);

			while (!_two_que.empty())
			{
				auto item = _two_que.top();
				_file << item.t << "," << item.v << '\n';
				_two_que.pop();
			}

			//ˢ����־
			_file.flush();
		}
	}

private:
	static const int LOG_QUEUE_LEN = 100;
	//˫�������
	std::priority_queue<LogItem, std::vector<LogItem>> _one_que;
	std::priority_queue<LogItem, std::vector<LogItem>> _two_que;

	//�����߳�
	std::thread _process_thread;
	std::mutex _mutex;
	std::condition_variable _process_cond;

	//��־�ļ�
	std::ofstream _file;

	//��ʱʱ��
	int _window;
	//�����߳�����
	bool _running;
};
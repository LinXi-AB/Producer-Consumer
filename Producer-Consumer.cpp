#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>

using namespace std;

class Queue
{
public:
	explicit Queue(size_t max_size) : max_size(max_size)
	{
	}

	void push(int val)
	{
		unique_lock<mutex> lock(mtx);
		producer_cv.wait(lock, [this]
			{
				return q.size() <= 10;
			});// 如果队列满则阻塞

		// 如果队列未满
		q.push(val);
		consumer_cv.notify_one();// 通知消费者停止阻塞
	}

	int pop()
	{
		unique_lock<mutex> lock(mtx);
		consumer_cv.wait(lock, [this]
			{
				return !q.empty();
			});// 如果队列为空则阻塞

		// 如果队列不为空
		int front = q.front();
		q.pop();
		producer_cv.notify_one();// 通知生产者
		return front;
	}

private:
	queue<int> q;
	size_t max_size = 10;// 队列的最大长度
	mutex mtx;
	condition_variable consumer_cv;// 消费者同步
	condition_variable producer_cv;// 生产者同步
};

/**
 * @brief 从消息队列中取走数据，当消息队列为空时需要阻塞
 */
void consumer(Queue& q)
{
	for (int i = 1; i < 100; i++)
	{
		cout << "消费者: " << q.pop() << endl;
		this_thread::sleep_for(1000ms);
	}
}

/**
 * @brief 将数据放入消息队列，当消息队列满时需要阻塞
 */
void producer(Queue& q)
{
	for (int i = 1; i < 100; i++)
	{
		cout << "生产者: " << i << endl;
		q.push(i);
		this_thread::sleep_for(100ms);
	}
}

/**
 * @brief 如果要编写测试，则应该考虑一下几个测试点
 * 1.队列为空时消费者阻塞
 * 2.队列满时生产者阻塞
 * 3.队列大小永远不大于最大值
 * 4,。
 */
int main()
{
	Queue q(10);
	thread pro(producer, std::ref(q));
	thread con(consumer, std::ref(q));
	pro.join();
	con.join();
	return 0;
}


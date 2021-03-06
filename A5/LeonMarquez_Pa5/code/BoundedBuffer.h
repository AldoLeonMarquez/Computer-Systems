#ifndef BoundedBuffer_h
#define BoundedBuffer_h

#include <iostream>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
using namespace std;

class BoundedBuffer
{
private:
  	int cap;
  	queue<vector<char>> q;

	/* mutexto protect the queue from simultaneous producer accesses
	or simultaneous consumer accesses */
	mutex mtx;
	
	/* condition that tells the consumers that some data is there */
	condition_variable data_available;
	/* condition that tells the producers that there is some slot available */
	condition_variable slot_available;

public:
	BoundedBuffer(int _cap):cap(_cap){
		 
	}
	~BoundedBuffer(){

	}

	void push(vector<char> data){
		unique_lock<mutex> lk (mtx);
		//wait until a slot opens
		slot_available.wait (lk,[this]{return q.size() < cap ;}); 
		//Push data msg
		q.push(data);
		//notify consumers
		data_available.notify_one();
		lk.unlock();
	}

	vector<char> pop(){
		
		unique_lock<mutex> lk (mtx);
		//wait until theres data 
		data_available.wait (lk,[this]{return q.size() > 0 ;}); 
		vector<char> temp = q.front();
		q.pop();
		//notify Producers
		slot_available.notify_one();
		lk.unlock();
		
		return temp;  
	}
};

#endif /* BoundedBuffer_ */

#include "common.h"
#include "BoundedBuffer.h"
#include "Histogram.h"
#include "common.h"
#include "HistogramCollection.h"
#include "NRC.h"
#include <mutex>
#include <ostream>
#include <stdio.h>
#include <unistd.h> 
#include <sys/types.h> 
#include <fcntl.h> 

using namespace std;

struct Msg {
	int n = 100;    
    int p = 14;          //I had the idea of passing a struct wit
	int m = MAX_MESSAGE; //I forgot about it and and left this mess
};
//I had the idea of passing a struct wit
//I forgot about it and and left this mess
void * patient_function(int p, int* n, BoundedBuffer& B,int* done, mutex* m,int maxp,int mode,string* fName,char* fSize)
{
    /* What will the patient threads do? */
	
	if (mode == 0){  //Data mode 
		double Limit = ((*n)-1) * 0.004;
	 
		for(double i = 0; i < Limit ; i+=0.004){
				
			datamsg dm1 = datamsg(p,i,1); //ecg 1
			char* Msg = reinterpret_cast<char *> (&dm1);
			
			vector<char> msgBB(Msg,Msg+sizeof(dm1)); //vector Message for the Buffer
			B.push(msgBB);
			
			//cout<<i<<endl;
			//o<<i<<", "<<*(double*)temp<<", "<<*(double*)temp2<<endl;
		}
		m->lock();
		*done = *done + 1;
		//cout<<*done<<endl;
		if(*done == maxp){
			MESSAGE_TYPE q = QUIT_MSG;
			char* ex = reinterpret_cast<char *> (&q);
			vector<char> msgBB(ex,ex+sizeof(q));
			B.push(msgBB);
			//cout<<"just once pls :)"<<endl;
		}
		m->unlock();
	}
	else if(mode == 1){// Data mode;
		//cout<<*(__int64_t*)fSize<< "?? POGCHAMP ?"<<endl; //size of file
		
		int last = *(__int64_t*)fSize % 256;
		
		
		for(int i = 0;i<*(__int64_t*)fSize-last;i+=256){
			filemsg fullFm =filemsg(1*i,256);// no truncate;
			char* Msg = reinterpret_cast<char *> (&fullFm);
			vector<char> msgBB(Msg,Msg+sizeof(fullFm));//vector Message for the Buffer
			B.push(msgBB);
			//cout<<i<<endl;
		}
		if(last != 0){
			filemsg fullFm =filemsg(*(__int64_t*)fSize-last,last);// no truncate; 	
			char* Msg = reinterpret_cast<char *> (&fullFm);
			vector<char> msgBB(Msg,Msg+sizeof(fullFm));//vector Message for the Buffer
			B.push(msgBB);
		}
	
		MESSAGE_TYPE q = QUIT_MSG;
		char* ex = reinterpret_cast<char *> (&q);
		vector<char> msgBB(ex,ex+sizeof(q));
		B.push(msgBB);
		
	}
}
void *worker_function(int n,BoundedBuffer& B,HistogramCollection* H,string file, NRC* C,string* fName)
{
    /*
		Functionality of the worker threads	
    */
	bool exit = false;
	vector<char> Req ;	
	while (!exit){
		Req = B.pop();
	
		char* pog = reinterpret_cast<char *>(Req.data());
	
		MESSAGE_TYPE* a = reinterpret_cast<MESSAGE_TYPE *>(pog);
		//cout<<*a<<"AYY LMAO"<<endl;
		if(*a == 3){
			//exit;
			B.push(Req); //Push the Exit message for the other threads
			exit = true;
		}
		else if(*a == 0){
			//data message
			datamsg* PogU = (datamsg*) pog;
			//cout<<"PogU"<<endl;
			datamsg dm = *PogU;
			//cout<<(int)dm.person<<"p,   s: "<<(double)dm.seconds<<"  ecgn: "<<(int)dm.ecgno<<endl;
			C->cwrite((char*)&dm,sizeof(dm));
			char* data = C->cread();
		
			//mutex lock inside the  histogram update function
			H->update((int)dm.person,*(double*)data);
		
		}
		else if(*a == 1){
			//file message
			filemsg* PogU = (filemsg*) pog;
			filemsg fullFm = *PogU;
			
			int off = fullFm.offset;
			int size = fullFm.length;
			char* msgBuff2 = new char[sizeof(fullFm)+fName->length()+1];
			memcpy(msgBuff2,&fullFm,sizeof(fullFm));                        //This is working
			strcpy(msgBuff2+sizeof(fullFm),fName->c_str());
			C->cwrite(msgBuff2,sizeof(fullFm)+sizeof(*fName)+1);
			char* buffer2 = C->cread();
			
			//char Q = NULL;
			char* q = new char[size];
			string file = "received/y"+*fName;
			int f_write = open(file.c_str(), O_RDWR); 
			//cout<<f_write<<endl;
			if ((buffer2 != NULL) && (buffer2[0] == '\0')) {
				//cout<<size<<endl<<"maybe?"<<endl;
					lseek(f_write, off, SEEK_SET);
					write(f_write,q, size); 
					//write (f_read, NULL, 1); 
						//My function passed null as empty not as the char 							
			}
			else {
				//cout<<buffer2<<" so close dud"<<endl;
				lseek(f_write, off, SEEK_SET);
				write(f_write,buffer2, size); 
			}
			close(f_write); 
			delete q;
			delete msgBuff2;
		}
		else{
			//unkown message
			//abort 
			exit = true;
		}
	}
	
	MESSAGE_TYPE q = QUIT_MSG;
	C->cwrite((char *) &q,sizeof(MESSAGE_TYPE));
	delete C;
}

int main(int argc, char *argv[])
{
    int n = 2000;    //default number of requests per "patient"
    int p = 1;     // number of patients [1,15]
    int w = 100;    //default number of worker threads
    int b = 50; 	// default capacity of the request buffer, you should change this default
	int m = MAX_MESSAGE; 	// default capacity of the file buffer
    string port = "50000"; //default port number
	string host_name ="";
	srand(time_t(NULL));
    
    
   
    
	//NRC* chan = new NRC("control", NRC::CLIENT_SIDE);
    
// //	datamsg dm1 = datamsg(1,20.00,1);
	// MESSAGE_TYPE dm1 = QUIT_MSG;
	// char* pog = reinterpret_cast<char *>(&dm1);
	// MESSAGE_TYPE* a = reinterpret_cast<MESSAGE_TYPE *>(pog);
	
	// cout<<"msg type" << *a<<endl;
	int M;
	string Name ="1.csv";
	bool fileRequest = false;
	while(( M =getopt(argc,argv,"p:n:w:f:b:r:h:")) != -1){
	  switch (M){
		  case 'p' :
			p = stoi(optarg);
			break;
		  case 'n' :
			n = stoi(optarg);
			break;
		  case 'w' :
			w = stoi(optarg);
			break;
		  case 'f' :
			fileRequest = true;
			Name =optarg;
			break;
		  case 'b' :
			 b = stoi(optarg);
			break;
		  case 'r' :
			 port = optarg;
			break;
		  case 'h' :
			 host_name = optarg;
			break;
			
	  }
	}
    struct timeval start, end;
    gettimeofday (&start, 0);
	
	//cout<<b<<"  DVFDFV"<<endl;
	BoundedBuffer request_buffer(b);
	HistogramCollection hc;
	HistogramCollection* H = &hc;
    /* Start all threads here */
	vector <thread> Producers;
	vector <thread> Consumers;
	vector <NRC*> ConsumersChan; //Store pointers to the Channels 
	mutex Pro; //lock for # of Producer done 
	int ProDone = 0;
	int* np = &n; 
	if(!fileRequest){
		//Populate the Producers 
		for(int i = 0;i<p;i++){
			Histogram* tempPatient = new Histogram(10,-2.00,2.00);
			hc.add (tempPatient);
			char* c;//another dummy variable that i could have avoided with the struct
			thread temp(patient_function,(i+1),np,ref(request_buffer),&ProDone,&Pro,p,0,&Name,c);
			Producers.push_back(move(temp));

		}
	
		//Populate the workers i.e Consumers
		for(int i = 0; i < w ; i++){
			
			// int a= 0;//dummy variable should have removed it 
			// MESSAGE_TYPE newChannnel = NEWCHANNEL_MSG;
	 
			// chan->cwrite ((char *) &newChannnel, sizeof (MESSAGE_TYPE));
			// char* name = chan->cread();
			    
			NRC* B = new NRC(host_name.c_str(), port.c_str());
			ConsumersChan.push_back(B);
		
			thread temp(worker_function,0,ref(request_buffer),ref(H),Name,ConsumersChan[i],&Name);
			Consumers.push_back(move(temp));
		}

		/* Join all threads here */
		for (thread & th : Producers)
		{
			th.join();
		}
		//When the last Producer is done, push an exit message
		for (thread & th : Consumers)
		{
			th.join();
		}
		
	}
	
	if(fileRequest){
		NRC* chan = new NRC(host_name.c_str(), port.c_str());
		string file = "received/y"+Name;
		int f_create = open(file.c_str(), O_CREAT);
		
		filemsg fm = filemsg(0,0);
		char* msgBuff = new char[sizeof(fm)+Name.length()+1]; 
	
		memcpy(msgBuff,&fm,sizeof(fm));                        //This is working
		strcpy(msgBuff+sizeof(fm),Name.c_str());

		chan->cwrite(msgBuff,sizeof(fm)+sizeof(Name)+1);

		char* buffer = chan->cread();
		
		MESSAGE_TYPE q = QUIT_MSG;
		chan->cwrite((char *) &q,sizeof(MESSAGE_TYPE));
		delete chan;
		//cout<<chan<<endl;
		delete msgBuff;
		
		//Create only one thread
		//send file size to it 
		thread temp(patient_function,0,np,ref(request_buffer),&ProDone,&Pro,p,1,&Name,buffer);
		//Same worker Creation for file and message modes
		for(int i = 0; i < w ; i++){
			// int a= 0;//dummy variable should have removed it 
			// MESSAGE_TYPE newChannnel = NEWCHANNEL_MSG;
	 
			// chan->cwrite ((char *) &newChannnel, sizeof (MESSAGE_TYPE));
			// char* name = chan->cread();
			    
			NRC* B = new NRC(host_name.c_str(), port.c_str());
			ConsumersChan.push_back(B);
		
			thread temp(worker_function,0,ref(request_buffer),ref(H),Name,ConsumersChan[i],&Name);
			Consumers.push_back(move(temp));
		}
		temp.join();
		for (thread & th : Consumers)
		{
			th.join();
		}
		
	}
	
	if(ProDone == p){
		ConsumersChan.clear();
	}
	hc.print ();   
    gettimeofday (&end, 0);

    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

}

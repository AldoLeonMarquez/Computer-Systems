	/*
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date  : 2/8/19
 */
#include "common.h"
#include "FIFOreqchannel.h"
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>

using namespace std;


int main(int argc, char *argv[]){
    int n = 100;    // default number of requests per "patient"
	int p = 15;		// number of patients
    srand(time_t(NULL));
	int a = fork();
	if(!a){
	char * argv[] = {"./dataserver",NULL};//execute server, no arguments 
		execvp(argv[0],argv);  
		
	}
	else{
	
    FIFORequestChannel chan ("control", FIFORequestChannel::CLIENT_SIDE);

    //sending a non-sense message, you need to change this
    string Name;
	int client,ecg=0;
	double time= -1.00;
	int M;
	bool channelRequest =false;
	bool fileRequest = false;
	bool dataRequest = false;
	
	while(( M =getopt(argc,argv,"p:t:e:f:c:")) != -1){
	  switch (M){
		  case 'p' :
			dataRequest = true;
			client = atoi(optarg);
			break;
		  case 't' :
			time = atof(optarg);
			break;
		  case 'e' :
			ecg = atoi(optarg);
			break;
		  case 'f' :
			fileRequest = true;
			Name =optarg;
			break;
		  case 'c' :
			channelRequest = true;
	  }
	}
  
    if(dataRequest && time != -1.00){
	//single data point 
	
	struct timeval start, end;
	double t;
	gettimeofday(&start, NULL);   //time
		
	datamsg dm = datamsg(client,time,ecg); //data point example
	chan.cwrite(&dm,sizeof(dm)+1); // This is Working 
	
	char* Pog = chan.cread();
	cout<<"Single Data Point: ";
	cout<<*(double*)Pog<<endl;
	
	gettimeofday(&end, NULL);
	t = (double) ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)) / 1000000.0;
	cout<<"Data Point Time: "<<t<<endl;
	
	}
	if(dataRequest && time == -1.00){
		struct timeval start, end;
		double t;
		gettimeofday(&start, NULL);   //time
		
		ofstream o;
		string outName = ("x"+to_string(client)+".csv");
		outName = "received/"+ outName;
		o.open(outName);
		for(double i = 0; i <59.996 ; i+=0.004){
			datamsg dm1 = datamsg(client,i,1);
			chan.cwrite(&dm1,sizeof(dm1));
			char* temp = chan.cread();
			datamsg dm2 = datamsg(client,i,2);
			chan.cwrite(&dm2,sizeof(dm2));
			char* temp2 = chan.cread();
			//cout<<i<<endl;
			o<<i<<", "<<*(double*)temp<<", "<<*(double*)temp2<<endl;
		}
		o.close();
		gettimeofday(&end, NULL);
		t = (double) 
		rom data points Time: "<<t<<endl;
	}
	
	if(fileRequest){
		
	struct timeval start, end;
	double t;
	gettimeofday(&start, NULL);   //time
	
	string FileName = Name;
	filemsg fm = filemsg(0,0);
	char* msgBuff = new char[sizeof(fm)+FileName.length()+1]; 
	
	memcpy(msgBuff,&fm,sizeof(fm));                        //This is working
	strcpy(msgBuff+sizeof(fm),FileName.c_str());

	chan.cwrite(msgBuff,sizeof(fm)+sizeof(FileName)+1);

	char* buffer = chan.cread();
	//cout<<*(__int64_t*)buffer<< "???"<<endl; //size of file
	
	delete msgBuff;
	ofstream out;
	string File = "received/y"+Name;

	out.open(File);
	int last = *(__int64_t*)buffer % 256;
		for(int i =0;i<*(__int64_t*)buffer-last;i+=256){
			filemsg fullFm =filemsg(1*i,256);// no truncate;
			char* msgBuff2 = new char[sizeof(fullFm)+FileName.length()+1]; 
			memcpy(msgBuff2,&fullFm,sizeof(fullFm));                        //This is working
			strcpy(msgBuff2+sizeof(fullFm),FileName.c_str());
			chan.cwrite(msgBuff2,sizeof(fullFm)+sizeof(FileName)+1);
			char* buffer2 = chan.cread();
			if ((buffer2 != NULL) && (buffer2[0] == '\0')) {
				for(int i= 0;i<256;i++){
					out<<'\0';           //My function passed null as empty not as the char 
				}
			}
			else{
			out<<buffer2;
			}
		
		}
//C:\Users\Aldo Leon\AppData\Local\Packages\CanonicalGroupLimited.UbuntuonWindows_79rhkp1fndgsc\LocalState\rootfs\home\aldo\Skeleton code\received
		if(last != 0){filemsg fullFm =filemsg(*(__int64_t*)buffer-last,last);// no truncate;
	
		char* msgBuff2 = new char[sizeof(fullFm)+FileName.length()+1]; 
		memcpy(msgBuff2,&fullFm,sizeof(fullFm));                        //This is working
		strcpy(msgBuff2+sizeof(fullFm),FileName.c_str());
		chan.cwrite(msgBuff2,sizeof(fullFm)+sizeof(FileName)+1);
		char* buffer2 = chan.cread();
		out<<buffer2;
		delete buffer2;
		delete msgBuff2;
		
		}
	out.close();
	gettimeofday(&end, NULL);
	t = (double) ((end.tv_sec * 1000000 + end.tv_usec) - (start.tv_sec * 1000000 + start.tv_usec)) / 1000000.0;
	cout<<"File Request Time: "<<t<<endl;
	
	}
	if(channelRequest){
		
		MESSAGE_TYPE newChannnel = NEWCHANNEL_MSG;
		chan.cwrite (&newChannnel, sizeof (MESSAGE_TYPE));
		char* name = chan.cread();
		FIFORequestChannel B (name, FIFORequestChannel::CLIENT_SIDE);
		
		datamsg dmB = datamsg(13,15.00,2); //data point example
		chan.cwrite(&dmB,sizeof(dmB)+1); // This is Working 
	
		char* Pog1 = B.cread();
		cout<<"Single Data Point: ";
		cout<<*(double*)Pog1<<endl;
		//exit new channel
		MESSAGE_TYPE m2 = QUIT_MSG;
		B.cwrite (&m2, sizeof (MESSAGE_TYPE));
	}
	
	
	
	
	//this part is to get the file size 
	//to get the full thing just do the same but change the second term for the obtained size 
	
	//New Channel   This is Working  
	
	// MESSAGE_TYPE newChannnel = NEWCHANNEL_MSG;
	 
	// chan.cwrite (&newChannnel, sizeof (MESSAGE_TYPE));
	// char* name = chan.cread();
	// FIFORequestChannel B (name, FIFORequestChannel::CLIENT_SIDE);
	
	// datamsg dm = datamsg(10,10.00,2); //data point example
	
	// B.cwrite(&dm,sizeof(dm)+1); // This is Working  
	// int* a;
	// char* Pog = B.cread(a);
	
	// cout<<*(double*)Pog<<endl;
	
	
    // closing the channel    
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite (&m, sizeof (MESSAGE_TYPE));
	wait(NULL);
	}
}

#ifndef Histogram_h
#define Histogram_h
#include <mutex>
#include <queue>
#include <string>
#include <vector>
using namespace std;

class Histogram {
private:
	
	vector<int> hist;
	int nbins;
	double start, end;
	mutex Safe; //for safe update 
public:
    Histogram(int, double, double);
	
	~Histogram();
	void update (double ); 		// updates the histogram
    vector<int> get_hist();		// returns the histogram
    int size ();
	vector<double> get_range ();
};

#endif 

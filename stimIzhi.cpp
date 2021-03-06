//********************************************************************
// Izhikevich neuron network for stimulation tests
// Author : Madhavun Candadai Vasu
//
// logNormal weights and connectivity read in from weights_logNorm.csv
// or generate random weights
//********************************************************************

#include <iostream>
#include <cstdlib>
#include <fstream>
#include <cmath>
#include <iomanip>
#include "cblas.h"

#define PI 3.14

const int Ne = 8; // number of excitatory neurons
const int Ni = 2; // number of inhibitory neurons
const int N = Ne+Ni;
const double spikeV = 30;

struct Neuron{
	//params
	double a;
	double b;
	double c;
	double d;

	double weights[N];

	double v,vbuf;
	double u,ubuf;
};

int main(){
	using namespace std;
	
	const int simTime(1000);
	double activity[N];
	srand(5231);

	Neuron n[N];
	int stim[2] = {3,4}; //  neurons that are stimulated - provide exactly 2 neurons. repeat if required.
	int stimTime = simTime; // stimulating for entire time of simulation

	// init params for excitatory neurons
	for(int i=0;i<Ne; i++){
		double re = rand()/float(RAND_MAX);
		n[i].a = 0.02;
		n[i].b = 0.2;
		n[i].c = -65+(15*re*re);
		n[i].d = 8-(6*re*re);

		n[i].v = -65;
		n[i].u = 0.2*-65; // b*v
		n[i].ubuf = n[i].u;
	}
	// init params for inhibitory neurons
	for(int i=Ne;i<N; i++){
		double ri = rand()/float(RAND_MAX);
		double b = 0.25-(0.05*ri);
		n[i].a = 0.02+(0.08*ri);
		n[i].b = b;
		n[i].c = -65;
		n[i].d = 2;

		n[i].v = -65;
		n[i].u = b*-65; //b*v
		n[i].ubuf = n[i].u;
	}

	//init weights 
	/*for (int i = 0; i < (N); i++)
	{
		// from excitatory neurons
		for (int j = 0; j < Ne; j++)
		{
			n[i].weights[j] = 10*(rand()/float(RAND_MAX));
			//cout << n[i].weights[j] << endl;
		}
		// from inhibitory neurons
		for (int j=Ne; j <N; j++){
			n[i].weights[j] = -rand()/float(RAND_MAX);
			//cout << n[i].weights[j] << endl;
		}
		// self weight
		//n[i].weights[i] = 0;
	}*/
	// read pre-generated weights
	ifstream wts("weights_logNorm.csv");
    string value;
    for(int i=0;i<N;i++){
    	for(int j=0;j<N;j++){
	    	getline(wts, value, ',');
	    	n[i].weights[j] = stod(value);
	    }
    }
    wts.close();

	ofstream izhiRaster;
	izhiRaster.open("izhiSpikes.csv");
	//simulate over time
	for(int t=1; t<simTime; t++){
		//cout << t << endl;
		for(int i=0;i<N;i++){
			// reset if fired
			if(n[i].v >= spikeV){
				activity[i] = 1;
				izhiRaster << t << ',' << i <<'\n'; 
				n[i].v = n[i].c;
				n[i].u += n[i].d;
			}
			else{
				activity[i] = 0;
			}
		}

		// update buffers 
		for(int i=0;i<N;i++){
			double in(0);
			// stimulating some excitatory neurons 
			if(t < stimTime){ 
				if(i== stim[0] || i == stim[1]){ // stimulating neurons
					//generate input
					//in = rand()/float(RAND_MAX)*5;
					//in = abs(cos(t*PI/180))*4;
					in = 10;
				}
				else{
					//in = rand()/float(RAND_MAX)*2;					
					//in = abs(cos(t*PI/180))*1;
				}
			}		
			double totalIn = in + cblas_ddot(N,activity,1,n[i].weights,1);
			double v = n[i].v, u = n[i].u; 
			n[i].vbuf = v+0.5*(0.04*v*v + 5*v + 140 - u + totalIn);
			n[i].vbuf = n[i].vbuf+0.5*(0.04*v*v + 5*v + 140 - n[i].u + totalIn);
			n[i].ubuf = u + n[i].a*(n[i].b*v - u);
		
		}

		// update activations from buffers
		for(int i=0;i<N;i++){
			n[i].v = n[i].vbuf;
			n[i].u = n[i].ubuf;
		}

	}

	izhiRaster.close();
	cout << "Done!" << endl;
}


// Some statistics calculations
#include "TH1F.h"
#include "TMath.h"
#include <vector>
#include <iostream>

double calculateExpectedSignificance(std::vector<double> signal, std::vector<double> background){
  
  double sterm=0;
  double logterms=0;
  unsigned int nchannel  = signal.size();

  if (background.size()!=nchannel) { 
  	std::cout << " background and signal vectors should be the same size!" << std::endl;
	return 0;
  }

  for (unsigned int i=0;i<nchannel;i++){
    if (!signal[i]>0) continue;
    logterms+=(signal[i]+background[i])*TMath::Log((signal[i]+background[i])/background[i]);
    sterm+=signal[i];
  }
  double sig =  1.4142*TMath::Sqrt(logterms - sterm);
  //std::cout << "Significance @ " << nchannel << " bins = " << sig << std::endl; 
  return sig;
}


double calculateExpectedSignificance(TH1F *signal, TH1F *background){

  int nchannel = signal->GetNbinsX();
  
  if (background->GetNbinsX()!=nchannel) { 
  	std::cout << " background and signal vectors should be the same size!" << std::endl;
	return 0;
  }

  std::vector<double> signal_v;
  std::vector<double> background_v;

  for (int b=1;b<=nchannel;b++){
    signal_v.push_back(signal->GetBinContent(b));
    background_v.push_back(background->GetBinContent(b));
  }

  double sig = calculateExpectedSignificance(signal_v,background_v);
  return sig;
   
}


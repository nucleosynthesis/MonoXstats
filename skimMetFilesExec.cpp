#define RUN_SIGNAL
//#define RUN_DIMU
//#define RUN_SINGLEMU

// Bunch of ROOT includes
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
#include "TH1F.h"
#include "TEventList.h"
#include "TLorentzVector.h"

#include <string>
#include <cassert>
#include <iostream>

using namespace std;

float luminosity(19.7);
TH1F  *reweightgenpt; 

void skim(std::string var, TTree *fullTree, TTree *newTree, std::string cutstr, bool isWeighted , bool reweighted){

  std::cout << newTree->GetName() << " " << cutstr.c_str() << std::endl;
  fullTree->Draw(">>cutlist",cutstr.c_str());
  TEventList *keep_points = (TEventList*)gDirectory->Get("cutlist");
  int nEntries = fullTree->GetEntries();

  float x;
  float weight, weight_in, weight_in_pu; 
  float catvar1,catvar2,catvar3,catvar4,catvar5;
  float tau1,tau2;

  float genpt;

  TLorentzVector *jet = new TLorentzVector();
  TBranch *brjet = fullTree->GetBranch("jet1");
  brjet->SetAddress(&jet);

  //TTree *newTree = new TTree(newname.c_str(),newname.c_str());
  newTree->Branch(var.c_str(),&x,Form("%s/Float_t",var.c_str()));
  newTree->Branch("weight",&weight,"weight/Float_t");
  newTree->Branch("jet1mprune",&catvar1,"jet1mprume/Float_t");
  newTree->Branch("jet1mtrim",&catvar2,"jet1mtrim/Float_t");
  newTree->Branch("jet1tau2o1",&catvar3,"jet1tau12o1/Float_t");
  newTree->Branch("jet1pt",&catvar4,"jet1pt/Float_t");
  newTree->Branch("jet1QGtag",&catvar5,"jet1QGtag/Float_t");
  
  fullTree->SetBranchAddress(var.c_str(),&x);
  fullTree->SetBranchAddress("scale1fb",&weight_in);
  fullTree->SetBranchAddress("puweight",&weight_in_pu);
  fullTree->SetBranchAddress("jet1mprune",&catvar1);
  fullTree->SetBranchAddress("jet1mtrim" ,&catvar2);
  fullTree->SetBranchAddress("jet1tau2" ,&tau2);
  fullTree->SetBranchAddress("jet1tau1" ,&tau1);
  fullTree->SetBranchAddress("jet1QGtag",&catvar5);

  //fullTree->SetBranchAddress("jet1"     ,&jet);

  if ( reweighted){
    	std::cout << "Reweighting For generated PT " << std::endl;
  	fullTree->SetBranchAddress("genvpt"     ,&genpt);
  }

  std::cout << " Filling Skim tree " << newTree->GetName() << std::endl;

  for (int evt=0;evt<nEntries;evt++){
    if ( !(keep_points->Contains(evt)) ) continue;
    fullTree->GetEntry(evt);
    if (isWeighted) weight = weight_in*weight_in_pu*luminosity;
    else weight = 1.;
    if (reweighted) { 
    	if (genpt<reweightgenpt->GetXaxis()->GetXmax() && genpt>reweightgenpt->GetXaxis()->GetXmin()) {
	  weight*=reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt));
	}
	//std::cout << "genpt = " << genpt << ", weight = " << reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt)) <<std::endl;
    }
    catvar3 = tau2/tau1;  // make the sub-structure variable
    //catvar4 = jet->Pt();
    //std::cout << jet->Pt() << std::endl; 
    newTree->Fill();
  }
  std::cout << " Finished skim " << std::endl;
  //return newTree; 
}
#ifdef __CINT__
int skimMetFilesExec(int cindex ){ //genlphi_1-genlphi_2") { 
#else
int main(int argc, char *argv[] ){ //genlphi_1-genlphi_2") { 
  if( argc < 2) {
	std::cout << " Run with  index (file) to run over" << std::endl;
	assert(0);
  }
  int cindex = atoi(argv[1]);
#endif

  std::string iVar = "metRaw";

  const int lN  = 14;
  const int lNc   = 8;
  const int lNslc = 9;

  assert(cindex<lN+lNc+lNslc);

  std::string *fString       = new std::string[lN+lNc+lNslc]; 
  std::string *fWeights      = new std::string[lN+lNc+lNslc];
  //TTree **lTree = new TTree*[lN+lNc+lNslc]; 
  std::string *lTree 		     = new std::string[lN+lNc+lNslc]; 
  TTree *lCutTree ; 
  bool isMC[lN+lNc+lNslc];
  // Full path
  std::string lName   = "/afs/cern.ch/work/p/pharris/public/bacon/prod/CMSSW_5_3_13/src/BaconAnalyzer/MJMITSelection/";

  // reweighting for Generated PT of the Z
  TFile *freweight = TFile::Open("reweightgenptZ.root","OPEN");
  reweightgenpt = (TH1F*)freweight->Get("reweight");

  // Signal Region
  lTree[0]     = (lName+ "met/DYNN.root");        isMC[0]     =   1    ;    fString[0]     = "DY";                     
  lTree[1]     = (lName+ "met/DYLL.root");        isMC[1]     =   1    ;    fString[1]     = "DYLL";                     
  lTree[2]     = (lName+ "met/W.root");           isMC[2]     =   1    ;    fString[2]     = "W";                     
  lTree[3]     = (lName+ "met/WHT.root");         isMC[3]     =   1    ;    fString[3]     = "WHT";                     
  lTree[4]     = (lName+ "met/TT.root");          isMC[4]     =   1    ;    fString[4]     = "TT";                    
  lTree[5]     = (lName+ "met/T.root");           isMC[5]     =   1    ;    fString[5]     = "T";                     
  lTree[6]     = (lName+ "met/ZZ.root");          isMC[6]     =   1    ;    fString[6]     = "ZZ";                    
  lTree[7]     = (lName+ "met/WW.root");          isMC[7]     =   1    ;    fString[7]     = "WW";                    
  lTree[8]     = (lName+ "met/WZ.root");          isMC[8]     =   1    ;    fString[8]     = "WZ";                    
  lTree[9]     = (lName+ "met/WH_bacon.root");    isMC[9]     =   1    ;    fString[9]     = "WH0";                   
  lTree[10]    = (lName+ "met/ZH_bacon.root");   isMC[10]    =   1    ;    fString[10]    = "ZH0";                   
  lTree[11]    = (lName+ "met/GGH_bacon.root");   isMC[11]    =   1    ;    fString[11]    = "GGH0";                  
  lTree[12]    = (lName + "met/VBF_bacon.root");  isMC[12]    =   1    ;    fString[12]    = "VBFH0";                 
  lTree[13]    = (lName +"met/MET.root");         isMC[13]    =   0    ;    fString[13]    = "data_obs";              
                                                                       
  // Dimuon control sample                                             
  lTree[14]     = (lName+ "dimu/DYDM.root");      isMC[14]     =  1    ;    fString[14]     = "DY_control_bkg_mc";                     
  lTree[15]     = (lName+ "dimu/DYLLDM.root");    isMC[15]     =  1    ;    fString[15]     = "Zvv_control_mc";                     
  lTree[16]     = (lName+ "dimu/TDM.root");       isMC[16]     =  1    ;    fString[16]     = "T_control_bkg_mc";                     
  lTree[17]     = (lName+ "dimu/TTDM.root");      isMC[17]     =  1    ;    fString[17]     = "TT_control_bkg_mc";                     
  lTree[18]     = (lName+ "dimu/WWDM.root");      isMC[18]     =  1    ;    fString[18]     = "WW_control_bkg_mc";                     
  lTree[19]     = (lName+ "dimu/WZDM.root");      isMC[19]     =  1    ;    fString[19]     = "WZ_control_bkg_mc";                     
  lTree[20]     = (lName+ "dimu/ZZDM.root");      isMC[20]     =  1    ;    fString[20]     = "ZZ_control_bkg_mc";                     
  lTree[21]     = (lName+ "dimu/METDM.root");     isMC[21]     =  0    ;    fString[21]     = "Zvv_control";                    


  // Single lepton control sample  
  lTree[22]     = (lName+"lep/ZZL.root");          isMC[22]    =  1      ; fString[22]    = "ZZ_sl_control_bkg_mc";   
  lTree[23]     = (lName+"lep/WZL.root");          isMC[23]    =  1      ; fString[23]    = "WZ_sl_control_bkg_mc";                  
  lTree[24]     = (lName+"lep/DYLLL.root");        isMC[24]    =  1      ; fString[24]    = "DY_sl_control_bkg_mc";                   
  lTree[25]     = (lName+"lep/WL.root");           isMC[25]    =  1     ; fString[25]    = "Wlv_control_mc_1";                    
  lTree[26]     = (lName+"lep/WHTL.root");         isMC[26]    =  1     ; fString[26]    = "Wlv_control_mc_2";                 
  lTree[27]     = (lName+"lep/TTL.root");          isMC[27]    =  1      ; fString[27]    = "TT_sl_control_bkg_mc";                  
  lTree[28]     = (lName+"lep/TL.root");           isMC[28]    =  1      ; fString[28]    = "T_sl_control_bkg_mc";                   
  lTree[29]     = (lName+"lep/WWL.root");          isMC[29]    =  1      ; fString[29]    = "WW_sl_control_bkg_mc";                  
  lTree[30]  	= (lName+"lep/METL.root");         isMC[30]    =  0      ; fString[30]    = "Wlv_control";                   

  // Check files were opened 
  //for(int i0 = 0; i0 < lN+lNc+lNslc-1; i0++) {
  //  if ( !(lTree[i0]) ) {
  //    std::cout << "Tree for following sample not loaded, check file! " << fString[i0] << std::endl;
  //    assert(0);
  //  }
 // }

  // Define the cuts to be applied for the analysis (signal region)
  //for(int i0 = 0; i0 < lN;     i0++) fWeights[i0]  += "(njets < 4 && nbtags > 1 && metFiltersWord == 0 && trigger > 0 && jet1.Pt() > 250 && jet1NHF < 0.9 && jet1NEMF < 0.9 && jet1mtrim > 70 )";
  for(int i0 = 0; i0 < lN+lNc+lNslc;     i0++) fWeights[i0]  += "(trigger > 0 && metRaw > 200 && metFiltersWord == 0  && njets < 3  && jet1.Pt() > 110  && abs(jet1.Eta()) < 2.0 && jet1NHF < 0.7 && jet1NEMF < 0.7  && jet1CHF>0.2 && jdphi<2.0) "; //&& jet1NHF < 0.9 && jet1NEMF < 0.9 && jet1mtrim > 70 )";
  for(int i0 = 0; i0 < lN;      i0++) fWeights[i0] += "&& ( ntaus == 0 && nphotons == 0 && nelectrons == 0)";//*(dimu.M() > 60 && dimu.M() < 120) ";
  for(int i0 = 0; i0 < 8;      i0++) fWeights[i0]  += "&& ( nmuons == 0 ) ";//*(dimu.M() > 60 && dimu.M() < 120) ";
  for(int i0 = 9; i0 < 11;      i0++) fWeights[i0] += "&& ( nmuons == 2 ) ";//*(dimu.M() > 60 && dimu.M() < 120) "; // No H invisible sample so use another one and add Hdecay to the MET
  for(int i0 = 11; i0 < lN;      i0++) fWeights[i0] += "&& ( nmuons == 0 ) ";//*(dimu.M() > 60 && dimu.M() < 120) ";
  //for(int i0 = 0; i0 < lN-1;   i0++) fWeights[i0] += "*scale1fb*19.7";

  fWeights[2]    += "&&(genvpt < 300)";
  fWeights[3]    += "&&(genvpt > 300)";

  // Control sample (Di muon) (dimu is added to the met)
  for(int i0 = lN; i0 < lN+lNc; i0++) fWeights[i0] += "&& (nmuons == 2 && ntaus == 0 && nphotons == 0 && nelectrons == 0)";//*(dimu.M() > 60 && dimu.M() < 120) ";

  // Control sample (Single Muon)
  for(int i0 = lN+lNc; i0 < lN+lNc+lNslc; i0++) fWeights[i0] += "&& (nmuons == 1 && ntaus == 0 && nphotons == 0 && nelectrons == 0)";//*(dimu.M() > 60 && dimu.M() < 120) ";
  fWeights[25]    += "&&(genvpt < 300)";
  fWeights[26]    += "&&(genvpt > 300)";

 
  bool reweightpt = false;
  if (cindex==0) reweightpt=true;


  TFile *lFile   = new TFile("Output_tree.root","RECREATE");
  TTree *cutTree = new TTree(fString[cindex].c_str(),fString[cindex].c_str());
  cutTree->SetDirectory(lFile);
 
  TFile *gInFilePointer = new TFile(lTree[cindex].c_str(),"OPEN");
  TTree *thisTree  = (TTree*) gInFilePointer->Get("Tree");
  //lFile->cd();
  //gInFilePointer->cd();
  skim(iVar,thisTree,cutTree,fWeights[cindex],isMC[cindex]/*isWeighted*/,reweightpt);  // Skim makes a RooDataSet from the Cut tree
  //lCutTree->Write();
 
  //lFile->Close();
  //lH[1]->Divide(lH[0]);
  return 0;
}

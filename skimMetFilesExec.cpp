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
#include "TVector3.h"
#include "Math/LorentzVector.h"
#include "Math/PtEtaPhiM4D.h"
#include "Math/PxPyPzM4D.h"
#include <string>
#include <cassert>
#include <iostream>

using namespace std;

float luminosity(19700);
TH1F  *reweightgenpt; 

double deltaphi(double phi1, double phi2){
        // do somthing to sort out delta R??
        double mpi = TMath::Pi();
        double res = phi1 - phi2;
        while (res > mpi) res-= 2*mpi;
        while (res <= -mpi) res+= 2*mpi;
        return res;

}

void skim(std::string var, TTree *fullTree, TTree *newTree, std::string cutstr, bool isWeighted , bool reweighted, bool addDiMuSel){


  std::cout << fullTree->GetName() << " -> " <<  newTree->GetName() << " " << cutstr.c_str() << std::endl;
  fullTree->Draw(">>cutlist",cutstr.c_str());
  TEventList *keep_points = (TEventList*)gDirectory->Get("cutlist");
  int nEntries = fullTree->GetEntries();

  float x;
  float weight, weight_in, weight_in_pu, metRaw; 
  float catvar1,catvar2,catvar3,catvar4,catvar5;
  float tau1,tau2;
  float dimuM;
  float genpt;
  int passVBF;

  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> > *lepV   = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> >();
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> > *lep2V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> >();

  TBranch *brlep   = fullTree->GetBranch("lep1");
  TBranch *brlep_2 = fullTree->GetBranch("lep2");
  brlep->SetAddress(&lepV);
  brlep_2->SetAddress(&lep2V);

  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> > *jetV   = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> >();
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> > *jet2V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzM4D<double> >();

  TBranch *brjet   = fullTree->GetBranch("jet1");
  TBranch *brjet_2 = fullTree->GetBranch("jet2");
  brjet->SetAddress(&jetV);
  brjet_2->SetAddress(&jet2V);

  //TTree *newTree = new TTree(newname.c_str(),newname.c_str());
  newTree->Branch("mvamet",&x,"mvamet/Float_t");
  newTree->Branch("weight",&weight,"weight/Float_t");
  newTree->Branch("metRaw",&metRaw,"metRaw/Float_t"); // useful to keep this
  newTree->Branch("jet1mprune",&catvar1,"jet1mprume/Float_t");
  newTree->Branch("jet1mtrim",&catvar2,"jet1mtrim/Float_t");
  newTree->Branch("jet1tau2o1",&catvar3,"jet1tau12o1/Float_t");
  newTree->Branch("jet1pt",&catvar4,"jet1pt/Float_t");
  newTree->Branch("jet1QGtag",&catvar5,"jet1QGtag/Float_t");
  newTree->Branch("passVBF",&passVBF,"passVBF/Int_t");
  if (addDiMuSel) {	
    newTree->Branch("dimu_m",&dimuM,"dimu_m/Float_t");
  }

  fullTree->SetBranchAddress(var.c_str(),&x);
  fullTree->SetBranchAddress("weight",&weight_in);
  fullTree->SetBranchAddress("metRaw",&metRaw);
  if (isWeighted) fullTree->SetBranchAddress("puWeight",&weight_in_pu);
  //fullTree->SetBranchAddress("jet1mprune",&catvar1);
  //fullTree->SetBranchAddress("jet1mtrim" ,&catvar2);
  //fullTree->SetBranchAddress("jet1tau2" ,&tau2);
  //fullTree->SetBranchAddress("jet1tau1" ,&tau1);
  //fullTree->SetBranchAddress("jet1QGtag",&catvar5);

  //fullTree->SetBranchAddress("jet1"     ,&jet);
  
  
  if ( reweighted){
    	std::cout << "Reweighting For generated PT " << std::endl;
  	fullTree->SetBranchAddress("genvpt"     ,&genpt);
  }

  std::cout << " Filling Skim tree " << newTree->GetName() << std::endl;

  for (int evt=0;evt<nEntries;evt++){
    if ( !(keep_points->Contains(evt)) ) continue;

    fullTree->GetEntry(evt);

    // Fuck the millions of differnt lorentz vectors we have!
    TLorentzVector jet, jet2;
    jet.SetPtEtaPhiM(jetV->Pt(),jetV->Eta(),jetV->Phi(),jetV->M());
    jet2.SetPtEtaPhiM(jet2V->Pt(),jet2V->Eta(),jet2V->Phi(),jet2V->M());
 
    if (addDiMuSel){
      TLorentzVector lep, lep2;
      lep.SetPxPyPzE(lepV->Px(),lepV->Py(),lepV->Pz(),lepV->E());
      lep2.SetPxPyPzE(lep2V->Px(),lep2V->Py(),lep2V->Pz(),lep2V->E());
      dimuM = TMath::Sqrt(2*0.1 + 2*(lep.E()*lep2.E()-(lep.Px()*lep2.Px()+lep.Py()*lep2.Py()+lep.Pz()*lep2.Pz())));   // -> this doesn't work ?! just spits out nonsense, selection done with Draw
      dimuM = 0;
      //std::cout << "diMuon of course>!?!>!? " << dM << std::endl;
      //if ( dimuM < 61 || dimuM > 121 ) continue;     
    }

    //std::cout << " Entry " << evt << ", " << jetV->Pt() << std::endl;
    if (jet2.Pt() > 30 && TMath::Abs(jet2.Eta()) < 2.4){
      if  (deltaphi(jet.Phi(),jet2.Phi()) > 2) continue;
    }
    if (isWeighted) weight = weight_in*weight_in_pu*luminosity;
    //if (isWeighted) weight = weight_in*luminosity;
    else weight = 1.;
    if (reweighted) { 
    	if (genpt<reweightgenpt->GetXaxis()->GetXmax() && genpt>reweightgenpt->GetXaxis()->GetXmin()) {
	  weight*=reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt));
	}
	//std::cout << "genpt = " << genpt << ", weight = " << reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt)) <<std::endl;
    }
    catvar3 = tau2/tau1;  // make the sub-structure variable
    catvar4 = jet.Pt();
    //std::cout << jet->Pt() << std::endl;

    // Interesting is to add the VBF selection and then flag if it is passed or not
    // is there a second jet 
    // pt>50, eta <4.7, eta1*eta2 < 0 , delta(Eta) > 4.2, Mjj > 1100, deltaPhi < 1.0
    passVBF=0;
    //std::cout << " now look, could be VBF!" <<  std::endl;
    if (jet2.Pt() > 50 && TMath::Abs(jet2.Eta()) < 4.7 ) {
      double j1eta = jet.Eta();
      double j2eta = jet2.Eta();
      //std::cout << "could be VBF!" <<  j1eta << ", " << j2eta << ", "<< deltaphi(jet.Phi(),jet2.Phi())<< ", "<< (jet+jet2).M()  << std::endl;
      if( j1eta*j2eta <0  && deltaphi(jet.Phi(),jet2.Phi()) < 1.0 && (jet+jet2).M() > 1100 && TMath::Abs(j1eta-j2eta) > 4.2) {
	passVBF=1;
      }
    }
    // finally cut on jet dphi since its not in the trees
    newTree->Fill();
  }
  std::cout << " Finished skim " << std::endl;
  //return newTree; 
}
#ifdef __CINT__
int skimMetFilesExec(int cindex, std:string name ){ //genlphi_1-genlphi_2") { 
#else
int main(int argc, char *argv[] ){ //genlphi_1-genlphi_2") { 
  if( argc < 2) {
	std::cout << " Run with  index (file) to run over" << std::endl;
	assert(0);
  }
  int cindex = atoi(argv[1]);
  std::string name;
  for(int p = 2; p < argc; p++){
   name += argv[p];
  }
#endif

  std::cout << cindex << ", " << name << std::endl;
  std::string iVar = "mvamet";

  const int lN    = 11;  // number of MET samples
  const int lNc   = 7;   // number of Di-Muon 
  const int lNslc = 9;   // number of Single Muon 

  assert(cindex<lN+lNc+lNslc);

  std::string *fString       = new std::string[lN+lNc+lNslc]; 
  std::string *fWeights      = new std::string[lN+lNc+lNslc];
  //TTree **lTree = new TTree*[lN+lNc+lNslc]; 
  std::string *lTree 	     = new std::string[lN+lNc+lNslc]; 
  TTree *lCutTree ; 
  // Full path
  //std::string lName   = "/afs/cern.ch/work/p/pharris/public/bacon/prod/CMSSW_5_3_13/src/BaconAnalyzer/MJMITSelection/";
  std::string lName   = "root://eoscms//eos/cms/store/cmst3/user/pharris/Marco/";

  // reweighting for Generated PT of the Z
  //TFile *freweight = TFile::Open("/afs/cern.ch/work/n/nckw/monojet/monojetModelBuilder/reweightgenptZ.root","OPEN");
  //reweightgenpt = (TH1F*)freweight->Get("reweight");

  // Signal Region
  fString[0]     = "DY";                     
  fString[1]     = "W";                     
  fString[2]     = "WHT";                     
  fString[3]     = "TT";                    
  fString[4]     = "T";                     
  fString[5]     = "ZZ";                    
  fString[6]     = "WW";                    
  fString[7]     = "WZ";                    
  fString[8]     = "GGH0";                  
  fString[9]     = "VBFH0";                 
  fString[10]    = "data_obs";              
 
  // Dimuon
  fString[11]    = "Zvv_control_mc";                     
  fString[12]    = "T_control_bkg_mc";                     
  fString[13]    = "TT_control_bkg_mc";                     
  fString[14]    = "WW_control_bkg_mc";                     
  fString[15]    = "WZ_control_bkg_mc";                     
  fString[16]    = "ZZ_control_bkg_mc";                     
  fString[17]    = "Zvv_control";                    
  
  // Single muon
  fString[18]    = "ZZ_sl_control_bkg_mc";   
  fString[19]    = "WZ_sl_control_bkg_mc";                  
  fString[20]    = "DY_sl_control_bkg_mc";                   
  fString[21]    = "Wlv_control_mc_1";                    
  fString[22]    = "Wlv_control_mc_2";                 
  fString[23]    = "TT_sl_control_bkg_mc";                  
  fString[24]    = "T_sl_control_bkg_mc";                   
  fString[25]    = "WW_sl_control_bkg_mc";                  
  fString[26]    = "Wlv_control";                   

  // Define the cuts to be applied for the analysis (signal region)
  for(int i0 = 0; i0 < lN+lNc+lNslc;     i0++) fWeights[i0]  += "(trigger > 0  && njets < 3  && jet1.Pt() > 110  && abs(jet1.Eta()) < 2.4 && jet1NHF < 0.7 && jet1NEMF < 0.7  && jet1CHF>0.2) "; //&& jet1NHF < 0.9 && jet1NEMF < 0.9 && jet1mtrim > 70 )";
  for(int i0 = 0; i0 < lN;      i0++) fWeights[i0]  += "&& ( ntaus == 0 && nphotons == 0 && nlep == 0)";//*(dimu.M() > 60 && dimu.M() < 120) ";

  // Control sample (Di lep) (dimu is added to the met)
  for(int i0 = lN; i0 < lN+lNc; i0++) fWeights[i0] += "&& (nlep == 2 && ntaus == 0 && nphotons == 0 ) && (TMath::Sqrt(2*(lep1.E()*lep2.E()-(lep1.Px()*lep2.Px()+lep1.Py()*lep2.Py()+lep1.Pz()*lep2.Pz()))) < 121 && TMath::Sqrt(2*(lep1.E()*lep2.E()-(lep1.Px()*lep2.Px()+lep1.Py()*lep2.Py()+lep1.Pz()*lep2.Pz()))) > 61 )";//*(dimu.M() > 60 && dimu.M() < 120) ";

  // Control sample (Single Muon)
  for(int i0 = lN+lNc; i0 < lN+lNc+lNslc; i0++) fWeights[i0] += "&& (nlep == 1 && ntaus == 0 && nphotons == 0 ) && (TMath::Sqrt(2*mvamet*lep1.Pt()*(1-TMath::Cos(mvametPhi-lep1.Phi()))) > 50 && TMath::Sqrt(2*mvamet*lep1.Pt()*(1-TMath::Cos(mvametPhi-lep1.Phi()))) < 100 ) ";//*(dimu.M() > 60 && dimu.M() < 120) ";  // use original met to calculate the MT guy!!!


  bool addDiMuSel = false;
  std::string TreeName;

  if  	   (cindex < lN ) 				{ TreeName = "tree";	  addDiMuSel = false;  iVar = "mvamet";}
  else if  (cindex >= lN && cindex < lN+lNc) 		{ TreeName = "treeZll";   addDiMuSel = true;   iVar = "mvametCorZ";}
  else if  (cindex >= lN+lNc && cindex < lN+lNc+lNslc ) { TreeName = "treeWln";   addDiMuSel = false;  iVar = "mvametCorW";}

 
  TFile *gInFilePointer = TFile::Open(name.c_str());
  TTree *thisTree  = (TTree*) gInFilePointer->Get(TreeName.c_str());

  TFile *lFile   = new TFile("Output_tree.root","RECREATE");
  TTree *cutTree = new TTree(fString[cindex].c_str(),fString[cindex].c_str());
  cutTree->SetDirectory(lFile);

//gInFilePointer->cd();
  bool isMC = 1;
  if (cindex==10 || cindex==17 || cindex==26){
    isMC=0;
  }

  lFile->cd();
  skim(iVar,thisTree,cutTree,fWeights[cindex],isMC/*isWeighted*/,false/*do the ptWeighting*/,addDiMuSel);  // Skim makes a RooDataSet from the Cut tree
  cutTree->Write();
 
  lFile->Close();
  //lH[1]->Divide(lH[0]);
  return 0;
}

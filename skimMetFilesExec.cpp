#define RUN_SIGNAL
//#define RUN_DIMU
//#define RUN_SINGLEMU

// Bunch of ROOT includes
#include "TTree.h"
#include "TBranch.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TEventList.h"
#include "TLorentzVector.h"
#include "TVector3.h"
#include "Math/LorentzVector.h"
#include "Math/PtEtaPhiM4D.h"
#include "Math/PxPyPzE4D.h"
#include <string>
#include <cassert>
#include <iostream>

using namespace std;

float luminosity(19700);
TH2F  *reweightpheff; 

double deltaphi(double phi1, double phi2){
        // do somthing to sort out delta R??
        double mpi = TMath::Pi();
        double res = phi1 - phi2;
        while (res > mpi) res-= 2*mpi;
        while (res <= -mpi) res+= 2*mpi;
        return TMath::Abs(res);
}

double computeMET(const double met, const double metphi, const TLorentzVector &photon)
{
  double metx = met*cos(metphi) + photon.Px();
  double mety = met*sin(metphi) + photon.Py();
  TLorentzVector mvec(metx, mety, 0, 0);
  return mvec.Pt();
}

double computeMETPhi(const double met, const double metphi, const TLorentzVector &photon)
{
  double metx = met*cos(metphi) + photon.Px();
  double mety = met*sin(metphi) + photon.Py();
  TLorentzVector mvec(metx, mety, 0, 0);
  return mvec.Phi();
}


void skim(std::string var, TTree *fullTree, TTree *newTree, std::string cutstr, bool isWeighted , bool reweighted, bool addDiMuSel, bool addMuSel, bool addPhoSel){


  std::cout << fullTree->GetName() << " -> " <<  newTree->GetName() << " " << cutstr.c_str() << std::endl;
  fullTree->Draw(">>cutlist",cutstr.c_str());
  TEventList *keep_points = (TEventList*)gDirectory->Get("cutlist");
  int nEntries = fullTree->GetEntries();

  float x,x_orig;
  float mvametPhi, metRaw;
  float mvametPhiCor ;// corected phis for deltaPhi(met,jet) cut (// NOT to be used in calculation of MT!)
  float weight, weight_wpt, weight_in, weight_in_pu; 
  float catvar1,catvar2,catvar3,catvar4,catvar5,catvar6;   // These are empty, do as we want with them 
  float tau1,tau2;
  float dimuM, mt;	// used for the W/Z selection
  int passVBF;
  unsigned int njets;
  unsigned int nphotons;
  float phopt, phoeta;
  // Why do we use these?
  float metBasicPhi, metBasic;

 
  // Used for reweighting if needed
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *genZ  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();
  TBranch *brgenZ ;

  // W/Z selection for leptions
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *lepV   = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *lep2V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();

  TBranch *brlep   = fullTree->GetBranch("lep1");
  TBranch *brlep_2 = fullTree->GetBranch("lep2");
  brlep->SetAddress(&lepV);
  brlep_2->SetAddress(&lep2V);

  // Check VBF selection 
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *jetV   = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *jet2V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();

  // Also need these for second jet veto 
  TBranch *brjet   = fullTree->GetBranch("jet1");
  TBranch *brjet_2 = fullTree->GetBranch("jet2");
  brjet->SetAddress(&jetV);
  brjet_2->SetAddress(&jet2V);


  // if addPhoSel add the two photons
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *pho1V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();
  ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> > *pho2V  = new ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >();
  TBranch *brpho_1 ;
  TBranch *brpho_2 ;
  if (addPhoSel){
    brpho_1 = fullTree->GetBranch("pho1");
    brpho_2 = fullTree->GetBranch("pho2");    
    brpho_1->SetAddress(&pho1V);
    brpho_2->SetAddress(&pho2V);
  }


  // mu/dimu ?
  std::string origVar = var;
  if (addMuSel)   var+="CorW";
  if (addDiMuSel) var+="CorZ";

  // Add Output branches to our very skimmed down tree.
  //TTree *newTree = new TTree(newname.c_str(),newname.c_str());

  newTree->Branch("mvamet",&x,"mvamet/Float_t");
  newTree->Branch("mvamet_orig",&x_orig,"mvamet_orig/Float_t");
  newTree->Branch("weight",&weight,"weight/Float_t");
  newTree->Branch("weight_wpt",&weight_wpt,"weight_wpt/Float_t");
  newTree->Branch("metRaw",&metRaw,"metRaw/Float_t"); // useful to keep this
  newTree->Branch("jet1mprune",&catvar1,"jet1mprume/Float_t");
  newTree->Branch("jet1mtrim",&catvar2,"jet1mtrim/Float_t");
  newTree->Branch("jet1tau2o1",&catvar3,"jet1tau12o1/Float_t");
  newTree->Branch("jpt",&catvar4,"jpt/Float_t");
  newTree->Branch("jet1eta",&catvar6,"jet1eta/Float_t");
  newTree->Branch("jet1QGtag",&catvar5,"jet1QGtag/Float_t");
  newTree->Branch("passVBF",&passVBF,"passVBF/Int_t");
  newTree->Branch("njets",&njets,"njets/Int_t");

  if (addDiMuSel) {	
    newTree->Branch("dimu_m",&dimuM,"dimu_m/Float_t");
  }
  if (addMuSel) {	
    newTree->Branch("mt",&mt,"mt/Float_t");
  }
  if (addPhoSel) {	
    newTree->Branch("phopt",&phopt,"phopt/Float_t");
    newTree->Branch("phoeta",&phoeta,"phoeta/Float_t");
  }

  fullTree->SetBranchAddress(var.c_str(),&x);
  if (addDiMuSel || addMuSel) fullTree->SetBranchAddress(origVar.c_str(),&x_orig);
  fullTree->SetBranchAddress(Form("%sPhi",var.c_str()),&mvametPhiCor);
  if (addDiMuSel || addMuSel) fullTree->SetBranchAddress(Form("%sPhi",origVar.c_str()),&mvametPhi);
  if (isWeighted) fullTree->SetBranchAddress("weight",&weight_in);
  fullTree->SetBranchAddress("metRaw",&metRaw);
  fullTree->SetBranchAddress("njets",&njets);
  fullTree->SetBranchAddress("nphotons",&nphotons);


  if (isWeighted) fullTree->SetBranchAddress("puWeight",&weight_in_pu);
  //fullTree->SetBranchAddress("jet1mprune",&catvar1);
  //fullTree->SetBranchAddress("jet1mtrim" ,&catvar2);
  //fullTree->SetBranchAddress("jet1tau2" ,&tau2);
  //fullTree->SetBranchAddress("jet1tau1" ,&tau1);
  //fullTree->SetBranchAddress("jet1QGtag",&catvar5);
  //fullTree->SetBranchAddress("jet1"     ,&jet);
   
  if ( reweighted){

    	std::cout << "Reweighting For Photon Efficiency! " << std::endl;
  }

  std::cout << " Filling Skim tree " << newTree->GetName() << std::endl;

  for (int evt=0;evt<nEntries;evt++){
    if ( !(keep_points->Contains(evt)) ) continue;

    fullTree->GetEntry(evt);

    TLorentzVector jet, jet2;
    jet.SetPtEtaPhiM(jetV->Pt(),jetV->Eta(),jetV->Phi(),jetV->M());
    jet2.SetPtEtaPhiM(jet2V->Pt(),jet2V->Eta(),jet2V->Phi(),jet2V->M());
 
    if (addDiMuSel){
      TLorentzVector lep, lep2;
      lep.SetPxPyPzE(lepV->Px(),lepV->Py(),lepV->Pz(),lepV->E());
      lep2.SetPxPyPzE(lep2V->Px(),lep2V->Py(),lep2V->Pz(),lep2V->E());
      dimuM = TMath::Sqrt(2*(lep.E()*lep2.E()-(lep.Px()*lep2.Px()+lep.Py()*lep2.Py()+lep.Pz()*lep2.Pz()))); 
      if ( dimuM < 61 || dimuM > 121 ) continue;     
    }

    if (addMuSel){
      TLorentzVector lep;
      lep.SetPxPyPzE(lepV->Px(),lepV->Py(),lepV->Pz(),lepV->E());
      mt = TMath::Sqrt(2*x_orig*lep.Pt()*(1-TMath::Cos(deltaphi(mvametPhi,lep.Phi()))));  // used original metphi and met to calculate mt
      if ( mt < 50 || mt > 100 ) continue;     
    }

    if (addPhoSel){
      double pho2eta = TMath::Abs(pho2V->Eta());
      if (nphotons>1 && pho2V->Pt() > 100 && (pho2eta<1.4442 || ( pho2eta > 1.566 && pho2eta<2.5))) continue;
      TLorentzVector pho;
      pho.SetPxPyPzE(pho1V->Px(),pho1V->Py(),pho1V->Pz(),pho1V->E());
      x_orig = x;
      double metphocor = computeMET(x,mvametPhiCor,pho);  
      double metphophicor = computeMETPhi(x,mvametPhiCor,pho);  
      x = metphocor;    // Set the variables to the corrected ones now
      mvametPhiCor = metphophicor;
      phopt = pho.Pt();
      phoeta = pho.Eta();
    }
    // Back to regular selection
    // Add MET and second Jet balance cuts !!!
    if (jet2.Pt() > 30){
      if  (deltaphi(jet.Phi(),jet2.Phi()) >= 2) continue;
    }
    if ( deltaphi(jet.Phi(),mvametPhiCor)<2 ) continue;  // this will always be the corrected Phi 
    
    // Weights/Reweightsi
    if (isWeighted) weight = weight_in*weight_in_pu*luminosity;
    else  weight = 1.;
    if (reweighted && isWeighted) { 
        //float genpt = genZ->Pt();
	double phpt  = phopt;
	double pheta = TMath::Abs(phoeta);
    	if (
	    phpt<reweightpheff->GetXaxis()->GetXmax() && phpt > reweightpheff->GetXaxis()->GetXmin()
  	 && pheta<reweightpheff->GetYaxis()->GetXmax() && pheta > reweightpheff->GetYaxis()->GetXmin() 	
	   ) {
	  weight_wpt = weight*reweightpheff->GetBinContent(reweightpheff->FindBin(phpt,pheta));
	} else {
	  weight_wpt = weight;
	}
    } else {
	weight_wpt = weight;
    }
    catvar3 = tau2/tau1;  // make the sub-structure variable
    catvar4 = jet.Pt();
    catvar6 = jet.Eta();

    // Now check the VBF selection (added as a flag)
    // pt>50, eta <4.7, eta1*eta2 < 0 , delta(Eta) > 4.2, Mjj > 1100, deltaPhi < 1.0
    passVBF=0;
    if (jet2.Pt() > 50 && TMath::Abs(jet2.Eta()) < 4.7 ) {
      double j1eta = jet.Eta();
      double j2eta = jet2.Eta();
      if( j1eta*j2eta <0  && deltaphi(jet.Phi(),jet2.Phi()) < 1.0 && (jet+jet2).M() > 1100 && TMath::Abs(j1eta-j2eta) > 4.2) {
	passVBF=1;
      }
    }
    newTree->Fill();
  }
  std::cout << " Finished skim " << std::endl;
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
  const int lNpho = 2;   // number of Photon Samples 

  assert(cindex<lN+lNc+lNslc+lNpho);

  std::string *fString       = new std::string[lN+lNc+lNslc+lNpho]; 
  std::string *fWeights      = new std::string[lN+lNc+lNslc+lNpho];
  //TTree **lTree = new TTree*[lN+lNc+lNslc]; 
  std::string *lTree 	     = new std::string[lN+lNc+lNslc+lNpho]; 
  TTree *lCutTree ; 
  // Full path
  //std::string lName   = "/afs/cern.ch/work/p/pharris/public/bacon/prod/CMSSW_5_3_13/src/BaconAnalyzer/MJMITSelection/";
  std::string lName   = "root://eoscms//eos/cms/store/cmst3/user/pharris/Marco/";

  // reweighting for Generated PT of the Z
  TFile *freweight = TFile::Open("/afs/cern.ch/work/n/nckw/private/monojet/Photon_ID_CSEV_SF_Jan22rereco_Full2012_S10_MC_V01.root","OPEN");
  reweightpheff   = (TH2F*)freweight->Get("PhotonIDSF_MediumWP_Jan22rereco_Full2012_S10_MC_V01");
  TH2F *ecdafeeff = (TH2F*)freweight->Get("PhotonCSEVSF_MediumWP_Jan22rereco_Full2012_S10_MC_V01");
  reweightpheff->Multiply(ecdafeeff);

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

  // Photon + Jet
  fString[27]    = "PhoData";                  
  fString[28]    = "GJet";                   

  // Define the cuts to be applied for the analysis (signal region)
  for(int i0 = 0; i0 < lN+lNc+lNslc+lNpho;     i0++) fWeights[i0]  += " jet1CHF>0.2 && jet1NHF<0.7 && jet1NEMF<0.7 && TMath::Abs(jet1.Eta())<2.0 && (metFiltersWord==1023||metFiltersWord==511) && jet1.Pt()>150 && njets<3 ";
  for(int i0 = 0; i0 < lN+lNc+lNslc;     i0++) fWeights[i0]  += " && ((trigger&1)==1 || (trigger&2)==2) ";
  for(int i0 = 0; i0 < lN; i0++) 	       fWeights[i0]  += "&& ( ntaus == 0 && nphotons == 0 && nlep == 0)";

  // Control sample (Di lep) (dimu is added to the met)
  for(int i0 = lN; i0 < lN+lNc; i0++) fWeights[i0] += "&& (nlep == 2 && ntaus == 0 && nphotons == 0 ) && lep1IsIsolated && lid1+lid2==0 && TMath::Abs(lid1)==13 ";

  // Control sample (Single Muon)
  for(int i0 = lN+lNc; i0 < lN+lNc+lNslc; i0++) fWeights[i0] += "&& (nlep == 1 && ntaus == 0 && nphotons == 0 ) && TMath::Abs(lid1)==13 && lep1.Pt()>20 && lep1IsTightMuon==1 && lep1IsIsolated==1";

  for(int i0 = lN+lNc+lNslc; i0 < lN+lNc+lNslc+lNpho; i0++) fWeights[i0] += " && (nlep == 0 && ntaus == 0 && nphotons>0 )";

  bool addDiMuSel = false;
  bool addMuSel   = false;
  bool addPhoSel   = false;
  std::string TreeName;

  if  	   (cindex < lN ) 				{ TreeName = "tree";	  addDiMuSel = false;  addMuSel=false; addPhoSel=false;iVar = "met";}
  else if  (cindex >= lN && cindex < lN+lNc) 		{ TreeName = "treeZll";   addDiMuSel = true;   addMuSel=false; addPhoSel=false;iVar = "met";}
  else if  (cindex >= lN+lNc && cindex < lN+lNc+lNslc ) { TreeName = "treeWln";   addDiMuSel = false;  addMuSel=true; addPhoSel=false; iVar = "met";}
  else if  (cindex >= lN+lNc+lNslc && cindex < lN+lNc+lNslc+lNpho ) { TreeName = "tree";   addDiMuSel = false;  addMuSel=false; addPhoSel=true; iVar = "met";}

 
  TFile *gInFilePointer = TFile::Open(name.c_str());
  TTree *thisTree  = (TTree*) gInFilePointer->Get(TreeName.c_str());

  TFile *lFile   = new TFile("Output_tree.root","RECREATE");
  TTree *cutTree = new TTree(fString[cindex].c_str(),fString[cindex].c_str());
  cutTree->SetDirectory(lFile);

//gInFilePointer->cd();
  bool isMC = 1;
  if (cindex==10 || cindex==17 || cindex==26 || cindex==27){
    isMC=0;
  }

  bool isPhWeighted = false;
  if (cindex==28) isPhWeighted=true;

  lFile->cd();
  skim(iVar,thisTree,cutTree,fWeights[cindex],isMC/*isWeighted*/,isPhWeighted/*do the photon eff Weighting*/,addDiMuSel, addMuSel, addPhoSel);  // Skim makes a RooDataSet from the Cut tree
  cutTree->Write();
 
  lFile->Close();
  return 0;
}

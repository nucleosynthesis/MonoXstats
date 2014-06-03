#define RUN_SIGNAL
#define RUN_DIMU
#define RUN_SINGLEMU

float luminosity(19.7);
TH1F *reweightgenpt; 

TTree * load(std::string iName) { 
  TFile *lFile = new TFile(iName.c_str());
  lFile->cd();
  TTree *lTree  = (TTree*) lFile->FindObjectAny("Tree");
  if(lTree == 0) lTree = (TTree*) lFile->FindObjectAny("Flat"); 
  if(lTree == 0) lTree = (TTree*) lFile->FindObjectAny("TestTree"); 
  if(lTree == 0) lTree = (TTree*) lFile->FindObjectAny("TauCheck"); 
  return lTree;
}

TTree *skim(std::string var, TTree *fullTree, std::string newname, std::string cutstr, bool isWeighted , bool reweighted){

  std::cout << newname.c_str() << " " << cutstr.c_str() << std::endl;
  fullTree->Draw(">>cutlist",cutstr.c_str());
  TEventList *keep_points = (TEventList*)gDirectory->Get("cutlist");
  int nEntries = fullTree->GetEntries();

  float x;
  float weight, weight_in, weight_in_pu; 
  float catvar1,catvar2,catvar3,catvar4;
  float tau1,tau2;

  float genpt;

  TLorentzVector *jet;

  TTree *newTree = new TTree(newname.c_str(),newname.c_str());
  newTree->Branch(var.c_str(),&x,Form("%s/Float_t",var.c_str()));
  newTree->Branch("weight",&weight,"weight/Float_t");
  newTree->Branch("jet1mprune",&catvar1,"jet1mprume/Float_t");
  newTree->Branch("jet1mtrim",&catvar2,"jet1mtrim/Float_t");
  newTree->Branch("jet1tau2o1",&catvar3,"jet1tau12o1/Float_t");
  newTree->Branch("jet1pt",&catvar4,"jet1pt/Float_t");
  
  fullTree->SetBranchAddress(var.c_str(),&x);
  fullTree->SetBranchAddress("scale1fb",&weight_in);
  fullTree->SetBranchAddress("puweight",&weight_in_pu);
  fullTree->SetBranchAddress("jet1mprune",&catvar1);
  fullTree->SetBranchAddress("jet1mtrim" ,&catvar2);
  fullTree->SetBranchAddress("jet1tau2" ,&tau2);
  fullTree->SetBranchAddress("jet1tau1" ,&tau1);
  fullTree->SetBranchAddress("jet1"     ,&jet);

  if ( reweighted){
    	std::cout << "Reweighting For generated PT " << std::endl;
  	fullTree->SetBranchAddress("genvpt"     ,&genpt);
  }

  std::cout << " Filling Skim tree " << newTree->GetName() << std::endl;

  for (int evt=0;evt<nEntries;evt++){
    fullTree->GetEntry(evt);
    if ( !(keep_points->Contains(evt)) ) continue;
    if (isWeighted) weight = weight_in*weight_in_pu*luminosity;
    else weight = 1.;
    if (reweighted) { 
    	if (genpt<reweightgenpt->GetXaxis()->GetXmax() && genpt>reweightgenpt->GetXaxis()->GetXmin()) {
	  weight*=reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt));
	}
	//std::cout << "genpt = " << genpt << ", weight = " << reweightgenpt->GetBinContent(reweightgenpt->FindBin(genpt)) <<std::endl;
    }
    catvar3 = tau2/tau1;  // make the sub-structure variable
    catvar4 = jet->Pt();
    //std::cout << jet->Pt() << std::endl; 
    newTree->Fill();
  }

  return newTree;
}

void skimMetFiles(std::string iVar="metRaw") { //genlphi_1-genlphi_2") { 
  const int lN  = 14;
  const int lNc   = 8;
  const int lNslc = 9;

  std::string *fString       = new std::string[lN+lNc+lNslc]; 
  std:;string *fWeights      = new std::string[lN+lNc+lNslc];
  TTree **lTree = new TTree*[lN+lNc+lNslc]; 
  TTree **lCutTree   = new TTree*[lN+lNc+lNslc]; 
  // Full path
  std::string lName   = "/afs/cern.ch/work/p/pharris/public/bacon/prod/CMSSW_5_3_13/src/BaconAnalyzer/MJMITSelection/";

  // reweighting for Generated PT of the Z
  TFile *freweight = TFile::Open("reweightgenptZ.root","OPEN");
  reweightgenpt = (TH1F*)freweight->Get("reweight");

  // Signal Region
  lTree[0]     = load(lName+ "met/DYNN.root");                  fString[0]     = "DY";                     
  lTree[1]     = load(lName+ "met/RDY.root");                   fString[1]     = "RDY";                     
  lTree[2]     = load(lName+ "met/W.root");                     fString[2]     = "W";                     
  lTree[3]     = load(lName+ "met/WHT.root");                   fString[3]     = "WHT";                     
  lTree[4]     = load(lName+ "met/TT.root");                    fString[4]     = "TT";                    
  lTree[5]     = load(lName+ "met/T.root");                     fString[5]     = "T";                     
  lTree[6]     = load(lName+ "met/ZZ.root");                    fString[6]     = "ZZ";                    
  lTree[7]     = load(lName+ "met/WW.root");                    fString[7]     = "WW";                    
  lTree[8]     = load(lName+ "met/WZ.root");                    fString[8]     = "WZ";                    
  lTree[9]     = load(lName+ "met/WH_bacon.root");              fString[9]     = "WH0";                   
  lTree[10]     = load(lName+ "met/ZH_bacon.root");             fString[10]    = "ZH0";                   
  lTree[11]    = load(lName+ "met/GGH_bacon.root");             fString[11]    = "GGH0";                  
  lTree[12]    = load(lName + "met/VBF_bacon.root");            fString[12]    = "VBFH0";                 
  lTree[13]    = load(lName +"met/MET.root");                   fString[13]    = "data_obs";              
  
  // Dimuon control sample 
  lTree[14]     = load(lName+ "dimu/DYDM.root");                fString[14]     = "DY_control_bkg_mc";                     
  lTree[15]     = load(lName+ "dimu/DYLLDM.root");              fString[15]     = "Zvv_control_mc";                     
  lTree[16]     = load(lName+ "dimu/TDM.root");                 fString[16]     = "T_control_bkg_mc";                     
  lTree[17]     = load(lName+ "dimu/TTDM.root");                fString[17]     = "TT_control_bkg_mc";                     
  lTree[18]     = load(lName+ "dimu/WWDM.root");                fString[18]     = "WW_control_bkg_mc";                     
  lTree[19]     = load(lName+ "dimu/WZDM.root");                fString[19]     = "WZ_control_bkg_mc";                     
  lTree[20]     = load(lName+ "dimu/ZZDM.root");                fString[20]     = "ZZ_control_bkg_mc";                     
  lTree[21]     = load(lName+ "dimu/METDM.root");               fString[21]     = "Zvv_control";                    


  // Single lepton control sample 
  lTree[22]     = load(lName+"lep/ZZL.root");              fString[22]    = "ZZ_sl_control_bkg_mc";   
  lTree[23]     = load(lName+"lep/WZL.root");              fString[23]    = "WZ_sl_control_bkg_mc";                  
  lTree[24]     = load(lName+"lep/DYLLL.root");            fString[24]    = "DY_sl_control_bkg_mc";                   
  lTree[25]     = load(lName+"lep/WL.root");               fString[25]    = "Wlv_control_mc_1";                    
  lTree[26]     = load(lName+"lep/WHTL.root");             fString[26]    = "Wlv_control_mc_2";                 
  lTree[27]     = load(lName+"lep/TTL.root");              fString[27]    = "TT_sl_control_bkg_mc";                  
  lTree[28]     = load(lName+"lep/TL.root");               fString[28]    = "T_sl_control_bkg_mc";                   
  lTree[29]     = load(lName+"lep/WWL.root");              fString[29]    = "WW_sl_control_bkg_mc";                  
  lTree[30]  	= load(lName+"lep/METL.root");             fString[30]    = "Wlv_control";                   

  // Check files were opened 
  for(int i0 = 0; i0 < lN+lNc+lNslc-1; i0++) {
    if ( !(lTree[i0]) ) {
      std::cout << "Tree for following sample not loaded, check file! " << fString[i0] << std::endl;
      assert(0);
    }
  }

  // Define the cuts to be applied for the analysis (signal region)
  //for(int i0 = 0; i0 < lN;     i0++) fWeights[i0]  += "(njets < 4 && nbtags > 1 && metFiltersWord == 0 && trigger > 0 && jet1.Pt() > 250 && jet1NHF < 0.9 && jet1NEMF < 0.9 && jet1mtrim > 70 )";
  for(int i0 = 0; i0 < lN+lNc+lNslc;     i0++) fWeights[i0]  += "(trigger > 0 && metRaw > 200 && metFiltersWord == 0  && njets < 3  && jet1.Pt() > 110  && abs(jet1.Eta()) < 2.0 && jet1NHF < 0.7 && jet1NEMF < 0.7  && jet1CHF>0.2 && jdphi<1.8) "; //&& jet1NHF < 0.9 && jet1NEMF < 0.9 && jet1mtrim > 70 )";
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

  TFile *lFile = new TFile("Output_skimtrees.root","RECREATE");
 
  bool reweightpt = false;
  // How can we parallelize this?
#ifdef RUN_SIGNAL
  for(int i0 = 0; i0 < lN-1; i0++) {
    if (i0 == 0) reweightpt=true;
    else  reweightpt=false;
    lCutTree[i0] = skim(iVar,lTree[i0],fString[i0],fWeights[i0],1/*isWeighted*/,reweightpt);  // Skim makes a RooDataSet from the Cut tree
    lCutTree[i0]->Write();
  }
  lCutTree[lN-1]   = skim(iVar,lTree[lN-1],fString[lN-1],fWeights[lN-1],0,0);  // Skim the tree
  lCutTree[lN-1]->Write();
#endif

#ifdef RUN_DIMU
  for(int i0 = lN; i0 < lN+lNc-1; i0++) {
    lCutTree[i0] = skim(iVar,lTree[i0],fString[i0],fWeights[i0],1/*isWeighted*/,0);  // Skim makes a RooDataSet from the Cut tree
    lCutTree[i0]->Write();
  }
  lCutTree[lN+lNc-1]   = skim(iVar,lTree[lN+lNc-1],fString[lN+lNc-1],fWeights[lN+lNc-1],0,0);  // Skim the tree
  lCutTree[lN+lNc-1]->Write();
#endif

#ifdef RUN_SINGLEMU
  for(int i0 = lN+lNc; i0 < lN+lNc+lNslc-1; i0++) {
    lCutTree[i0] = skim(iVar,lTree[i0],fString[i0],fWeights[i0],1/*isWeighted*/,0);  // Skim makes a RooDataSet from the Cut tree
    lCutTree[i0]->Write();
  }
  lCutTree[lN+lNc+lNslc-1]   = skim(iVar,lTree[lN+lNc+lNslc-1],fString[lN+lNc+lNslc-1],fWeights[lN+lNc+lNslc-1],0,0);  // Skim the tree
  lCutTree[lN+lNc+lNslc-1]->Write();
#endif
  
  lFile->Close();
  //lH[1]->Divide(lH[0]);
  return;
}

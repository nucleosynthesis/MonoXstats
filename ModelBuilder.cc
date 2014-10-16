
#include "ModelBuilder.h"
#include "RooCategory.h"
#include "RooSimultaneous.h"

const char * ModelBuilder::doubleexp(RooWorkspace *ws,RooRealVar &x,std::string ext){ 
   // Double exponential model
   RooRealVar frac(Form("f_%s",ext.c_str()),"f",0.8,0.,1.);
   RooRealVar m1(Form("m1_%s",ext.c_str()),"m1",-0.02,-0.3,0.05);
   RooRealVar m2(Form("m2_%s",ext.c_str()),"m2",-0.01,-0.3,0.05);

   RooExponential exp1(Form("exp1_%s",ext.c_str()),Form("exp1_%s",ext.c_str()),x,m1);
   RooExponential exp2(Form("exp2_%s",ext.c_str()),Form("exp2_%s",ext.c_str()),x,m2);

   RooAddPdf *sumexp = new RooAddPdf(Form("doubleExponential_%s",ext.c_str()), Form("doubleExponential_%s",ext.c_str()),RooArgList(exp1,exp2),RooArgList(frac));
   ws->import(*sumexp);
   return sumexp->GetName(); 
}
void ModelBuilder::create_photonjet_weights(TDirectory *fout){

  // Fin should be the place where the FEWZ weights are, fout is where the MET weights end up

  // Get The RECO Level Selected Zvv and Photon MC
  TTree *tree_Zvv_reco = (TTree*) fIn->Get("DY");
  TTree *tree_Pho      = (TTree*) f_PhoSample->Get("GJet");

  // This guy knows about lMET :)

  TH1F * th1f_mvamethist_preweighting = (TH1F*) lMet->Clone("Zvv_madgraph_met");  // no fewz weights
  TH1F * th1f_mvamethist_weighting    = (TH1F*) lMet->Clone("Zvv_fewz_met");
  th1f_mvamethist_preweighting->SetName("Zvv_madgraph_met");
  th1f_mvamethist_weighting->SetName("Zvv_fewz_met");
 
  th1f_mvamethist_preweighting->Sumw2();
  th1f_mvamethist_weighting->Sumw2();

  // Fill the MET histogram (accounting for the fewz weights) 
  tree_Zvv_reco->Draw(Form("%s>>Zvv_madgraph_met",varstring.c_str()),Form("weight*(%s)",cutstring.c_str())); 
  std::cout << "Produced Weighed Zvv generator template" << std::endl;

  // This should ideally be in the config file (OR already selected)
  std::string photon_selection_cuts =  Form("weight*(%s && %s )",cutstring.c_str(), photon_selection.c_str());

  std::cout << "Generating Weights --- Photon Cuts " << photon_selection_cuts.c_str() << std::endl;;
  std::cout << "Generating Weights --- Zvv    Cuts " << cutstring.c_str() << std::endl;;
  // repeat for photons 
  
  TH1F * th1f_hist_photons_mvamet    = (TH1F*)lMet->Clone(); // blah
  th1f_hist_photons_mvamet->SetName("Pho_mg_met");
  th1f_hist_photons_mvamet->Sumw2();
  tree_Pho->Draw(Form("%s>>Pho_mg_met",varstring.c_str()),photon_selection_cuts.c_str());  // most cuts already appliued  (in fact also these ones are applied?!

  // Finally make weights photon->Zvv /******************** WARNING, FOR NOW I USED PRE FEWZ-WEIGTED SAMPLES!!!!!!!! **********/

  TH1F * hist_photon_reweighting_met = (TH1F*)th1f_mvamethist_preweighting->Clone();   // acceptance effect is there but no photon-efficiency correction (assume its high wrt trigger?)
  hist_photon_reweighting_met->SetName("Photon_to_Zvv_weights_met");
  hist_photon_reweighting_met->Divide(th1f_hist_photons_mvamet);
  // Scale by 20.% scale factor
  //hist_photon_reweighting_met->Scale(1./1.2);


  TH1F * hist_photon_reweighting_fewz_met = (TH1F*)th1f_mvamethist_weighting->Clone();   // acceptance effect is there but no photon-efficiency correction (assume its high wrt trigger?)
  hist_photon_reweighting_fewz_met->SetName("Photon_to_Zvv_FEWZ_weights_met");
  hist_photon_reweighting_fewz_met->Divide(th1f_hist_photons_mvamet);

  // turn into spectra for a nice fit ?!
  makeSpectrumHist(th1f_mvamethist_preweighting);
  makeSpectrumHist(th1f_mvamethist_weighting);
  makeSpectrumHist(th1f_hist_photons_mvamet);

  // Save the work
  fout->cd();
  hist_photon_reweighting_met->Write();
  hist_photon_reweighting_fewz_met->Write();
  th1f_hist_photons_mvamet->Write();
  th1f_mvamethist_preweighting->Write();
  th1f_mvamethist_weighting->Write();

}


void ModelBuilder::buildAndFitModels(TDirectory *fout, RooRealVar &x, std::string proc){

   // Build and fit the model for the Zvv/Wlv background
   RooAbsPdf *pdfZvv 		   = wspace->pdf(doubleexp(wspace,x,Form("%s_control",proc.c_str())));
   RooAbsPdf *pdfZvv_mc 	   = wspace->pdf(doubleexp(wspace,x,Form("%s_control_mc",proc.c_str())));
   RooAbsPdf *pdfZvv_background_mc = wspace->pdf(doubleexp(wspace,x,Form("%s_control_bkg_mc",proc.c_str())));

   pdfZvv_mc->Print("v");
   // Fit control region MC
   std::cout << " Fit for control MC " << Form("%s_control_mc",proc.c_str())<< std::endl;
   RooFitResult *fit_res_control_mc  = pdfZvv_mc->fitTo(*(wspace->data(Form("%s_control_mc",proc.c_str()))),RooFit::Save(1),RooFit::SumW2Error(false));
   fout->cd(); fit_res_control_mc->SetName(Form("fitResult_%s_control_mc",proc.c_str())); fit_res_control_mc->Write();

   std::cout << " Fit for background MC " << Form("%s_control_bkg_mc",proc.c_str()) << std::endl;
   // Fit background MC and then fix it
   pdfZvv_background_mc->fitTo(*(wspace->data(Form("%s_control_bkg_mc",proc.c_str()))),RooFit::SumW2Error(true)); // we dont use the errors so who cares about sumW2?
   freezeParameters(pdfZvv_background_mc->getParameters(RooArgSet(x)));
   
   // Now fit the Zvv Data 
   //RooRealVar frac_contamination_Zvv(Form("frac_contamination_%s",proc.c_str()),Form("frac_contamination_%s",proc.c_str()),0,1);
   double nbkgcont = wspace->data(Form("%s_control_bkg_mc",proc.c_str()))->sumEntries();
   double ncont    = wspace->data(Form("%s_control",proc.c_str()))->sumEntries()-nbkgcont;

   RooRealVar num_contamination_Zvv(Form("num_contamination_%s",proc.c_str()),Form("num_contamination_%s",proc.c_str()),nbkgcont,0,10E10);
   num_contamination_Zvv.setConstant();
   RooRealVar num_Zvv(Form("num_%s",proc.c_str()),Form("num_%s",proc.c_str()),ncont,0,10E10);
   num_Zvv.setConstant(true);// freeze the n_data now

   RooAddPdf modelZvv(Form("model_%s_control",proc.c_str()),Form("model_%s_control",proc.c_str()),RooArgList(*pdfZvv_background_mc,*pdfZvv),RooArgList(num_contamination_Zvv,num_Zvv));
   std::cout << " Fit for control Data " << Form("%s_control",proc.c_str()) << std::endl;
   RooFitResult *fit_res_control = modelZvv.fitTo(*(wspace->data(Form("%s_control",proc.c_str()))),RooFit::Save(1));
   fout->cd(); fit_res_control->SetName(Form("fitResult_%s_control",proc.c_str())); fit_res_control->Write();


   // Find the scale of ndata/nmc to normalize the yields
   double nmontecarlo = wspace->data(Form("%s_control_mc",proc.c_str()))->sumEntries();
   double ndata = num_Zvv.getVal();
   RooRealVar scalef(Form("scalef_%s",proc.c_str()),"scalef",ndata/nmontecarlo);
   // uncomment make this ONLY a shape correction!
   // scalef.setVal(1);
   scalef.setConstant(true);

   std::cout << proc.c_str() << " N Control Data == " << ndata       << std::endl;
   std::cout << proc.c_str() << " N Control MC   == " << nmontecarlo << std::endl;

   // Plot the fits
   TCanvas can_datafit(Form("%s_datafit",proc.c_str()),"Data Fit",800,600); 
   RooPlot *pl = x.frame();
   (wspace->data(Form("%s_control_bkg_mc",proc.c_str())))->plotOn(pl,RooFit::MarkerStyle(kOpenCircle));
   (wspace->data(Form("%s_control",proc.c_str())))->plotOn(pl);
   modelZvv.plotOn(pl);
   modelZvv.plotOn(pl,RooFit::Components(pdfZvv_background_mc->GetName()),RooFit::LineStyle(kDashed),RooFit::LineColor(1));
   modelZvv.plotOn(pl,RooFit::VisualizeError(*fit_res_control,2),RooFit::FillColor(kYellow));
   modelZvv.plotOn(pl,RooFit::VisualizeError(*fit_res_control,1),RooFit::FillColor(kGreen) );
   modelZvv.plotOn(pl);
   (wspace->data(Form("%s_control_bkg_mc",proc.c_str())))->plotOn(pl,RooFit::MarkerStyle(kOpenCircle));
   (wspace->data(Form("%s_control",proc.c_str())))->plotOn(pl);
   modelZvv.paramOn(pl);
   //pdfZvv_background_mc->plotOn(pl,RooFit::LineStyle(2));
   pl->SetMinimum(0.001);
   pl->Draw();
   fout->cd();can_datafit.Write();

   TCanvas can_mcfit(Form("%s_mcfit",proc.c_str()),"MC Fit",800,600); 
   RooPlot *plmc = x.frame();
   (wspace->data(Form("%s_control_mc",proc.c_str())))->plotOn(plmc,RooFit::MarkerColor(kBlack));
   pdfZvv_mc->plotOn(plmc,RooFit::LineStyle(1),RooFit::LineColor(2));
   pdfZvv_mc->paramOn(plmc);
   plmc->SetMinimum(0.001);
   plmc->Draw();
   fout->cd();can_mcfit.Write();

   TCanvas can_mcdatafit(Form("%s_mcdatafit",proc.c_str()),"MC and Data Fits",800,600); 
   RooPlot *plmcdata = x.frame();
   pdfZvv_mc->plotOn(plmcdata,RooFit::LineColor(2),RooFit::Normalization(nmontecarlo));
   pdfZvv->plotOn(plmcdata,RooFit::Normalization(ndata));
   plmcdata->SetMinimum(0.001);
   plmcdata->Draw();
   fout->cd();can_mcdatafit.Write();

   // Import the correction and the models 
   RooFormulaVar ratio(Form("ratio_%s",proc.c_str()),"@0*@1/@2",RooArgList(scalef,*pdfZvv,*pdfZvv_mc));
   wspace->import(ratio);
   wspace->import(num_Zvv);

   TCanvas can_ra(Form("%s_ratio",proc.c_str()),"MC Fit",800,600); 
   RooPlot *plra = x.frame();
   ratio.plotOn(plra,RooFit::LineStyle(1));
   plra->Draw();
   fout->cd();can_ra.Write();
}

void ModelBuilder::makeAndImportDataSets(TFile *fin, RooRealVar &x){

   // Get TTrees from input files and fill RooDataSets!
   //const char *name = x.GetName();
   
   RooRealVar weight("weight","weight",0,100000);
   weight.removeRange();

   RooArgSet treevariables(x,weight);
   // List all branches in the tree and add them as variables 
   TObjArray *branches = (TObjArray*) ((TTree*)fin->Get("data_obs"))->GetListOfBranches();
   TIter next(branches);
   TBranch *br;
   while ( (br = (TBranch*)next()) ){
	const char *brname = br->GetName();
	std::cout << brname << std::endl;
	if ( std::strcmp(brname,weight.GetName())!=0 && std::strcmp(brname,x.GetName())!=0 ){
	//  std::cout << " Add variable " << brname << std::endl;
	  RooRealVar *vartmp = new RooRealVar(brname,brname,0,1); vartmp->removeRange();
	  treevariables.add(*vartmp);
	}
   }

    
   std::cout << "Building RooDataSets " << std::endl; 
   treevariables.Print("V");
   //assert(0);
   // Zvv Signal region MC sample 
   RooDataSet zvv("DY","DY",treevariables,RooFit::Import(*((TTree*)fin->Get("DY"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));

   // Zvv Control sample 
   // Data ... (double mu)  Zvv_control
   RooDataSet zvvcontrol("Zvv_control","Zvv_control",treevariables,RooFit::Import(*((TTree*)fin->Get("Zvv_control"))),RooFit::Cut(cutstring.c_str()));

   // Backgrounds ...	    Zvv_control_bkg_mc
   RooDataSet zvvcontrolbkgmc("Zvv_control_bkg_mc","Zvv_control_bkg_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("T_control_bkg_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet zvvcontrolbkgmc_1("1","1",treevariables,RooFit::Import(*(TTree*)fin->Get("TT_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet zvvcontrolbkgmc_2("2","2",treevariables,RooFit::Import(*(TTree*)fin->Get("WW_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet zvvcontrolbkgmc_3("3","3",treevariables,RooFit::Import(*(TTree*)fin->Get("WZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet zvvcontrolbkgmc_4("4","4",treevariables,RooFit::Import(*(TTree*)fin->Get("ZZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_1);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_2);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_3);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_4);

   // MC ...		    Zvv_control_mc
   RooDataSet zvvcontrolmc("Zvv_control_mc","Zvv_control_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("Zvv_control_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   std::cout << " Weights of datasets "  << std::endl;
   std::cout << " Zvv_control " << zvvcontrol.sumEntries() << std::endl;
   std::cout << " Zvv_control_mc " << zvvcontrolmc.sumEntries() << std::endl;

   // Wlv Control sample (single mu)
   // Data ... (double mu)
   // Backgrounds ...
   //.append()
   // MC ...

   // Store these to the output workspace  
   wspace->import(zvvcontrol);
   wspace->import(zvvcontrolmc);
   wspace->import(zvvcontrolbkgmc);
   wspace->import(zvv);


   // Wlv Signal region MC sample 
   RooDataSet wlv("W","W",treevariables,RooFit::Import(*((TTree*)fin->Get("W"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlv_ht("WHT","WHT",treevariables,RooFit::Import(*((TTree*)fin->Get("WHT"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   wlv.append(wlv_ht);

   // Wlv Control sample 
   // Data ... (single mu)  Wlv_control
   RooDataSet wlvcontrol("Wlv_control","Wlv_control",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control"))),RooFit::Cut(cutstring.c_str()));

   // Backgrounds ...	    Wlv_control_bkg_mc
   RooDataSet wlvcontrolbkgmc("Wlv_control_bkg_mc","Wlv_control_bkg_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("T_sl_control_bkg_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolbkgmc_1("1","1",treevariables,RooFit::Import(*(TTree*)fin->Get("TT_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolbkgmc_2("2","2",treevariables,RooFit::Import(*(TTree*)fin->Get("DY_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolbkgmc_3("3","3",treevariables,RooFit::Import(*(TTree*)fin->Get("WZ_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolbkgmc_4("4","4",treevariables,RooFit::Import(*(TTree*)fin->Get("ZZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolbkgmc_5("5","5",treevariables,RooFit::Import(*(TTree*)fin->Get("WW_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_1);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_2);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_3);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_4);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_5);

   // MC ...		    Zvv_control_mc
   RooDataSet wlvcontrolmc("Wlv_control_mc","Wlv_control_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control_mc_1"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   RooDataSet wlvcontrolmc_2("Wlv_control_mc_2","Wlv_control_mc_2",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control_mc_2"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar(weight.GetName()));
   wlvcontrolmc.append(wlvcontrolmc_2);
   std::cout << " Weights of datasets "  << std::endl;
   std::cout << " Wlv_control "    << wlvcontrol.sumEntries() << std::endl;
   std::cout << " Wlv_control_mc " << wlvcontrolmc.sumEntries() << std::endl;

   // Wlv Control sample (single mu)
   // Data ... (double mu)
   // Backgrounds ...
   //.append()
   // MC ...

   // Store these to the output workspace  
   wspace->import(wlvcontrol);
   wspace->import(wlvcontrolmc);
   wspace->import(wlvcontrolbkgmc);
   wspace->import(wlv);

}


void ModelBuilder::createWeightedZmmDataset(TDirectory *fout){

   // Again don't make weighted dataset, the model will HAVE to have 
   // the transfer function already inside it.

   TFile *WeightsFile = TFile::Open("weight_pho_to_zvv.root");
   TH1F  *Weights     = (TH1F*) WeightsFile->Get("Zmm_to_Zvv_weights_met");
   Weights->Fit("pol8");
   fout->WriteTObject(Weights);

}

void ModelBuilder::createPhotonDataset(TDirectory *fout, std::string name, bool makeweights){

   // Never Use errors from Weighted datasets unless the error is intended to represent the error from the sample 
   // size equivalent to weighted dataset (i.e error propagation is wrong in ROOFIT for this (still !!!!)

   // 1. Open the file containing the Photon Data
   //TFile *phoFile = TFile::Open("Output_Photon.root");
   TTree *phoTree = (TTree*) f_PhoSample->Get(name.c_str());   // swap this to GJet for MC based model

   std::cout << " Input from  " << f_PhoSample->GetName() << std::endl;
   if (makeweights) create_photonjet_weights(fout);
   // Weighing histogram for the Photon->Zvv weights;
   //
   //TFile *phoWeightsFile = TFile::Open("weight_pho_to_zvv.root");
   TH1F    *phoWeights_hist     = (TH1F*)   fout->Get("Photon_to_Zvv_weights_met");
   //TGraph  *phoWeights     	= (TGraph*) phoWeightsFile->Get("gr_weights_pho_to_zvv");

   // create a dataset by looping through and weighting events. 
   RooRealVar weight("weight","weight",0,100000);
   weight.removeRange();

   RooRealVar *var = wspace->var(varstring.c_str());
   RooDataSet photondata(Form("photon_data_%s",name.c_str()),Form("photon_data_%s",name.c_str()),RooArgSet(*var,weight),weight.GetName());
   
   // Setup the branches 
   float met, pt, y, pho_pt ,pho_y, pho_id, weight_v, mvamet_raw;
   int njets;

   phoTree->SetBranchAddress(var->GetName(),&met);
   //phoTree->SetBranchAddress("jpt",&pt);
   //phoTree->SetBranchAddress("jeta",&y);
   phoTree->SetBranchAddress("phopt",&pho_pt);
   phoTree->SetBranchAddress("phoeta",&pho_y);
   //phoTree->SetBranchAddress("pho_idmva",&pho_id);
   phoTree->SetBranchAddress("weight",&weight_v);
   //phoTree->SetBranchAddress("mvamet_raw",&mvamet_raw);
   //phoTree->SetBranchAddress("njets",&njets);
  
   double ndatacount = 0.;
   double weight_tot = 0.;
   int nevents = phoTree->GetEntries();
   std::cout << " Filling dataset from Tree "<< std::endl;
   //std::cout << "  Weights from "<< phoWeights->GetName() << std::endl;

   std::string cutstring_photon = cutstring+std::string(" && ")+photon_selection;  // make this configurable I should think
   std::cout << cutstring_photon.c_str() << std::endl;

   //assert(0);
   phoTree->Draw(Form(">>cutlist_photons_%s",name.c_str()),cutstring_photon.c_str());
   TEventList *keep_points = (TEventList*)gDirectory->Get(Form("cutlist_photons_%s",name.c_str()));

   // Right at the end lets just make a histogram of the jet pt and eta, for the Photon (the DY will also be there)! 
   TH1F histo_pt(Form("photon_jpt_%s",name.c_str()),Form("photon_jpt_%s",name.c_str()),60,100,500);
   TH1F histo_eta(Form("photon_jeta_%s",name.c_str()),Form("photon_jeta_%s",name.c_str()),25,0,2.5);

   TH1F histo_phopt(Form("photon_phopt_%s",name.c_str()),Form("photon_phopt_%s",name.c_str()),60,100,500);
   TH1F histo_phoeta(Form("photon_phoeta_%s",name.c_str()),Form("photon_phoeta_%s",name.c_str()),25,0,2.5);

   TH1F histo_phoid(Form("photon_phoidmva_%s",name.c_str()),Form("photon_phoidmva_%s",name.c_str()),100,0,1.);

   TH1F histo_metraw(Form("photon_metraw_%s",name.c_str()),Form("photon_metraw_%s",name.c_str()),80,0,400);

   // Weighted closure models
   TH1F *histo_met = (TH1F*)lMet->Clone(); histo_met->SetName(Form("photon_closure_model_%s",name.c_str()));
   TH1F histo_jpt_weighted(Form("photon_closure_jpt_%s",name.c_str()),Form("photon_closure_jpt_%s",name.c_str()),60,100,500);
   TH1F histo_jeta_weighted(Form("photon_closure_jeta_%s",name.c_str()),Form("photon_closure_jeta_%s",name.c_str()),50,-2.5,2.5);
   TH1F histo_njets_weighted(Form("photon_closure_njets_%s",name.c_str()),Form("photon_closure_njets_%s",name.c_str()),6,0,6);

   double wscale=1.;
   for (int ev=0;ev<nevents;ev++){
      phoTree->GetEntry(ev);
      if ( !(keep_points->Contains(ev)) ) continue;

      weight_v*=wscale; /// NLO K-factors for photons?!

      var->setVal(met);
      photondata.add(RooArgSet(*var),weight_v);

      ndatacount+=weight_v;

      double weight_zvv = weight_v;
      weight_zvv*=phoWeights_hist->GetBinContent(phoWeights_hist->FindBin(met));
      weight_tot+=weight_zvv;

      // Only weight the closure test
      histo_pt.Fill(pt,weight_v);
      histo_eta.Fill(TMath::Abs(y),weight_v);
      histo_phopt.Fill(pho_pt,weight_v);
      histo_phoeta.Fill(TMath::Abs(pho_y),weight_v);
      histo_phoid.Fill(pho_id,weight_v);
      histo_metraw.Fill(mvamet_raw,weight_v);

      histo_met->Fill(met,weight_zvv);
      histo_jpt_weighted.Fill(pt,weight_zvv);
      histo_jeta_weighted.Fill(y,weight_zvv);
      histo_njets_weighted.Fill(njets,weight_zvv);
   }

   RooRealVar ndata_c(Form("ndatacount_%s",name.c_str()),"ndatacount",ndatacount);
   ndata_c.setConstant();

   // Close these inputs now
   wspace->import(photondata); 
   wspace->import(ndata_c); 

   fout->cd();
   histo_pt.Write();
   histo_eta.Write();
   histo_phopt.Write();
   histo_phoeta.Write();
   histo_phoid.Write();
   histo_metraw.Write();
   histo_met->Write();
   histo_jpt_weighted.Write();
   histo_jeta_weighted.Write();
   histo_njets_weighted.Write();

   //fout->WriteTObject(phoWeights_hist);
   //fout->WriteTObject(phoWeights);

   //TCanvas *can_weights = new TCanvas("can_photon_weights","",800,600);
   //phoWeights_hist->GetXaxis()->SetTitle(var->GetName());
   //phoWeights_hist->Draw("PE");
   //phoWeights->SetLineColor(2);
   //phoWeights->Draw();
   //fout->WriteTObject(can_weights);

   //phoFile->Close();
   //phoWeightsFile->Close(); 

}

void ModelBuilder::createPhotonModel(TDirectory *fout){

   std::string proc = "photon";
   RooRealVar *var = wspace->var(varstring.c_str());
   //RooDataSet *photondata_unbinned = (RooDataSet*)wspace->data(Form("%s_data",proc.c_str()));
   RooDataSet *photondata = (RooDataSet*)wspace->data(Form("%s_data_PhoData",proc.c_str()));
   // Try a binned weighted dataset!?
   //var->setBins(800);
   //RooDataHist *photondata = new RooDataHist("photon_data_binned","photon_data_binned",RooArgSet(*var),*photondata_unbinned);
   double ndatacount = ((RooRealVar*)wspace->var("ndatacount_PhoData"))->getVal();
   std::cout << "Ready to fit then " << std::endl;
   // Now we can FIT this weighted guy to get the model :) 
   RooAbsPdf *pdfPho_pdf = wspace->pdf(doubleexp(wspace,*var,Form("%s_control",proc.c_str())));
   RooRealVar normalization("photon_data_norm","photon_data_norm",photondata->sumEntries());
   normalization.setConstant();
   RooExtendPdf *pdfPho = new RooExtendPdf("photon_control_Extend","photon_control_Extend",*pdfPho_pdf,normalization);
   RooFitResult *fit_res_photon  = pdfPho->fitTo(*(wspace->data(Form("%s_data_PhoData",proc.c_str()))),RooFit::Save(1),RooFit::SumW2Error(false));
   wspace->import(*pdfPho);

   fout->cd(); fit_res_photon->SetName(Form("fitResult_%s_control",proc.c_str())); fit_res_photon->Write();

  // wspace->import(*pdfPho); 
   std::cout << "Ok that worked " << std::endl;

   // Make the nominal model 
   TH1F *phoWeights = (TH1F*) fout->Get("Photon_to_Zvv_weights_met");
   //TGraph *phoWeights  = (TGraph*)fout->Get("gr_weights_pho_to_zvv");

   TH1F *photon_zvv_model = (TH1F*) fillHistogramFromPdf(lMet,pdfPho,var,phoWeights);//(TH1F*) lMet->Clone();
   photon_zvv_model->SetName("photon_zvv_model");
   photon_zvv_model->Scale(pdfPho->expectedEvents(*var)); // it has the scale
   photon_zvv_model->Write();

   // Systematic variations !
   TCanvas *can_pho_systs = new TCanvas("can_pho_systs","can_pho_systs",800,600);
   TLegend *leg = new TLegend(0.6,0.4,0.89,0.89); leg->SetFillColor(0); leg->SetTextFont(42);
   photon_zvv_model->SetLineColor(1);photon_zvv_model->SetLineWidth(3); photon_zvv_model->Draw();
   
   std::vector<TH1F> v_th1f_Y;
   generateVariations(photon_zvv_model,(RooFitResult*)fout->Get("fitResult_photon_control"),(RooAbsPdf*)wspace->pdf(pdfPho->GetName()),wspace->var(var->GetName()),v_th1f_Y,wspace,phoWeights,fout);

   int colit=2, styleit=1;
   for (std::vector<TH1F>::iterator hit=v_th1f_Y.begin();hit!=v_th1f_Y.end();hit++){
     hit->SetLineColor(colit);
     hit->SetLineWidth(3);
     hit->SetLineStyle(styleit%2+1);
     leg->AddEntry(&(*hit),hit->GetName(),"L");
     //hit->Scale(norm/hit->Integral());
     hit->Draw("samehist"); 
     hit->Write();
     styleit++;
     if (styleit%2==1) colit++;
   }
   leg->Draw();
   can_pho_systs->Write();
   
   //    
   std::cout << "Photons in Data = " << ndatacount << ", scaled to " << photon_zvv_model->Integral() << " gamma factor = " << photon_zvv_model->Integral()/ndatacount; 
   
   // Plot the fits
   TCanvas can_datafit(Form("%s_datafit",proc.c_str()),"Data Fit",800,600); 
   RooPlot *pl = var->frame();
   (wspace->data(Form("%s_data_PhoData",proc.c_str())))->plotOn(pl,RooFit::DataError(RooAbsData::SumW2));
   pdfPho->plotOn(pl,RooFit::VisualizeError(*fit_res_photon,2),RooFit::FillColor(kYellow));
   pdfPho->plotOn(pl,RooFit::VisualizeError(*fit_res_photon,1),RooFit::FillColor(kGreen));
   //pdfPho->plotOn(pl,RooFit::VisualizeError(*fit_res_photon,1,false),RooFit::LineColor(kRed));
   pdfPho->plotOn(pl);
   (wspace->data(Form("%s_data_PhoData",proc.c_str())))->plotOn(pl,RooFit::DataError(RooAbsData::SumW2));
   pdfPho->paramOn(pl);
   pl->SetMinimum(0.001);
   pl->Draw();
   fout->cd();can_datafit.Write();
   // end it 
}


void ModelBuilder::buildModel( TFile *fin, TDirectory *fout){
   // Split into several parts
   //
   //std::cout << "Cut selection"  << cutstring << std::endl;
   gROOT->SetBatch(1);
   // Define outputs
   wspace = new RooWorkspace();
   wspace->SetName(Form("normalization_cat%d",sel_i));
   fout->cd();

   // Input to this is TTrees 
   // Setup the "x" variable and weights 
   RooRealVar mvamet(varstring.c_str(),varstring.c_str(),xmin,xmax);
   //mvamet.removeRange();
   wspace->import(mvamet);
   
   // TH1F Base Style
   std::string lName = "basehist";
   //const int numberofBins = 13;
   //double myBins[numberofBins+1] = {200,250,300,350,400,450,500,550,600,700,800,900,1000,1200};
   //lMet = new TH1F(lName.c_str(),lName.c_str(),numberofBins,myBins);

   // Make Datasets 
   makeAndImportDataSets(fin,mvamet);

   // ==========================================================================================================
   // This needs to be configured!
   const int nProcs = 27; 
   std::string procnames[nProcs];
   procnames[0]  = "DY";
   procnames[1]  = "DYLL";
   procnames[2]  = "W";
   procnames[3]  = "WHT";
   procnames[4]  = "TT";
   procnames[5]  = "T";
   procnames[6]  = "ZZ";
   procnames[7]  = "WW";
   procnames[8]  = "WZ";
   procnames[9]  = "WH0";
   procnames[10]  = "ZH0";
   procnames[11] = "GGH0";
   procnames[12] = "VBFH0";
   //procnames[12] = "DY_control_bkg_mc";
   procnames[13] = "Zvv_control_mc";   
   procnames[14] = "T_control_bkg_mc"; 
   procnames[15] = "TT_control_bkg_mc";
   procnames[16] = "WW_control_bkg_mc";
   procnames[17] = "WZ_control_bkg_mc";
   procnames[18] = "ZZ_control_bkg_mc";

   procnames[19] = "Wlv_control_mc_1";   
   procnames[20] = "Wlv_control_mc_2";   
   procnames[21] = "T_sl_control_bkg_mc"; 
   procnames[22] = "TT_sl_control_bkg_mc";
   procnames[23] = "WW_sl_control_bkg_mc";
   procnames[24] = "WZ_sl_control_bkg_mc";
   procnames[25] = "ZZ_sl_control_bkg_mc";
   procnames[26] = "DY_sl_control_bkg_mc";


   RooRealVar weight("weight","weight",0,10000); 

   // make jet pt, eta templates 
   TH1F *lPt  = new TH1F("lPt","lPt;;p_{T}^{jet} GeV",60,100,500);
   TH1F *lEta = new TH1F("lEta","lEta;;#eta^{jet}",50,-2.5,2.5);
   // Fill TF1s which do not need corrections
   for (int p0=0;p0<nProcs;p0++){
     if (!fin->Get(procnames[p0].c_str())) continue;
     std::cout << "Filling hist for " << procnames[p0] << std::endl;
     TH1F *hist_      = (TH1F*)generateTemplate(lMet, (TTree*)fin->Get(procnames[p0].c_str()), mvamet.GetName(), weight.GetName(),cutstring);  // standard processes are TTrees
     TH1F *hist_jpt   = (TH1F*)generateTemplate(lPt, (TTree*)fin->Get(procnames[p0].c_str()), "jpt", weight.GetName(),cutstring,"jpt");  // standard processes are TTrees
     //TH1F *hist_jeta  = (TH1F*)generateTemplate(lEta, (TTree*)fin->Get(procnames[p0].c_str()), "jet1eta", weight.GetName(),cutstring,"jeta");  // standard processes are TTrees
     hist_->Write();
     hist_jpt->Write();
   }
   std::cout << "Filling hist for " << "data_obs" << std::endl;;
   TH1F *hist_  = (TH1F*)generateTemplate(lMet, (TTree*)fin->Get("data_obs"), mvamet.GetName(), "",cutstring);  // standard processes are TTrees
   hist_->Write();
   std::cout << "Filling hist for " << "Zvv_control" << std::endl;;
   hist_  = (TH1F*)generateTemplate(lMet, (TTree*)fin->Get("Zvv_control"), mvamet.GetName(), "",cutstring);  // standard processes are TTrees
   hist_->Write();
   std::cout << "Filling hist for " << "Wlv_control" << std::endl;;
   hist_  = (TH1F*)generateTemplate(lMet, (TTree*)fin->Get("Wlv_control"), mvamet.GetName(), "",cutstring);  // standard processes are TTrees
   hist_->Write();
   // ==========================================================================================================

   // Fit backgrounds to produce fit model
 if( run_correction==1 ){
   TDirectory *zmm_model_dir = fout->mkdir("zmm_model");
   buildAndFitModels(zmm_model_dir,mvamet,"Zvv"); 
   TDirectory *wlv_model_dir = fout->mkdir("wlv_model");
   buildAndFitModels(wlv_model_dir,mvamet,"Wlv");

   zmm_model_dir->cd();
   //double mcyield   = wspace->data("DY")->sumEntries();
   double datayield = wspace->var("num_Zvv")->getVal();  // post fit number of data Z->mumu in control
   //std::cout << "sfactor" << brscaleFactorZvv*datayield/mcyield << std::endl; 
   TH1F *hist_zvv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("ratio_Zvv") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("DY")
   //TH1F *hist_zvv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("DY")
   		, 1 /*run correction*/
		, 1 /*brscaleFactorZvv*datayield/mcyield*/ /*additional weight*/);
   hist_zvv->Write();
   std::cout << " DataCardInfo ---------------- " << std::endl;
   std::cout << Form("  Zvv_norm gmN  %d  %g ",(int)datayield,hist_zvv->Integral()/datayield) << std::endl;
   std::cout << " ----------------------------- " << std::endl;

   // Also correct normalization data why not?
   hist_zvv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("ratio_Zvv") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("Zvv_control_mc")
   		, 1 /*run correction*/
		, 1. /*additional weight*/);
   hist_zvv->Write();

   // Single muon
   wlv_model_dir->cd();
   //mcyield   = wspace->data("W")->sumEntries();
   datayield = wspace->var("num_Wlv")->getVal();  // post fit number of data W->munu in control
   //std::cout << "sfactor" << brscaleFactorWlv*datayield/mcyield << std::endl; 
   TH1F *hist_wlv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("ratio_Wlv") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("W")
   		, 1 /*run correction*/
		, 1./*brscaleFactorWlv*datayield/mcyield*/ /*additional weight*/);
   hist_wlv->Write();
   std::cout << " DataCardInfo ---------------- " << std::endl;
   std::cout << Form("  Wlv_norm gmN  %d  %g ",(int)datayield,hist_wlv->Integral()/datayield) << std::endl;
   std::cout << " ----------------------------- " << std::endl;

   // Also correct normalization data why not?
   hist_wlv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("ratio_Wlv") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("Wlv_control_mc")
   		, 1 /*run correction*/
		, 1. /*additional weight*/);
   hist_wlv->Write();
   //buildAndFitModels(fout,wspace,mvamet,"Wlv");   
   //TH1F *hist_wlv = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("ratio_Wlv"), &mvamet, (RooDataSet*) wspace->data(""));
   //hist_wlv->Write();
   // Since the W came in 2 parts, we can make the histogram based on the dataset (called uncorrected)
   TH1F *hist_wlv_uc = (TH1F*)generateTemplate(lMet, (RooFormulaVar*)wspace->function("") , *(wspace->var(mvamet.GetName())), (RooDataSet*) wspace->data("W")
   		, 1 /*run correction forwards*/
		, 1./*brscaleFactorWlv*datayield/mcyield*/ /*additional weight*/);
   hist_wlv_uc->Write();


  if (run_bkgsys==1){
   // Load and run systematics from fit model
   std::vector<TH1F> v_th1f_Z;
   generateVariations((TH1F*)zmm_model_dir->Get("th1f_corrected_DY"),(RooFitResult*)zmm_model_dir->Get("fitResult_Zvv_control"),(RooFormulaVar*)wspace->function("ratio_Zvv"),wspace->var(mvamet.GetName()),v_th1f_Z,wspace,"DY",zmm_model_dir);
   std::vector<TH1F> v_th1f_W;
   generateVariations((TH1F*)wlv_model_dir->Get("th1f_corrected_W"),(RooFitResult*)wlv_model_dir->Get("fitResult_Wlv_control"),(RooFormulaVar*)wspace->function("ratio_Wlv"),wspace->var(mvamet.GetName()),v_th1f_W,wspace,"W",wlv_model_dir);

  }
  if (runphotonmodel){
   TDirectory *photon_model_dir = fout->mkdir("photon_model");
   std::cout << "Creating Photon Model" << std::endl;
   createPhotonDataset(photon_model_dir,"PhoData",1); 
   createPhotonDataset(photon_model_dir,"GJet"); 
   //createPhotonDataset(photon_model_dir,"QCD"); 
   createPhotonModel(photon_model_dir);

   //TDirectory *photon_zmm_model_dir = fout->mkdir("photon_zmm_model");
   //std::cout << "Creating Photon + Zmm Model" << std::endl;
   //createWeightedZmmDataset(photon_zmm_model_dir); 
   //createPhotonAndZvvModel(photon_zmm_model_dir); 
  }
 }
  // Save the work
 fout->cd();
 wspace->Write();
   // Done!
}


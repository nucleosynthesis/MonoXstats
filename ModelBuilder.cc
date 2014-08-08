

//int run_correction(1);
//int run_bkgsys(1);


//double brscaleFactorZvv(5.942); 
//double brscaleFactorWlv(1.017); 
//double luminosity(19.7);

//std::string outfilename = "workspace.root";

//std::string cutvar    = "jet1mprune";
//std::string cutstring =  "" ;//"jet1mprune > 60";

#include "ModelBuilder.h"

const char * ModelBuilder::doubleexp(RooWorkspace *ws,RooRealVar &x,std::string ext){ 
   // Double exponential model
   RooRealVar frac(Form("f_%s",ext.c_str()),"f",0.9,0.,1.);
   RooRealVar m1(Form("m1_%s",ext.c_str()),"m1",-0.02,-0.2,0.);
   RooRealVar m2(Form("m2_%s",ext.c_str()),"m2",-0.01,-0.2,0.);

   RooExponential exp1(Form("exp1_%s",ext.c_str()),Form("exp1_%s",ext.c_str()),x,m1);
   RooExponential exp2(Form("exp2_%s",ext.c_str()),Form("exp2_%s",ext.c_str()),x,m2);

   RooAddPdf *sumexp = new RooAddPdf(Form("doubleExponential_%s",ext.c_str()), Form("doubleExponential_%s",ext.c_str()),RooArgList(exp1,exp2),RooArgList(frac));
   ws->import(*sumexp);
   return sumexp->GetName(); 
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
   pdfZvv_background_mc->fitTo(*(wspace->data(Form("%s_control_bkg_mc",proc.c_str()))),RooFit::SumW2Error(true));
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
   modelZvv.paramOn(pl);
   //pdfZvv_background_mc->plotOn(pl,RooFit::LineStyle(2));
   pl->Draw();
   fout->cd();can_datafit.Write();

   TCanvas can_mcfit(Form("%s_mcfit",proc.c_str()),"MC Fit",800,600); 
   RooPlot *plmc = x.frame();
   (wspace->data(Form("%s_control_mc",proc.c_str())))->plotOn(plmc,RooFit::MarkerColor(kBlack));
   pdfZvv_mc->plotOn(plmc,RooFit::LineStyle(1),RooFit::LineColor(2));
   pdfZvv_mc->paramOn(plmc);
   plmc->Draw();
   fout->cd();can_mcfit.Write();

   TCanvas can_mcdatafit(Form("%s_mcdatafit",proc.c_str()),"MC and Data Fits",800,600); 
   RooPlot *plmcdata = x.frame();
   pdfZvv_mc->plotOn(plmcdata,RooFit::LineColor(2),RooFit::Normalization(nmontecarlo));
   pdfZvv->plotOn(plmcdata,RooFit::Normalization(ndata));
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

/*
void ModelBuilder::buildDataset(TFile *fi, std::string name,std::vector<std::string> sets,std::string cutstring, bool weighted, RooArgSet &treevariables){

   for (std::vector<std::string>::iterator it=sets.begin();it!=sets.end();it++){

   }
}
*/

void ModelBuilder::makeAndImportDataSets(TFile *fin, RooRealVar &x){

   // Get TTrees from input files and fill RooDataSets!
   //const char *name = x.GetName();
   
   RooRealVar weight("weight","weight",0,100);
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

 //  RooRealVar cvar(cutvar.c_str(),cutvar.c_str(),0,100);
 //  cvar.removeRange();
    
   std::cout << "Building RooDataSets " << std::endl; 
   treevariables.Print("V");
   //assert(0);
   // Zvv Signal region MC sample 
   RooDataSet zvv("DY","DY",treevariables,RooFit::Import(*((TTree*)fin->Get("DY"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));

   // Zvv Control sample 
   // Data ... (double mu)  Zvv_control
   RooDataSet zvvcontrol("Zvv_control","Zvv_control",treevariables,RooFit::Import(*((TTree*)fin->Get("Zvv_control"))),RooFit::Cut(cutstring.c_str()));

   // Backgrounds ...	    Zvv_control_bkg_mc
   RooDataSet zvvcontrolbkgmc("Zvv_control_bkg_mc","Zvv_control_bkg_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("T_control_bkg_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet zvvcontrolbkgmc_1("1","1",treevariables,RooFit::Import(*(TTree*)fin->Get("TT_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet zvvcontrolbkgmc_2("2","2",treevariables,RooFit::Import(*(TTree*)fin->Get("WW_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet zvvcontrolbkgmc_3("3","3",treevariables,RooFit::Import(*(TTree*)fin->Get("WZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet zvvcontrolbkgmc_4("4","4",treevariables,RooFit::Import(*(TTree*)fin->Get("ZZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_1);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_2);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_3);
   zvvcontrolbkgmc.append(zvvcontrolbkgmc_4);

   // MC ...		    Zvv_control_mc
   RooDataSet zvvcontrolmc("Zvv_control_mc","Zvv_control_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("Zvv_control_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   std::cout << " Weighs of datasets "  << std::endl;
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
   RooDataSet wlv("W","W",treevariables,RooFit::Import(*((TTree*)fin->Get("W"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlv_ht("WHT","WHT",treevariables,RooFit::Import(*((TTree*)fin->Get("WHT"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   wlv.append(wlv_ht);

   // Wlv Control sample 
   // Data ... (single mu)  Wlv_control
   RooDataSet wlvcontrol("Wlv_control","Wlv_control",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control"))),RooFit::Cut(cutstring.c_str()));

   // Backgrounds ...	    Wlv_control_bkg_mc
   RooDataSet wlvcontrolbkgmc("Wlv_control_bkg_mc","Wlv_control_bkg_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("T_sl_control_bkg_mc"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolbkgmc_1("1","1",treevariables,RooFit::Import(*(TTree*)fin->Get("TT_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolbkgmc_2("2","2",treevariables,RooFit::Import(*(TTree*)fin->Get("DY_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolbkgmc_3("3","3",treevariables,RooFit::Import(*(TTree*)fin->Get("WZ_sl_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolbkgmc_4("4","4",treevariables,RooFit::Import(*(TTree*)fin->Get("ZZ_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolbkgmc_5("5","5",treevariables,RooFit::Import(*(TTree*)fin->Get("WW_control_bkg_mc")),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_1);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_2);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_3);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_4);
   wlvcontrolbkgmc.append(wlvcontrolbkgmc_5);

   // MC ...		    Zvv_control_mc
   RooDataSet wlvcontrolmc("Wlv_control_mc","Wlv_control_mc",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control_mc_1"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   RooDataSet wlvcontrolmc_2("Wlv_control_mc_2","Wlv_control_mc_2",treevariables,RooFit::Import(*((TTree*)fin->Get("Wlv_control_mc_2"))),RooFit::Cut(cutstring.c_str()),RooFit::WeightVar("weight"));
   wlvcontrolmc.append(wlvcontrolmc_2);
   std::cout << " Weighs of datasets "  << std::endl;
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



void ModelBuilder::buildModel( TFile *fin, TDirectory *fout){

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


   
   // Fill TF1s which do not need corrections
   for (int p0=0;p0<nProcs;p0++){
     if (!fin->Get(procnames[p0].c_str())) continue;
     std::cout << "Filling hist for " << procnames[p0] << std::endl;
     TH1F *hist_  = (TH1F*)generateTemplate(lMet, (TTree*)fin->Get(procnames[p0].c_str()), mvamet.GetName(), "weight",cutstring);  // standard processes are TTrees
     hist_->Write();
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
   buildAndFitModels(fout,mvamet,"Zvv");
   buildAndFitModels(fout,mvamet,"Wlv");

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
   generateVariations(lMet,(RooFitResult*)fout->Get("fitResult_Zvv_control"),(RooFormulaVar*)wspace->function("ratio_Zvv"),wspace->var(mvamet.GetName()),v_th1f_Z,wspace,"DY");
   std::vector<TH1F> v_th1f_W;
   generateVariations(lMet,(RooFitResult*)fout->Get("fitResult_Wlv_control"),(RooFormulaVar*)wspace->function("ratio_Wlv"),wspace->var(mvamet.GetName()),v_th1f_W,wspace,"W");

   // Nice plots
   hist_zvv = (TH1F*)fout->Get("th1f_corrected_DY");
   hist_wlv = (TH1F*)fout->Get("th1f_corrected_W");
   
   double norm = hist_zvv->Integral();
   int colit=2, styleit=1;
   TCanvas *can_zvv_systs = new TCanvas("can_zvv_systs","can_zvv_systs",800,600);
   TLegend *leg = new TLegend(0.6,0.4,0.89,0.89); leg->SetFillColor(0); leg->SetTextFont(42);
   hist_zvv->SetLineColor(1);hist_zvv->SetLineWidth(3); hist_zvv->Draw();
   for (std::vector<TH1F>::iterator hit=v_th1f_Z.begin();hit!=v_th1f_Z.end();hit++){
     hit->SetLineColor(colit);
     hit->SetLineWidth(3);
     hit->SetLineStyle(styleit%2+1);
     leg->AddEntry(&(*hit),hit->GetName(),"L");
     hit->Scale(norm/hit->Integral());
     hit->Draw("same"); 
     hit->Write();
     styleit++;
     if (styleit%2==1) colit++;
   }
   leg->Draw();
   can_zvv_systs->Write();

   norm = hist_wlv->Integral();
   styleit=1; colit=2; 
   TCanvas *can_wlv_systs = new TCanvas("can_wlv_systs","can_wlv_systs",800,600);
   TLegend *leg_2 = new TLegend(0.6,0.4,0.89,0.89); leg_2->SetFillColor(0); leg_2->SetTextFont(42);
   hist_wlv->SetLineColor(1);hist_wlv->SetLineWidth(3); hist_wlv->Draw();
   for (std::vector<TH1F>::iterator hit=v_th1f_W.begin();hit!=v_th1f_W.end();hit++){
     hit->SetLineColor(colit);
     hit->SetLineWidth(3);
     hit->SetLineStyle(styleit%2+1);
     leg_2->AddEntry(&(*hit),hit->GetName(),"L");
     hit->Scale(norm/hit->Integral());
     hit->Draw("same"); 
     hit->Write();
     styleit++;
     if (styleit%2==1) colit++;
   }
   leg_2->Draw();
   can_wlv_systs->Write();
  }
 }

   // Save the work
   fout->cd();
   wspace->Write();
   // Done!
}

/*
// None of this should now be done in c nit rather python 
void buildFullModel(){

   gROOT->SetBatch(1);
   TFile *fin = TFile::Open("Output_skimtrees.root");

   // Set up categories and cut-strings 
   std::vector<std::pair<std::string,int> > cut_selections;
   //cut_selections.push_back(" metRaw > 200 ");
   cut_selections.push_back(std::pair<std::string,int>(" !( jet1mprune > 60  && jet1tau2o1 <0.45 && jet1pt>250) ",1));  // cut, do correcteion from control
   cut_selections.push_back(std::pair<std::string,int>("  ( jet1mprune > 60  && jet1tau2o1 <0.45  && jet1pt>250) ",0));

   TFile *fout = new TFile(outfilename.c_str(),"RECREATE");
   
   int nselections = cut_selections.size();
   for (int sel_i=0;sel_i<nselections;sel_i++){
	TDirectory *fdir = fout->mkdir(Form("category_%d",sel_i));
	cutstring = cut_selections[sel_i].first;

	run_correction = cut_selections[sel_i].second;
	if (run_correction !=1 ) run_bkgsys=0;

	buildModel(sel_i,fin,fdir);
	TNamed cstr(Form("cuts_category_%d",sel_i), cutstring.c_str());
	fdir->cd();
	cstr.Write();
   }
}
*/

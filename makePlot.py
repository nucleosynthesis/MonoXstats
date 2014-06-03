import sys
import ROOT as r 
r.gStyle.SetOptStat(0)

fi = r.TFile.Open(sys.argv[1])
di = fi #.Get("mjw_1jet")

def getNormalizedHist(hist):
  nb = hist.GetNbinsX()
  thret = hist.Clone()
  for b in range(1,nb+1): 
    sfactor = 1./hist.GetBinWidth(b)
    thret.SetBinContent(b,hist.GetBinContent(b)*sfactor)
    thret.SetBinError(b,hist.GetBinError(b)*sfactor)
  thret.GetYaxis().SetTitle("Events/GeV")
  return thret

configs = sys.argv[2:]
canvs = []
if len(configs) > 1:
  print "Moving to batch mode, will save pdfs as name of config"
  r.gROOT.SetBatch(1)

for ic,config in enumerate(configs) :
 print "Run Config", config
 x = __import__(config)
 data 	    = di.Get(x.directory+"/"+x.dataname)
 data 	    = getNormalizedHist(data)
 data.SetTitle("")

 can = r.TCanvas("c_%d"%ic,"c_%d"%ic,800,800)
 leg = r.TLegend(0.6,0.6,0.89,0.89); leg.SetFillColor(0); leg.SetTextFont(42)

 # make 2 pads
 pad1 = r.TPad("p2","p2",0,0.28,1,1)
 pad1.SetCanvas(can)
 pad1.Draw()
 pad1.cd()
 data.Draw()

 thstack = r.THStack("bkg","backgroundstack")
 thstack.SetTitle("")
 totalbkg = 0; totalc=0
 print "Nevents ", data.GetName(), data.Integral("width")

 for bkg in x.key_order:
  nullhist = 0; nullc = 0
  for thist in x.backgrounds[bkg][0]:
    tmp = di.Get(x.directory+"/"+thist)
    if nullc == 0 : 	
        print "Starting ", tmp.GetName(), tmp.Integral("")
    	nullhist = tmp.Clone()
    else:
        print "  ... Adding ", tmp.GetName(), tmp.Integral("")
    	nullhist.Add(tmp)
    nullc+=1
   
  nullhist = getNormalizedHist(nullhist)
  print "Nevents ", bkg, nullhist.Integral("width")
  nullhist.SetLineColor(1)
  nullhist.SetLineWidth(2)
  nullhist.SetFillColor(x.backgrounds[bkg][1])
  if totalc==0: totalbkg=nullhist.Clone()
  else : totalbkg.Add(nullhist)
  x.backgrounds[bkg][2]=nullhist.Clone()
  thstack.Add(x.backgrounds[bkg][2])
  leg.AddEntry(x.backgrounds[bkg][2],bkg,"F")
  totalc+=1

 thstack.Draw("histFsame")
 for sig in x.signals.keys():
  tmp = di.Get(x.directory+"/"+sig)
  tmp = getNormalizedHist(tmp)
  tmp.SetLineColor(x.signals[sig][1])
  tmp.SetLineWidth(3)
  x.signals[sig][2]=tmp.Clone()
  x.signals[sig][2].Draw("samehist")
  leg.AddEntry(x.signals[sig][2],x.signals[sig][0],"L")
  print "Nevents ", tmp.GetName(), tmp.Integral("width")

 data.SetMarkerColor(1)
 data.SetLineColor(1)
 data.SetLineWidth(1)
 data.SetMarkerSize(0.8)
 data.SetMarkerStyle(20)
 data.Draw("same")
 leg.Draw()
 pad1.SetLogy()
 pad1.RedrawAxis()

 can.cd()
 pad2 = r.TPad("p2","p2",0,0.03,1,.28)
 pad2.SetCanvas(can)
 pad2.Draw()
 pad2.cd()

 ratio = data.Clone()
 ratio.GetYaxis().SetRangeUser(0.5,1.5)
 ratio.Divide(totalbkg)
 ratio.GetYaxis().SetTitle("Data/Bkg")
 ratio.GetYaxis().SetNdivisions(3)
 ratio.GetYaxis().SetLabelSize(0.1)
 ratio.GetYaxis().SetTitleSize(0.12)
 ratio.GetXaxis().SetTitleSize(0.085)
 ratio.GetXaxis().SetLabelSize(0.1)
 ratio.Draw()
 line = r.TLine(data.GetXaxis().GetXmin(),1,data.GetXaxis().GetXmax(),1)
 line.SetLineColor(2)
 line.SetLineWidth(3)
 line.Draw()
 ratio.Draw("same")
 canvs.append(can)
#can.SaveAs("metdist.pdf")
 can.Draw()
 if len(configs) > 1: can.SaveAs("%s.pdf"%config)

raw_input("Press enter")

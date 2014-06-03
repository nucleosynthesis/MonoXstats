import ROOT
import sys
fi = ROOT.TFile(sys.argv[1])
keys = fi.GetListOfKeys()

def printH(h):
    print " ", h.GetName(), h.Integral()

for k in keys:
 k2 = k.ReadObj()
 if type(k2)==type(ROOT.TProcessID()): continue
 if type(k2) == type(ROOT.TH1F()): printH(k2)
 elif type(k2)==type(ROOT.TDirectory()):
  print "Direcory", k2.GetName()
  for kk in k2.GetListOfKeys():
   h = kk.ReadObj()
   if type(h)==type(ROOT.TH1F()):  printH(h)

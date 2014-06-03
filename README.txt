Statistics/selection/fitting with MonoX Bacon search

1) run skims ....
  root -l skimMetFiles.C

2) fit controls/corrections/systematics/histograms
  root -l buildFullModel.C

3) make plots from the workspace with 
  python makePlot.py config1 config2 ....

4) discover DM

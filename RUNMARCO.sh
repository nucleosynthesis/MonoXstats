#!/bin/bash
EXEC=skimMetFilesExec
OUT=Output_tree.root 
BD=root://eoscms//eos/cms/store/cmst3/user/pharris/Marco/
BD2=root://eoscms//eos/cms/store/group/comm_luminosity/mzanetti/data/

./baconBatch.py $EXEC $OUT -n 1 -a "0:0" -a "1:<${BD}s12-zjets-ptz100-v7a.root>" -o MarcoMonoX/MET/DY 
./baconBatch.py $EXEC $OUT -n 2 -a "0:1" -a "1:<${BD}s12-wjets-ptw100-v7c_0.root,${BD}s12-wjets-ptw100-v7c_1.root>" -o MarcoMonoX/MET/W
./baconBatch.py $EXEC $OUT -n 2 -a "0:2" -a "1:<${BD}s12-wjets-ptw100-v7c_2.root,${BD}s12-wjets-ptw100-v7c_3.root>" -o MarcoMonoX/MET/WHT
./baconBatch.py $EXEC $OUT -n 1 -a "0:3" -a "1:<${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/MET/TT
./baconBatch.py $EXEC $OUT -n 3 -a "0:4" -a "1:<${BD}s12-sch-v7a.root,${BD} s12-schb-v7a.root,${BD}s12-tch-v7a.root,${BD}s12-tchb-v7a.root,${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/MET/T
./baconBatch.py $EXEC $OUT -n 1 -a "0:5" -a "1:<${BD}s12-zz-v7a.root>" -o MarcoMonoX/MET/ZZ 
./baconBatch.py $EXEC $OUT -n 1 -a "0:6" -a "1:<${BD}s12-ww-v7a.root>" -o MarcoMonoX/MET/WW 
./baconBatch.py $EXEC $OUT -n 1 -a "0:7" -a "1:<${BD}s12-wz-v7a.root>" -o MarcoMonoX/MET/WZ 
./baconBatch.py $EXEC $OUT -n 1 -a "0:8" -a "1:<${BD}s12-h125inv-gf-v7a.root>" -o MarcoMonoX/MET/GGH0 
./baconBatch.py $EXEC $OUT -n 1 -a "0:9" -a "1:<${BD}s12-h125inv-vbf-v7a.root>" -o MarcoMonoX/MET/VBF0 
./baconBatch.py $EXEC $OUT -n 2 -a "0:10" -a "1:<${BD}r12a-met-j22-v1.root,${BD}r12b-met-j22-v1.root,${BD}r12c-met-j22-v1.root,${BD}r12d-met-j22-v1.root>" -o MarcoMonoX/MET/data_obs


./baconBatch.py $EXEC $OUT -n 1 -a "0:11" -a "1:<${BD}s12-zll-ptz100-v7c_0.root,${BD}s12-zll-ptz100-v7c_1.root>" -o MarcoMonoX/DiMu/Zvv_control_mc
#./baconBatch.py $EXEC $OUT -n 1 -a "0:11" -a "1:<${BD}s12-zjets-ptz100-v7a.root>" -o MarcoMonoX/DiMu/Zvv_control_mc_V2 
./baconBatch.py $EXEC $OUT -n 3 -a "0:12" -a "1:<${BD}s12-sch-v7a.root,${BD} s12-schb-v7a.root,${BD}s12-tch-v7a.root,${BD}s12-tchb-v7a.root,${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/DiMu/T_control_bkg_mc
./baconBatch.py $EXEC $OUT -n 1 -a "0:13" -a "1:<${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/DiMu/TT_control_bkg_mc
./baconBatch.py $EXEC $OUT -n 1 -a "0:14" -a "1:<${BD}s12-ww-v7a.root>" -o MarcoMonoX/DiMu/WW_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 1 -a "0:15" -a "1:<${BD}s12-wz-v7a.root>" -o MarcoMonoX/DiMu/WZ_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 1 -a "0:16" -a "1:<${BD}s12-zz-v7a.root>" -o MarcoMonoX/DiMu/ZZ_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 2 -a "0:17" -a "1:<${BD}r12a-met-j22-v1.root,${BD}r12b-met-j22-v1.root,${BD}r12c-met-j22-v1.root,${BD}r12d-met-j22-v1.root>" -o MarcoMonoX/DiMu/Zvv_control

./baconBatch.py $EXEC $OUT -n 1 -a "0:18" -a "1:<${BD}s12-zz-v7a.root>" -o MarcoMonoX/SingleMu/ZZ_sl_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 1 -a "0:19" -a "1:<${BD}s12-wz-v7a.root>" -o MarcoMonoX/SingleMu/WZ_sl_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 1 -a "0:20" -a "1:<${BD}s12-zll-ptz100-v7c_0.root,${BD}s12-zll-ptz100-v7c_1.root>" -o MarcoMonoX/SingleMu/DY_sl_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 2 -a "0:21" -a "1:<${BD}s12-wjets-ptw100-v7c_0.root,${BD}s12-wjets-ptw100-v7c_1.root>" -o MarcoMonoX/SingleMu/Wlv_sl_control_mc_1
./baconBatch.py $EXEC $OUT -n 2 -a "0:22" -a "1:<${BD}s12-wjets-ptw100-v7c_2.root,${BD}s12-wjets-ptw100-v7c_3.root>" -o MarcoMonoX/SingleMu/Wlv_sl_control_mc_2
./baconBatch.py $EXEC $OUT -n 1 -a "0:23" -a "1:<${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/SingleMu/TT_sl_control_bkg_mc
./baconBatch.py $EXEC $OUT -n 3 -a "0:24" -a "1:<${BD}s12-sch-v7a.root,${BD} s12-schb-v7a.root,${BD}s12-tch-v7a.root,${BD}s12-tchb-v7a.root,${BD}s12-ttj-v1-v7a.root,${BD}s12-ttj-v2-v7a.root>" -o MarcoMonoX/SingleMu/T_sl_control_bkg_mc
./baconBatch.py $EXEC $OUT -n 1 -a "0:25" -a "1:<${BD}s12-ww-v7a.root>" -o MarcoMonoX/SingleMu/WW_sl_control_bkg_mc 
./baconBatch.py $EXEC $OUT -n 2 -a "0:26" -a "1:<${BD}r12a-met-j22-v1.root,${BD}r12b-met-j22-v1.root,${BD}r12c-met-j22-v1.root,${BD}r12d-met-j22-v1.root>" -o MarcoMonoX/SingleMu/Wlv_control

# Photons 

./baconBatch.py $EXEC $OUT -n 6 -a "0:27" -a "1:${BD2}<r12a-pho-j22-v1_0.root,${BD2}r12b-sph-j22-v1_0.root,${BD2}r12c-sph-j22-v1_0.root,${BD2}r12c-sph-j22-v1_1.root,${BD2}r12d-sph-j22-v1_0.root,${BD2}r12d-sph-j22-v1_1.root,${BD2}r12d-sph-j22-v1_2.root,${BD2}r12d-sph-j22-v1_3.root,${BD2}r12d-sph-j22-v1_4.root,${BD2}r12d-sph-j22-v1_5.root,${BD2}r12d-sph-j22-v1_6.root,${BD2}r12d-sph-j22-v1_7.root>" -o MarcoMonoX/Photon/Data

./baconBatch.py $EXEC $OUT -n 6 -a "0:28" -a "1:<${BD2}s12-pj120_170-v7a_0.root,${BD2}s12-pj1400_1800-v7a_0.root,${BD2}s12-pj170_300-v7a_0.root,${BD2}s12-pj1800-v7a_0.root,${BD2}s12-pj300_470-v7a_0.root,${BD2}s12-pj470_800-v7a_0.root,${BD2}s12-pj50_80-v7a_0.root,${BD2}s12-pj800_1400-v7a_0.root,${BD2}s12-pj80_120-v7a_0.root>" -o MarcoMonoX/Photon/GJet

#NO SUBMISSION

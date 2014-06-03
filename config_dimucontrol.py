import ROOT as r
signals = { }
directory = "category_0"
#key_order = ["Dibosons","top","Z#rightarrow ll","Z#rightarrow#nu#nu"]
key_order = ["Dibosons","top","Z#rightarrow#mu#mu"]

backgrounds = { 
		"top":			  [["th1f_TT_control_bkg_mc","th1f_T_control_bkg_mc"],		r.kBlue-2,0]
		,"Dibosons":		  [["th1f_ZZ_control_bkg_mc","th1f_WW_control_bkg_mc","th1f_WZ_control_bkg_mc"],	r.kPink-4,    0]
		,"Z#rightarrow#mu#mu":	  [["th1f_corrected_Zvv_control_mc"],				r.kBlue-9,  0]
		#,"Z#rightarrow#mu#mu":	  [["th1f_Zvv_control_mc"],				r.kBlue-9,  0]
		#,"Z#rightarrow ll":	  [["th1f_DY_control_bkg_mc"],				r.kBlue-9,  0]
	      }

dataname  = "th1f_Zvv_control"

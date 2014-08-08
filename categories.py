# build categories here 
"""
dummyCat = {
	    	"cutstring":"!(jet1tau2o1 <0.45 && jet1pt>250)"
	    	,"varstring":"mvametu"
	    	,"run_correction":0
	    	,"run_bkgsys":0
	    	,"nbins":80
	    	,"min":200
	    	,"max":1000
	    	,"luminosity":19.7
	
	   }
categories = []
for i in range(5,105,5):
	dc = dummyCat.copy()
	dc['nbins']=i
	categories.append(dc)
"""
outFileName = "output_model_marco.root"
inFileName  = "Output_Marco.root"
categories=[
	    {	# cat 0
	    	"cutstring":"mvamet>200"
	    	,"varstring":"mvamet"
	    	,"run_correction":1
	    	,"run_bkgsys":0
	    	,"nbins":20
	    	,"min":200
	    	,"max":1000
	    	,"luminosity":19.7
	    }
	    ,{	# cat 0
	    	"cutstring":"mvamet>200 && passVBF==0"
	    	,"varstring":"mvamet"
	    	,"run_correction":1
	    	,"run_bkgsys":0
	    	,"nbins":20
	    	,"min":200
	    	,"max":1000
	    	,"luminosity":19.7
	    }
	    ,{	# cat 0
	    	"cutstring":"mvamet>200 && passVBF==1"
	    	,"varstring":"mvamet"
	    	,"run_correction":0
	    	,"run_bkgsys":0
	    	,"nbins":20
	    	,"min":200
	    	,"max":1000
	    	,"luminosity":19.7
	    }
	    	# cat 1
#	   ,{
#	    	"cutstring":"jet1tau2o1 <0.45 && jet1pt>250 && mvametu >200"
#	    	,"varstring":"jet1mprune"
#	    	,"run_correction":0
#	    	,"run_bkgsys":0
#	    	,"nbins":20
#	    	,"min":0
#	    	,"max":200	
#	    	,"luminosity":19.7
#	    }
]



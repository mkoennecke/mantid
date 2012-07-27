# Algorithm to start Bayes programs
from MantidFramework import *
from IndirectImport import run_f2py_compatibility_test, is_supported_f2py_platform

if is_supported_f2py_platform():
	import IndirectBayes as Main

class Quest(PythonAlgorithm):
 
	def category(self):
		return "Workflow\\MIDAS;PythonAlgorithms"

	def PyInit(self):
		self.declareProperty(Name='InputType',DefaultValue='File',Validator=ListValidator(['File','Workspace']),Description = 'Origin of data input - File (*.nxs) or Workspace')
		self.declareProperty(Name='Instrument',DefaultValue='IRIS',Validator=ListValidator(['IRIS','OSIRIS']),Description = 'Instrument')
		self.declareProperty(Name='Analyser',DefaultValue='graphite002',Validator=ListValidator(['graphite002','graphite004']),Description = 'Analyser & reflection')
		self.declareProperty(Name='SamNumber',DefaultValue='',Validator=MandatoryValidator(),Description = 'Sample run number')
		self.declareProperty(Name='ResInputType',DefaultValue='File',Validator=ListValidator(['File','Workspace']),Description = 'Origin of res input - File (*_res.nxs) or Workspace')
		self.declareProperty(Name='ResNumber',DefaultValue='',Validator=MandatoryValidator(),Description = 'Resolution run number')
		self.declareProperty(Name='ElasticOption',DefaultValue=True,Description = 'Include elastic peak in fit')
		self.declareProperty(Name='BackgroundOption',DefaultValue='Sloping',Validator=ListValidator(['Sloping','Flat','Zero']),Description = 'Form of background to fit')
		self.declareProperty(Name='EnergyMin', DefaultValue=-0.5,Description = 'Minimum energy for fit. Default=-0.5')
		self.declareProperty(Name='EnergyMax', DefaultValue=0.5,Description = 'Maximum energy for fit. Default=0.5')
		self.declareProperty(Name='SamBinning', DefaultValue=1,Description = 'Binning value(integer) for sample. Default=1')
		self.declareProperty(Name='NumberSigma', DefaultValue=50,Description = 'Number of sigma values. Default=50')
		self.declareProperty(Name='NumberBeta', DefaultValue=30,Description = 'Number of beta values. Default=30')
		self.declareProperty(Name='Plot',DefaultValue='None',Validator=ListValidator(['None','Sigma','Beta','All']),Description = 'Plot options')
		self.declareProperty(Name='Verbose',DefaultValue=True,Description = 'Switch Verbose Off/On')
		self.declareProperty(Name='Save',DefaultValue=False,Description = 'Switch Save result to nxs file Off/On')
 
	def PyExec(self):
		run_f2py_compatibility_test()
		
		self.log().information('Quest input')
		inType = self.getPropertyValue('InputType')
		instr = self.getPropertyValue('Instrument')
		if instr == 'IRIS':
			prefix = 'irs'
		if instr == 'OSIRIS':
			prefix = 'osi'
		ana = self.getPropertyValue('Analyser')
		sam = self.getPropertyValue('SamNumber')
		rinType = self.getPropertyValue('ResInputType')
		res = self.getPropertyValue('ResNumber')
		elastic = self.getProperty('ElasticOption')
		bgd = self.getPropertyValue('BackgroundOption')
		emin = self.getPropertyValue('EnergyMin')
		emax = self.getPropertyValue('EnergyMax')
		nbin = self.getPropertyValue('SamBinning')
		nbins = [nbin, 1]
		nbet = self.getPropertyValue('NumberBeta')
		nsig = self.getPropertyValue('NumberSigma')
		nbs = [nbet, nsig]

		sname = prefix+sam+'_'+ana
		rname = prefix+res+'_'+ana
		erange = [float(emin), float(emax)]
		if elastic:
			o_el = 1
		else:
			o_el = 0
		if bgd == 'Sloping':
			o_bgd = 2
		if bgd == 'Flat':
			o_bgd = 1
		if bgd == 'Zero':
			o_bgd = 0
		fitOp = [o_el, o_bgd, 0, 0]
		verbOp = self.getProperty('Verbose')
		plotOp = self.getPropertyValue('Plot')
		saveOp = self.getProperty('Save')
		Main.QuestStart(inType,sname,rinType,rname,nbs,erange,nbins,fitOp,verbOp,plotOp,saveOp)

mantid.registerPyAlgorithm(Quest())         # Register algorithm with Mantid

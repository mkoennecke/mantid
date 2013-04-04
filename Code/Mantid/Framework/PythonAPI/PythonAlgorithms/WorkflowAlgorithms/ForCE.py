# Algorithm to start Force
from MantidFramework import *
from IndirectForce import IbackStart, InxStart

class ForCE(PythonAlgorithm):
 
	def category(self):
		return "Workflow\\MIDAS;PythonAlgorithms"

	def PyInit(self):
		self.declareProperty('Mode','ASCII',ListValidator(['ASCII','INX']),Description = 'Ascii format type')
		self.declareProperty('Instrument','IN10',ListValidator(['IN10','IN16']),Description = 'Instrument name')
		self.declareProperty('Analyser','silicon',ListValidator(['silicon']),Description = 'Analyser crystal')
		self.declareProperty('Reflection','111',ListValidator(['111']),Description = 'Analyuser reflection')
		self.declareProperty('RunName', DefaultValue='', Validator = MandatoryValidator(),Description = 'Run name (after <Instr>_)')
		self.declareProperty(Name='RejectZero',DefaultValue=False,Description = 'Reject spectra with zero total count')
		self.declareProperty(Name='UseMap',DefaultValue=False,Description = 'Use detector map')
		self.declareProperty(Name='Verbose',DefaultValue=True,Description = 'Switch Verbose Off/On')
		self.declareProperty(Name='Save',DefaultValue=False,Description = 'Switch Save result to nxs file Off/On')
		self.declareProperty(Name='Plot',DefaultValue='None',Validator=ListValidator(['None','Spectrum','Contour','Both']),Description = 'Plot options')
 
	def PyExec(self):

		self.log().information('ForCE input')
		mode = self.getPropertyValue('Mode')
		instr = self.getPropertyValue('Instrument')
		ana = self.getPropertyValue('Analyser')
		refl = self.getPropertyValue('Reflection')
		run = self.getPropertyValue('RunName')
		rejectZ = self.getProperty('RejectZero')
		useM = self.getProperty('UseMap')
		verbOp = self.getProperty('Verbose')
		saveOp = self.getProperty('Save')
		plotOp = self.getPropertyValue('Plot')

		if mode == 'ASCII':
			IbackStart(instr,run,ana,refl,rejectZ,useM,verbOp,plotOp,saveOp)
		if mode == 'INX':
			InxStart(instr,run,ana,refl,rejectZ,useM,verbOp,plotOp,saveOp)
 
mantid.registerPyAlgorithm(ForCE())                    # Register algorithm with Mantid

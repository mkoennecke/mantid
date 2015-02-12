#--------------------------------------------------------------
# Algorithm which loads a SINQ file. It matches the instrument
# and the right dictionary file and then goes away and calls
# LoadFlexiNexus and others to load the file.
#
# Mark Koennecke, November 2012
#--------------------------------------------------------------
from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty, FrameworkManager
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl
import mantid.simpleapi
from mantid import config
import os.path
import numpy as np

dictsearch= ''

class LoadSINQFile(PythonAlgorithm):
    def category(self):
        return "DataHandling;PythonAlgorithms"

    def summary(self):
        return "Load a SINQ file with the right dictionary."

    def PyInit(self):
        global dictsearch
        self.setWikiSummary("Load a SINQ file with the right dictionary.")
        instruments=["AMOR","AMORS1","AMORS2","BOA","DMC","FOCUS","HRPT","MARSI","MARSE","POLDI",
                     "RITA-2","SANS","SANS2","TRICS"]
        self.declareProperty("Instrument","AMOR",
                             StringListValidator(instruments),
                             "Choose Instrument",direction=Direction.Input)
        self.declareProperty(FileProperty(name="Filename",defaultValue="",
                                          action=FileAction.Load, extensions=[".h5",".hdf"]))
        self.declareProperty("OutputWorkspace","nexus")        
        dictsearch = os.path.join(config['instrumentDefinition.directory'],"nexusdictionaries")


    def PyExec(self):
        inst=self.getProperty('Instrument').value

        if inst =="AMOR":
            self.doAmor()
        if inst =="AMORS1":
            self.doAmorS1()
        if inst =="AMORS2":
            self.doAmorS2()
        elif inst == "BOA":
            self.doBoa()
        elif inst == "DMC":
            self.doDMC()
        elif inst == "FOCUS":
            self.doFocus()
        elif inst == "HRPT":
            self.doHRPT()
        elif inst == "MARSI":
            self.doMarsInelastic()
        elif inst == "MARSE":
            self.doMarsElastic()
        elif inst == "POLDI":
            self.doPoldi()
        elif inst == "RITA-2":
            self.doRITA()
        elif inst == "SANS":
            self.doSANS()
        elif inst == "SANS2":
            self.doSANS()
        elif inst == "TRICS":
            self.doTRICS()
        else: 
            pass
        wname = self.getProperty('OutputWorkspace').value
        mantid.simpleapi.RenameWorkspace('ws',OutputWorkspace =wname)
        
    def doAmor(self):
        dicname = dictsearch +"/mantidamor.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doAmorS1(self):
        dicname = dictsearch +"/mantidamors1.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doAmorS2(self):
        dicname = dictsearch +"/mantidamors2.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doBoa(self):
        dicname = dictsearch +"/mantidboa.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doDMC(self):
        dicname = dictsearch +"/mantiddmc.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doHRPT(self):
        dicname = dictsearch +"/mantidhrpt.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doFocus(self):
        dicname = dictsearch +"/mantidfocus.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doMarsInelastic(self):
        dicname = dictsearch +"/mantidmarsin.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doMarsElastic(self):
        dicname = dictsearch +"/mantidmarse.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doPoldi(self):
        dicname = dictsearch +"/mantidpoldi.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
        config.appendDataSearchDir(config['groupingFiles.directory'])
        grp_file = "POLDI_Grouping_800to400.xml"
        mantid.simpleapi.GroupDetectors(InputWorkspace=ws, 
                           OutputWorkspace=ws,
                           MapFile=grp_file, Behaviour="Sum")        
    def doSANS(self):
        dicname = dictsearch +"/mantidsans.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
    def doTRICS(self):
        dicname = dictsearch +"/mantidtrics.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,'tmp')
#        exec(wname + '= mantid.simpleapi.SINQTranspose3D(\'tmp\',\'TRICS\')')
        mantid.simpleapi.DeleteWorkspace('tmp')
    def doRITA(self):
        dicname = dictsearch +"/mantidrita.dic"
        fname =self.getProperty('Filename').value 
        wname =self.getProperty('OutputWorkspace').value
        ws = mantid.simpleapi.LoadFlexiNexus(fname,dicname,wname)
         

#---------- register with Mantid
AlgorithmFactory.subscribe(LoadSINQFile)

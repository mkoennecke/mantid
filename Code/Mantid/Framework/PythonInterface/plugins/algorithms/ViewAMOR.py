#--------------------------------------------------------------------
# Algorithm which loads a AMOR file and creates the 3 plots:
#
# - TOF summed
# - X-summed
# - Spectrum versus TOF
#
# Mark Koennecke, July 2013
#---------------------------------------------------------------------

from mantid.api import PythonAlgorithm, registerAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl
import mantid.simpleapi
from mantid.simpleapi import mtd
import datetime
import math
import os.path

class ViewAMOR(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;SINQ\\AMOR;Reflectometry\\SINQ'

    def PyInit(self):
        now = datetime.datetime.now()
        self.declareProperty("Year",now.year,"Choose year",direction=Direction.Input)
        self.declareProperty('Numor',0,'Choose file number',direction=Direction.Input)

    def PyExec(self):
        year=self.getProperty('Year').value
        num=self.getProperty('Numor').value
        self.log().error('Running ViewAMOR for file number ' + str(num))
        rawfile = 'tmp' + str(num)
        mantid.simpleapi.LoadSINQ('AMOR',year,num,rawfile)
        raw = mtd[rawfile]
        ntimebin = raw.getDimension(0).getNBins()
        psdsum = 'psdsum' + str(num)
        exec(psdsum + ' = mantid.simpleapi.ProjectMD(rawfile,\'X\',0,ntimebin)')
        ysum = 'ysum' + str(num)
        nx = raw.getDimension(1).getNBins()
        exec(ysum + ' = mantid.simpleapi.ProjectMD(rawfile,\'Z\',0,nx)')
        ny = raw.getDimension(2).getNBins()
        exec('tmp2' + ' = mantid.simpleapi.InvertMDDim(ysum)')
        exec(ysum + ' = mantid.simpleapi.MDHistoToWorkspace2D(tmp2)')
        hist = 'histogram' + str(num)
        exec('tmp3' + ' = mantid.simpleapi.MDHistoToWorkspace2D(\'tmp2\')')
        exec(hist + ' = mantid.simpleapi.GroupDetectors(\'tmp3\',\'\',\'\',\'0-' + str(ny) +
             '\',\'\',False,\'Sum\',False)')
        inf = raw.getExperimentInfo(0)
        r = inf.run()
        cdprop = r.getProperty('chopper_detector_distance')
        CD = cdprop.value[0]/100.
        self.TOFToLambda(hist,CD)
        self.TOFToLambda(ysum,CD)
        mantid.simpleapi.DeleteWorkspace(rawfile)
        mantid.simpleapi.DeleteWorkspace('tmp2')
        mantid.simpleapi.DeleteWorkspace('tmp3')

    def TOFToLambda(self, wsname, CD):
        ws2d = mtd[wsname]
        tofdata = ws2d.dataX(0)
        for i in range(len(tofdata)):
            tofdata[i] = (3.9560346E-7*(tofdata[i]*1.E-7/CD))*1.E11
        nSpectra = ws2d.getNumberHistograms()
        if nSpectra > 1:
            wdata = tofdata.copy()
            for i in range(1,nSpectra):
                ws2d.setX(i,wdata)


        

registerAlgorithm(ViewAMOR())

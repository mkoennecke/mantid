"""*WIKI*
== Description ==

AMOR Spectrum  loads 3D AMOR datasets and sums the x and y directions 
such that the result is a 1D spectrum counts versus TOF.
*WIKI*"""

from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, registerAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl, IntArrayBoundedValidator, IntArrayProperty
import mantid.simpleapi 
from mantid.simpleapi import mtd
import datetime
import numpy as np

class AMORSpectrum(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;Reflectometry\\SINQ;SINQ\\AMOR'

    def name(self):
        return 'AMORSpectrum'

    def version(self):
        return 1

    def PyInit(self):
        now = datetime.datetime.now()
        self.declareProperty("Year",now.year,"Choose year",direction=Direction.Input)
        greaterThanZero = IntArrayBoundedValidator()
        greaterThanZero.setLower(0)
        self.declareProperty(IntArrayProperty("Numors",values=[0], \
                                                  validator=greaterThanZero), doc="Run numbers tp process")
        self.declareProperty("xmin",0,"Lower X summation limit",direction=Direction.Input)
        self.declareProperty("xmax",0,"Upper X summation limit",direction=Direction.Input)
        self.declareProperty("ymin",0,"Lower Y summation limit",direction=Direction.Input)
        self.declareProperty("ymax",0,"Upper Y summation limit",direction=Direction.Input)
        self.declareProperty("Basename",'spec',"Basename of generated files",direction=Direction.Input)
        self.setWikiSummary('Reduce AMOR 3D data to spectrum')


    def PyExec(self):
        run_numbers = self.getProperty('Numors')
        year = self.getProperty('Year').value
        xmin = self.getProperty('xmin').value
        xmax = self.getProperty('xmax').value
        ymin = self.getProperty('ymin').value
        ymax = self.getProperty('ymax').value
        basename = self.getProperty('Basename').value

        for run in run_numbers.value:
            mantid.simpleapi.LoadSINQ('AMOR',year,np.asscalar(run),'tmp')
            tmp = mtd['tmp']
            axis = tmp.getDimension(1)
            mmin = self.findIndex(axis,xmin)
            mmax = self.findIndex(axis,xmax)
            self.log().notice('Projecting from ' + str(mmin) + ' to ' + str(mmax))
            outname = basename + str(run)
            exec(outname + ' = mantid.simpleapi.ProjectMD(tmp,\'Y\',' + str(mmin) + ',' + str(mmax) + ')')
            axis = tmp.getDimension(2)
            mmin = self.findIndex(axis,ymin)
            mmax = self.findIndex(axis,ymax)
            self.log().notice('Projecting from ' + str(mmin) + ' to ' + str(mmax))
            exec(outname + ' = mantid.simpleapi.ProjectMD(' + outname + ',\'Y\',' + str(mmin) + ',' + str(mmax) + ')')
            exec(outname + ' = mantid.simpleapi.MDHistoToWorkspace2D(' + outname + ')')
            mantid.simpleapi.DeleteWorkspace('tmp')


    def findIndex(self,axis,val):
        len = axis.getNBins()
        for i in range(len):
            if axis.getX(i) > val:
                return i
        return i


#---------- register with Mantid
AlgorithmFactory.subscribe(AMORSpectrum)

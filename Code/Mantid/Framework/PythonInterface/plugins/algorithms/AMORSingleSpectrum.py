"""*WIKI*
== Description ==

AMORSingleSpectrum loads  AMOR single detector spectrums 
*WIKI*"""

from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, registerAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl, IntArrayBoundedValidator, IntArrayProperty
import mantid.simpleapi 
import datetime
import numpy as np

class AMORSingleSpectrum(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;Reflectometry\\SINQ;SINQ\\AMOR'

    def name(self):
        return 'AMORSingleSpectrum'

    def version(self):
        return 1

    def PyInit(self):
        now = datetime.datetime.now()
        self.declareProperty("Year",now.year,"Choose year",direction=Direction.Input)
        greaterThanZero = IntArrayBoundedValidator()
        greaterThanZero.setLower(0)
        self.declareProperty(IntArrayProperty("Numors",values=[0], \
                                                  validator=greaterThanZero), doc="Run numbers tp process")
        dets=["Single1","Single2"]
        self.declareProperty("Detector","Single1",
                             StringListValidator(dets),
                             "Choose single detetcor",direction=Direction.Input)

        self.declareProperty("Basename",'spec',"Basename of generated files",direction=Direction.Input)
        self.setWikiSummary('Load AMOR single detector spectra')


    def PyExec(self):
        run_numbers = self.getProperty('Numors')
        year = self.getProperty('Year').value
        det = self.getProperty('Detector').value
        if det == 'Single1':
            inst = 'AMORS1'
        else:
            inst = 'AMORS2'
        basename = self.getProperty('Basename').value

        for run in run_numbers.value:
            outname = basename + str(run)
            mantid.simpleapi.LoadSINQ(inst,year,np.asscalar(run),outname)


#---------- register with Mantid
AlgorithmFactory.subscribe(AMORSingleSpectrum)

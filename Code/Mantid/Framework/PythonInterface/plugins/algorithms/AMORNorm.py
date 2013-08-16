"""*WIKI*
== Description ==

AMORNorm normalises a spectrum by dividing it though another spectrum. 
Appropriate scaling according to monitor counts is applied.

*WIKI*"""

from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, registerAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl, IntArrayBoundedValidator, IntArrayProperty
import mantid.simpleapi 
from mantid.simpleapi import mtd

class AMORNorm(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;Reflectometry\\SINQ;SINQ\\AMOR'

    def name(self):
        return 'AMORNorm'

    def version(self):
        return 1

    def PyInit(self):
        self.declareProperty(WorkspaceProperty("DataWorkspace", "", Direction.Input))
        self.declareProperty(WorkspaceProperty("NormWorkspace", "", Direction.Input))
        self.declareProperty("OutputWorkspace", "", Direction.Input)
        self.setWikiSummary('Normalize AMOR data')


    def PyExec(self):
        wsdata = self.getProperty("DataWorkspace").value
        normdata = self.getProperty("NormWorkspace").value

        try:
            datam1 = wsdata.run().getProperty('Monitor1').value
            normm1 = normdata.run().getProperty('Monitor1').value
        except:
            """
              For generalisation, try to find the monitor elsewhere, like in the instrument 
              data or such
            """
            self.log().error('Failed to find monitor values in Workspace')
            raise Exception('No monitor data')
                            
        scale = datam1[0]/normm1[0]
        
        outname = self.getProperty('OutputWorkspace').value
        exec(outname + '= wsdata/(scale*normdata)')



#---------- register with Mantid
AlgorithmFactory.subscribe(AMORNorm)

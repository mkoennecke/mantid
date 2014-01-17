"""*WIKI*
== Description ==

AMORAxis converts the TOF axis of a AMOR TOF spectrum to lambda or Q. Chopper detector 
distance and om are taken from the data file per default. But can be overriden when 
necessary.

*WIKI*"""

from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, WorkspaceFactory, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, StringArrayProperty
import mantid.simpleapi 
from mantid.simpleapi import mtd
import math

class AMORAxis(PythonAlgorithm):
    def category(self):
        return 'Reflectometry\\SINQ;SINQ\\AMOR'

    def name(self):
        return 'AMORAxis'

    def version(self):
        return 1

    def PyInit(self):
        self.declareProperty(StringArrayProperty("Workspaces", values=[], direction=Direction.Input))
        modes=["Wavelength","Q"]
        self.declareProperty("Axis type","Wavelength",
                             StringListValidator(modes),
                             "Choose axis conversion",direction=Direction.Input)
        self.declareProperty("OutputWorkspacePrefix", "", Direction.Input)
        self.declareProperty("CD", -100,"chopper detector distance in m")
        self.declareProperty("Omega", -100,"Sample rotation, two-theta/2")
        self.declareProperty("ApplyTOFOffset", False,
                             "If true, chopper detector distance and om will be overriden with values below")
        self.declareProperty("TOFOffset", 0.,"TOF offset to apply")
        

    def PyExec(self):
        targetmode = self.getProperty('Axis type').value
        wsnamelist = self.getProperty('Workspaces').value
        prefix = self.getProperty('OutputWorkspacePrefix').value
        override = self.getProperty('ApplyTOFOffset').value
        tofoffset = 0
        if override:
            tofoffset = self.getProperty('TOFOffset').value
        self.log().notice('wsnamelist = ' + str(wsnamelist))

        for name in wsnamelist:
            ws = mtd[name]
            om = self.getProperty('Omega').value
            CD = self.getProperty('CD').value
            if om < -90:
                try:
                    p = ws.run().getProperty('detectors2t').value
                    om = p[0]/2.
                except:
                    raise Exception('File value for omega not available')
            if CD < -90:
                try:
                    p = ws.run().getProperty('chopper_detector_distance').value
                    CD = p[0]/1000.
                except:
                    raise Exception('File value for CD not available')

            self.log().notice('Running AMORAxis with chopper detector distance = ' 
                          + str(CD) + ' and om = ' + str(om) + ' for ' + name)
            wsname = prefix + name
            exec(wsname + '= mantid.simpleapi.CloneWorkspace(ws)')
            out = mtd[wsname]

            axis = out.dataX(0)
            tof = ws.run().getProperty('tof').value
            end = len(axis)
            """
                    Replace TOF with separatly loaded values. This, because loading area 
                    detector data will mess up the TOF array to equidistant steps which just is not 
                    right at AMOR
            """
            for i in range(end):
                axis[i] = tof[i]

            if axis[end-1] - axis[0] < 100:
                raise Exception('Axis does not appear to be TOF')
            if targetmode == 'Wavelength':
                self.TOFToLambda(axis,tofoffset,CD)
            else:
                self.TOFToLambda(axis,tofoffset,CD)
                self.LambdaToQ(axis,om)
                self.invertData(out)
            mantid.simpleapi.ConvertToHistogram(InputWorkspace=out,OutputWorkspace=wsname)


    def TOFToLambda(self, tofdata, tofoffset,CD):
        for i in range(len(tofdata)):
            tofdata[i] = (3.9560346E-7*((tofoffset + tofdata[i])*1.E-7/CD))*1.E10

    def LambdaToQ(self,wdata,om):
        rad2deg = 180./math.pi
        om = om /rad2deg
        for i in range(len(wdata)):
            wdata[i] = (4.* math.pi/wdata[i])*math.sin(om)
        
    def invertData(self,out):
        cts = out.dataY(0)
        q = out.dataX(0)
        y = cts.copy()
        x = q.copy()
        end = len(cts)
        for i in range(end):
            cts[i] = y[end-i-1]
            q[i] = x[end-i-1]

#---------- register with Mantid
AlgorithmFactory.subscribe(AMORAxis)


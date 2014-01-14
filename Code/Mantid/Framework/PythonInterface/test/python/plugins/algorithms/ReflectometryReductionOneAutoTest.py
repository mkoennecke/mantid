import unittest
from mantid.simpleapi import ReflectometryReductionOneAuto, Load, DeleteWorkspace
from ReflectometryReductionOneBaseTest import ReflectometryReductionOneBaseTest

class ReflectometryReductionOneAutoTest(unittest.TestCase, ReflectometryReductionOneBaseTest):
    
    def __init__(self, *args, **kwargs):
        super(ReflectometryReductionOneAutoTest, self).__init__(*args, **kwargs)
    
    def setUp(self):
        ReflectometryReductionOneBaseTest.setUp(self)
        
    def tearDown(self):
        ReflectometryReductionOneBaseTest.setUp(self)
    
    def algorithm_type(self):
        return ReflectometryReductionOneAuto
    
    def test_minimal_inputs(self):

        in_ws = Load('INTER00013460.nxs', OutputWorkspace="13460")
        trans1 = Load('INTER00013463.nxs', OutputWorkspace="trans1")
        
        inst = trans1.getInstrument()
        
        out_ws, out_wsl_lam, thetafinal = ReflectometryReductionOneAuto(InputWorkspace=in_ws, AnalysisMode="PointDetectorAnalysis"
                                                                        ,OutputWorkspace="InQ", OutputWorkspaceWavelength="InLam")
        history = out_ws.getHistory()
        alg = history.lastAlgorithm()
        
        '''
        Here we are checking that the applied values (passed to CreateTransmissionWorkspace come from the instrument parameters.
        '''
        self.assertEqual(inst.getNumberParameter("LambdaMin")[0], alg.getProperty("WavelengthMin").value)
        self.assertEqual(inst.getNumberParameter("LambdaMax")[0], alg.getProperty("WavelengthMax").value)
        self.assertEqual(inst.getNumberParameter("MonitorBackgroundMin")[0], alg.getProperty("MonitorBackgroundWavelengthMin").value)
        self.assertEqual(inst.getNumberParameter("MonitorBackgroundMax")[0], alg.getProperty("MonitorBackgroundWavelengthMax").value)
        self.assertEqual(inst.getNumberParameter("MonitorIntegralMin")[0], alg.getProperty("MonitorIntegrationWavelengthMin").value)
        self.assertEqual(inst.getNumberParameter("MonitorIntegralMax")[0], alg.getProperty("MonitorIntegrationWavelengthMax").value)
        self.assertEqual(inst.getNumberParameter("I0MonitorIndex")[0], alg.getProperty("I0MonitorIndex").value)
        self.assertEqual(inst.getNumberParameter("PointDetectorStart")[0], float(alg.getProperty("ProcessingInstructions").value.split(',')[0]))
        self.assertEqual(inst.getNumberParameter("PointDetectorStop")[0], float(alg.getProperty("ProcessingInstructions").value.split(',')[1]))
    
        DeleteWorkspace(in_ws)
        DeleteWorkspace(trans1)
        
if __name__ == '__main__':
    unittest.main()
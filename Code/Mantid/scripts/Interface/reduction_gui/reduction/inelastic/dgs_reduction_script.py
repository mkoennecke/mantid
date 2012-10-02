"""
    Classes for each reduction step. Those are kept separately 
    from the the interface class so that the DgsReduction class could 
    be used independently of the interface implementation
"""
import xml.dom.minidom
import os
import time
from reduction_gui.reduction.scripter import BaseReductionScripter

class DgsReductionScripter(BaseReductionScripter):
    """
        Organizes the set of reduction parameters that will be used to
        create a reduction script. Parameters are organized by groups that
        will each have their own UI representation.
    """
    TOPLEVEL_WORKFLOWALG = "DgsReduction"
    WIDTH_END = "".join([" " for i in range(len(TOPLEVEL_WORKFLOWALG))])
    WIDTH = WIDTH_END + " "
    
    def __init__(self, name="SEQ", facility="SNS"):
        super(DgsReductionScripter, self).__init__(name=name, facility=facility)
        
    def to_script(self, file_name=None):
        """
            Generate reduction script
            @param file_name: name of the file to write the script to
        """     
        script = "config['default.facility']=\"%s\"\n" % self.facility_name
        script += "\n"
        script +=  "%s(\n" % DgsReductionScripter.TOPLEVEL_WORKFLOWALG
        for item in self._observers:
            if item.state() is not None:
                for subitem in str(item.state()).split('\n'):
                    if len(subitem):
                        script += DgsReductionScripter.WIDTH + subitem + "\n"
        script += DgsReductionScripter.WIDTH_END + ")\n"
        
        if file_name is not None:
            f = open(file_name, 'w')
            f.write(script)
            f.close()
        
        return script
    
    def to_live_script(self):
        script = "from mantidsimple import *\n"
        script += """StartLiveData(UpdateEvery='5',Instrument='FileEventDataListener',StartTime='2012-09-04T15:58:03',ProcessingScript="Rebin(InputWorkspace=input,OutputWorkspace='reb',Params=(5000,500,35000))\\nDgsReduction(SampleInputWorkspace=input,OutputWorkspace=output,IncidentEnergyGuess=25,UseIncidentEnergyGuess=1,EnergyTransferRange=(-20,0.5,20),SofPhiEIsDistribution=0,RejectZeroBackground=0)",PreserveEvents='1',PostProcessingScript="GroupDetectors(InputWorkspace=input,OutputWorkspace=input,MapFile=r'/home/tr9/HYS/HYS_Grouping_2x.xml',Behaviour='Average')\\nTranspose(InputWorkspace=input,OutputWorkspace='Ephi')\\nSofQW3(InputWorkspace=input,OutputWorkspace=input,QAxisBinning='2,0.1,7',EMode='Direct',EFixed='25')\\nTranspose(InputWorkspace=input,OutputWorkspace=output)\\nPlus(LHSWorkspace='reb2',RHSWorkspace='reb',OutputWorkspace='reb2') if mtd.workspaceExists('reb2') else CloneWorkspace(InputWorkspace='reb',OutputWorkspace='reb2')\\nSumSpectra(InputWorkspace='reb2',OutputWorkspace='sum')",EndRunBehavior='Stop',AccumulationWorkspace='acc',OutputWorkspace='ws')\n"""

        script += "sqw_mat = importMatrixWorkspace('ws')\n"
        script += "sqw_g = sqw_mat.plotGraph2D()\n"
        script += "sqw_g.activeLayer().setScale(Layer.Right,0.0001,1)\n"
        script += "sqw_g.activeLayer().logColor()\n"

        script += "ephi_mat = importMatrixWorkspace('Ephi')\n"
        script += "ephi_g = ephi_mat.plotGraph2D()\n"
        script += "ephi_l = ephi_g.activeLayer()\n"
        script += "ephi_l.setScale(Layer.Right,0.0001,1)\n"
        script += "ephi_l.logColor()\n"
        script += """ephi_l.setAxisTitle(Layer.Bottom,"Channel number")\n"""

        script += "ephi_1D = plotBin('Ephi',52,True)\n"

        script += "total_counts = plot('sum',0)\n"
        script += "inst_view = getInstrumentView('reb2')\n"
        script += "inst_view.show()\n"

        return script

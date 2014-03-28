"""*WIKI* 
== ScanAppendToGroup ==

This algorithm appends the input workspace to a workspace group. 
It thereby renames it with a consecutive number. This is meant to 
work together with a LiveDataListener which delivers new data at each 
point of a scan. This algorithm collects them into a group.

In order for the scan Replace or New Group options to work, this alogorithm
expects a workspace title of the form Image-NO: number. If number = 0, 
the requested processing is triggered.

*WIKI*"""

from mantid.api import AlgorithmFactory, WorkspaceGroup
from mantid.api import PythonAlgorithm,  WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl
import mantid.simpleapi
from mantid.simpleapi import mtd

scanNo = 0
scanMemberNo = 0

class ScanAppendToGroup(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;SINQ'

    def PyInit(self):
        self.declareProperty(WorkspaceProperty("Input Workspace", "", Direction.Input),'Workspace to append')
        self.declareProperty('Group Basename','scan',direction=Direction.Input)
        reset_type = ["Replace", "New Group"]
        self.declareProperty("New Scan Action", reset_type[0], 
                             StringListValidator(reset_type), 
                             "What happens for a new scan")
        self.declareProperty(WorkspaceProperty("OutputWorkspace", "", Direction.Output),
                             'Workspace to append')

    def name(self):
        return 'ScanAppendToGroup'

    def PyExec(self):
        global scanMemberNo
        global scanNo
        wsToAppend = self.getProperty('Input Workspace').value
        scanGroupBase = self.getProperty('Group Basename').value
        newScanAction = self.getProperty('New Scan Action').value

        if newScanAction == 'New Group':
            scanGroup = scanGroupBase + '-' + str(scanNo)
        else:
            scanGroup = scanGroupBase

        title = wsToAppend.getTitle()
        if title.find('Image-NO') >= 0:
            comp = title.split(':')
            imNo = int(comp[1])
            if imNo == 0:
                scanMemberNo = 0
                if newScanAction == 'Replace':
                    try:
                        mantid.simpleapi.DeleteWorkspace(scanGroup)
                    except:
                        pass
                else :
                    scanNo = scanNo + 1
                    scanGroup = scanGroupBase + '-' + str(scanNo)

        newname = "%r_data_%04d" % (scanGroup,scanMemberNo)
        scanMemberNo = scanMemberNo +1
        mantid.simpleapi.CloneWorkspace(InputWorkspace=wsToAppend,OutputWorkspace=newname)
                

        try:
            group = mtd[scanGroup]
            group.add(newname)
        except:
            """
               This code sucks a little: The only way to get a WorkspaceGroup from python 
               is through the GroupWorkspaces algorithm. Which wants at least two input 
               workspaces. But I have only one. So I create a dummy, create my group and 
               dump the dummy afterwards. 
            """
            dummy = mantid.simpleapi.CreateWorkspace(DataX=0,DataE=0,DataY=0,
                                                     VerticalAxisUnit='SpectraNumber')
            mantid.simpleapi.GroupWorkspaces(InputWorkspaces=['dummy',newname],
                                             OutputWorkspace=scanGroup)
            g = mtd[scanGroup]
            g.remove('dummy')
            mantid.simpleapi.DeleteWorkspace(Workspace='dummy')


          

AlgorithmFactory.subscribe(ScanAppendToGroup)

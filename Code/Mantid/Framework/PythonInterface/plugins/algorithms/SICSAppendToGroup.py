"""*WIKI* 
== SICSAppendToGroup ==

This algorithm appends the input workspace to a workspace group. 
It thereby renames it with a consecutive number. This is meant to 
work together with a LiveDataListener which delivers new data at each 
point of a scan. This algorithm collects them into a group.

This version builds a connection to a SICServer and learns about new 
scan points and scan starts by registrering an interest on the scan object. 
If your DAQ system is not SICS, then this algorithm is of no use to you.
Except as a code example.

*WIKI*"""

from mantid.api import AlgorithmFactory, WorkspaceGroup
from mantid.api import PythonAlgorithm,  WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl
import mantid.simpleapi
from mantid.simpleapi import mtd
import socket, select

scanNo = 0
scanMemberNo = 0
sics = -1
sicsfile = 0

class SICSAppendToGroup(PythonAlgorithm):
    def category(self):
        return 'PythonAlgorithms;SINQ'

    def PyInit(self):
        self.declareProperty(WorkspaceProperty("Input Workspace", "", Direction.Input),'Workspace to append')
        self.declareProperty('SICS address','boa:2911',direction=Direction.Input)
        self.declareProperty('Group Basename','scan',direction=Direction.Input)
        reset_type = ["Replace", "New Group"]
        self.declareProperty("New Scan Action", reset_type[0], 
                             StringListValidator(reset_type), 
                             "What happens for a new scan")
        self.declareProperty(WorkspaceProperty("OutputWorkspace", "", Direction.Output),
                             'Workspace to append')

    def name(self):
        return 'SICSAppendToGroup'

    def PyExec(self):
        global scanMemberNo
        global scanNo
        global sics
        global sicsfile

        wsToAppend = self.getProperty('Input Workspace').value
        sicsaddress = self.getProperty('SICS address').value
        scanGroupBase = self.getProperty('Group Basename').value
        newScanAction = self.getProperty('New Scan Action').value

        if newScanAction == 'New Group':
            scanGroup = scanGroupBase + '-' + str(scanNo)
        else:
            scanGroup = scanGroupBase

        if sics < 0:
            sics = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
            sa = sicsaddress.split(':')
            sics.connect((sa[0],int(sa[1])))
            sicsfile = sics.makefile('r+',1)
            sicsfile.readline()
            sicsfile.write('Spy 007\r\n')
            sicsfile.readline()
            sicsfile.write('xxxscan interest\r\n')
            sicsfile.readline()

        write = []
        read = [sics]
        inn,out,ex = select.select(read,write,read,.1)
        if sics in inn:
            fromSics = sicsfile.readline()
        else:
            return

        print(fromSics)

        if fromSics.find('NewScan') >= 0:
            scanMemberNo = 0
            if newScanAction == 'Replace':
                try:
                    mantid.simpleapi.DeleteWorkspace(scanGroup)
                except:
                    pass
            else :
                scanNo = scanNo + 1
                scanGroup = scanGroupBase + '-' + str(scanNo)
            return

        if fromSics.find('scan.Counts') >= 0:
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
            

AlgorithmFactory.subscribe(SICSAppendToGroup)

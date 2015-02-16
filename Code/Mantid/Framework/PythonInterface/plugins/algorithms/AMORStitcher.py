"""*WIKI*
== Description ==

This algorith stitches several AMOR file together. Input files 
should have been normalized and converted to Q in advance. 
The algorithm sorts the input workspaces into the right order, 
determines the overlap regions and provides for Jochen Stahns 
Q binning. Well this is a equidistant binning in delta Q. 
*WIKI*"""

from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, WorkspaceFactory, WorkspaceProperty
from mantid.kernel import Direction, StringArrayProperty
import mantid.simpleapi 
from mantid.simpleapi import mtd

def keyWS(ws):
    axis = ws.dataX(0)
    return axis[0]

def namekeyWS(name):
    ws = mtd[name]
    return keyWS(ws)

class AMORStitcher(PythonAlgorithm):
    def category(self):
        return 'Reflectometry\\SINQ;SINQ\\AMOR'

    def name(self):
        return 'AMORStitcher'

    def version(self):
        return 1

    def PyInit(self):
        self.declareProperty(StringArrayProperty("WS to Stitch",values=[],direction=Direction.Input))
        self.declareProperty("OutputWS",'qall',"Name of WS for stitched result data",direction=Direction.Input)
#        self.declareProperty(WorkspaceProperty("OutputWS", "", Direction.Output))

    def summary(self):
        return 'Join AMOR Q Spetra'

    def PyExec(self):
        wsnamelist =  self.getProperty('WS to Stitch').value
        wslist = []
        for name in wsnamelist:
            wslist.append(mtd[name])
        wslist = self.sortWSList(wslist)
        start,end = self.findOverlaps(wslist)
        self.log().debug(str(start))
        self.log().debug(str(end))
        bins = self.makeBins(wslist)
        self.log().debug(str(bins))
        startlist = str(start).strip('[]')
        endlist = str(end).strip('[]')
        binlist = str(bins).strip('[]')
        wsnamelist = sorted(wsnamelist,key=namekeyWS)
        sortedws = ''
        for ws in wsnamelist:
            sortedws += ws + ','
        sortedws = sortedws.strip(',')
        self.log().debug(sortedws)
        self.log().debug(startlist)
        self.log().debug(endlist)
#        self.log().debug(binlist)
        self.log().debug('Len binlist = ' + str(len(bins)))

        ws = mantid.simpleapi.Stitch1DMany(InputWorkspaces=sortedws,StartOverlaps=startlist,\
                                               EndOverlaps=endlist,Params=binlist)
        wname = self.getProperty('OutputWS').value
        mantid.simpleapi.RenameWorkspace('ws',OutputWorkspace =wname)


    def sortWSList(self,wslist):
        return sorted(wslist,key=keyWS)

    def findOverlaps(self,wslist):
        start = []
        end = []
        for i in range(len(wslist)-1):
            axis1 = wslist[i].dataX(0)
            axis2 = wslist[i+1].dataX(0)
            ovstart = axis2[0]
            ovend = axis1[len(axis1)-1]
            ovstart = ovstart + .33*(ovend-ovstart)
            start.append(ovstart)
            end.append(ovend)
        return start,end

    def makeBins(self,wslist):
        bins = []
        ax1 = wslist[0].dataX(0)
        ax2 = wslist[len(wslist)-1].dataX(0)
        start = ax1[0]
        end = ax2[len(ax2)-1]
        xcurr = start
        while xcurr < end:
            bins.append(xcurr)
            xs = xcurr*.01
            xcurr += xs
        # Make number of bins even
        if len(bins) % 2 == 0:
            xs = xcurr*.01
            xcurr += xs
            bins.append(xcurr)
        return bins


#---------- register with Mantid
AlgorithmFactory.subscribe(AMORStitcher)

#--------------------------------------------------------------
# Algorithm which loads a SINQ file.
# This algorithm calculates the filename from instrument
# year and filenumber and then goes away to call
# LoadSINQFile
#
# Mark Koennecke, November 2012
#--------------------------------------------------------------
from mantid.api import AlgorithmFactory
from mantid.api import PythonAlgorithm, WorkspaceFactory, FileProperty, FileAction, WorkspaceProperty
from mantid.kernel import Direction, StringListValidator, ConfigServiceImpl
import mantid.simpleapi
import datetime
import math
import os.path

#------------------------- where files live at SINQ
datapath='/afs/psi.ch/project/sinqdata'

class LoadSINQ(PythonAlgorithm):
    def category(self):
        return "DataHandling;PythonAlgorithms"

    def summary(self):
        return "SINQ data file loader"

    def PyInit(self):
        instruments=["AMOR","AMORS1","AMORS2","BOA","DMC","FOCUS","HRPT","MARSI","MARSE","POLDI",
                     "RITA-2","SANS","SANS2","TRICS"]
        self.declareProperty("Instrument","AMOR",
                             StringListValidator(instruments),
                             "Choose Instrument",direction=Direction.Input)
        now = datetime.datetime.now()
        self.declareProperty("Year",now.year,"Choose year",direction=Direction.Input)
        self.declareProperty('Numor',0,'Choose file number',direction=Direction.Input)
        self.setWikiSummary("SINQ data file loader")
        self.declareProperty(WorkspaceProperty("OutputWorkspace","",direction=Direction.Output))

    def PyExec(self):
        inst=self.getProperty('Instrument').value
        year=self.getProperty('Year').value
        num=self.getProperty('Numor').value
        self.log().error('Running LoadSINQ for ' + inst + " Y= " + str(year) + " N= " +str(num))
        out=self.getProperty('OutputWorkspace').value

        instmap = {}
        instmap['AMOR'] = 'amor'
        instmap['AMORS1'] = 'amor'
        instmap['AMORS2'] = 'amor'
        instmap['BOA'] = 'boa'
        instmap['DMC'] = 'dmc'
        instmap['FOCUS'] = 'focus'
        instmap['HRPT'] = 'hrpt'
        instmap['MARSI'] = 'mars'
        instmap['MARSE'] = 'mars'
        instmap['POLDI'] = 'poldi'
        instmap['RITA-2'] = 'rita2'
        instmap['SANS'] = 'sans'
        instmap['SANS2'] = 'sans2'
        instmap['TRICS'] = 'trics'

        hun=math.floor(num/1000.)
        filename= '%03d/%s%04dn%06d.hdf' % (hun,instmap[inst],year,num)
        fullpath= '%s/%04d/%s/%s' % (datapath,year,instmap[inst],filename)
<<<<<<< HEAD
=======
        wname = "__tmp" #hidden
        ws = None # Holds the workspace later
>>>>>>> ca6beab6e5ed39ba73afb104e05c5614b9860afb
        if os.path.exists(fullpath):
            mantid.simpleapi.LoadSINQFile(fullpath,inst,out)
        else:
            searchDirs = ConfigServiceImpl.Instance().getDataSearchDirs()
            filename= '%s%04dn%06d.hdf' % (instmap[inst],year,num)
            for entry in searchDirs:
                fullpath = '%s/%s' % (entry, filename)
                if os.path.exists(fullpath):
<<<<<<< HEAD
                    mantid.simpleapi.LoadSINQFile(fullpath,inst,out)
                    return
            raise Exception('File %s NOT found!' % filename)
=======
                    ws = mantid.simpleapi.LoadSINQFile(fullpath,inst,OutputWorkspace=wname)
                    break

            # If ws is still "None" at this point, the file was not found.
            if ws is None:
                raise Exception('File %s NOT found!' % filename)
>>>>>>> ca6beab6e5ed39ba73afb104e05c5614b9860afb


#---------- register with Mantid
AlgorithmFactory.subscribe(LoadSINQ)


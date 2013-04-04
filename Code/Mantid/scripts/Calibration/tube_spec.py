from mantid.simpleapi import *
from mantid.kernel import *

class TubeSpec:

# This class is to take a specification of a set of tubes for an instrument provided by a user
# and then provide a list of workspace index ranges corresponding to each of the specified tubes
# to be used the the tube calibration code

# Author: Karl Palmen ISIS

    def __init__(self,ws):
        """     
        Creates empty tube specification for specified instrument.
        
        @param ws: workspace containing the specified instrument with one pixel detector per spectrum.
        """	
	self.ws = ws
        self.inst = ws.getInstrument()
        self.numTubes = 0
        self.tubeSpecString = ""
        self.component = 0
        self.minNumDetsInTube = 200 
        self.tubes = []
        
    def setTubeSpecByString(self, tubeSpecString, DiscontinousIndicesInTube=False):
        """     
        Sets tube specification by string. The string specifies a component of the intrument
        as in the instrument tree of its IDF file. This component may contain one or more tubes
        and possibly all the tunes in the instrument. 
        
        @param tubeSpecString: string specifying tubes of a component of the instrument
        
        The 'tubeSpecString' may be the full path name for the component or 
        steps may be missed out provided it remains unique.
        For example panel03 of WISH can be specified by just 'panel03', 
        because panel03 is unique within the instrument. Also tube012 of this panel, 
        which is unique within the panel but not within the instrument 
        can be specified by 'panel03/tube012' but not 'tube012'.
        If the specification is not unique, the first found will be used and there will
        be no error message. So if in doubt don't skip a step.
        """	
        self.specString = tubeSpecString
        self.delimiter = '/' # delimiter between parts of string in tree
        self.numTubes = -1  # Negative value forces tubes to be searched and counted
        self.continuousIndicesInTube = not DiscontinousIndicesInTube
               
    def getInstrumentName (self):
        return self.inst.getName()
        
    def isTube(self, comp):
        """     
        Determines whether the component is a tube.
         
        @param comp: the component
        
        @return Value: true if component passes test as being a tube
        """
        # We simply assume it's a tube if it has a large number of children
        if( hasattr( comp, "nelements")):
           return (comp.nelements() >= self.minNumDetsInTube ) 
        else:
           return False 
           
    def searchForTubes(self, comp):
         """     
         Searches the component for tubes and saves them in array
         
         @param comp: the component
         """
         # Go through all descendents that are not a descendent of a tube and if it's a tube, store and count it.
         
         if self.isTube( comp ):
             self.tubes.append( comp )
             #print "Tube found", comp.getName()
         # If not tube, Search children, if any
         else:
             if( hasattr( comp, "nelements")):
                 for i in range(comp.nelements()):
                     self.searchForTubes(comp[i])
                     
        
    def getNumTubes(self):
        """     
        Returns number of tubes specified. May also save info about these tubes
         
        @return Value: number of tubes (-1 for erroneous specification)
        """
        if(self.numTubes >= 0):
            return self.numTubes
        
        # We have a negative number set in self.numTubes, so we search for tubes	           
        comp = self.getComponent()
        if( comp == 0): 
            return self.numTubes
        
        self.searchForTubes(comp)
        self.numTubes = len(self.tubes)
        return self.numTubes
            
        	
    def getComponent ( self ):
        """     
        Returns instrument component corresponding to specification
         	
	@Return value: instrument component
        """
        if( self.component != 0):
            return self.component
        
        # We look for the component    
        print "Looking for", self.specString, 
        
        comp = self.inst.getComponentByName(self.specString)

	if( comp ):
	     self.component = comp
	     
	return self.component
 
	
    def getDetectorInfoFromTube( self, tubeIx ):
        """     
        Returns detector info for one tube (currently ID of first detector and number of detectors in tube)
        It assumes that all the pixels along the tube have consecutive detector IDs.
        
        @param tubeIx:  index of Tube in specified set 
	
	@Return value: ID of first detector, number of detectors and step between detectors +1 or -1
        """	
	nTubes = self.getNumTubes()
	if(nTubes < 0):
		print "Error in listing tubes"
		return 0, 0, 1
	if(tubeIx < 0 or tubeIx >= nTubes):
		print "Tube index",tubeIx,"out of range 0 to",nTubes
		return 0, 0, 1
		
	comp = self.tubes[tubeIx]
        
	if(comp != 0):
            firstDet = comp[0].getID()
	    numDet = comp.nelements()
	    # Allow for reverse numbering of Detectors
	    lastDet = comp[numDet-1].getID()
	    if (lastDet < firstDet):
	       step = -1
	       if( firstDet - lastDet + 1 != numDet):
	       	  print "Detector number range",firstDet-lastDet+1," not equal to number of detectors",numDet
	          print "Detectors not numbered continuously in this tube. Calibration will fail for this tube."
	    else:
	       step = 1
	       if( lastDet - firstDet + 1 != numDet):
	       	  print "Detector number range",lastDet-firstDet+1," not equal to number of detectors",numDet
	          print "Detectors not numbered continuously in this tube. Calibration will fail for this tube."
	          
            #print "First dectector ", firstDet," Last detector ", firstDet+numDet-1, "Number of detectors ", numDet
            #print "First dectector ", firstDet," Last detector ", comp[numDet-1].getID()
        else:
            print self.specString, tubeIx, "not found"
            return 0, 0, 1
                        
        return firstDet, numDet, step
        
    def getTubeLength( self, tubeIx ):
        """     
        Returns length of tube.
        
        @param tubeIx:  index of Tube in specified set 
	
	@Return value: Length of tube (first pixel to last pixel) in metres. 0.0 if tube not found.
        """	
	nTubes = self.getNumTubes()
	if(nTubes < 0):
		print "Error in listing tubes"
		return 0.0
	if(tubeIx < 0 or tubeIx >= nTubes):
		print "Tube index",tubeIx,"out of range 0 to",nTubes
		return 0.0
		
	comp = self.tubes[tubeIx]
        
	if(comp != 0):
            firstDet = comp[0].getID()
	    numDet = comp.nelements()
            return comp[0].getDistance( comp[numDet-1] )
        else:
            print self.specString, tubeIx, "not found"
            return 0.0
            
    def getTubeName ( self, tubeIx ):
        """     
        Returns name of tube. 
        This function is not used in tube calibration, but may be useful as a diagnostic.
        It is used in creating the peakfile, which lists the peaks found for each tube.
        
        @param tubeIx:  index of Tube in specified set 
	
	@Return value: Name of tube as in IDF or 'unknown' if not found.
        """	
	nTubes = self.getNumTubes()
	if(nTubes < 0):
		print "Error in listing tubes"
		return 'Unknown'
	if(tubeIx < 0 or tubeIx >= nTubes):
		print "Tube index",tubeIx,"out of range 0 to",nTubes
		return 'Unknown'
		
	comp = self.tubes[tubeIx]
        
	if(comp != 0):
	    return comp.getFullName()
        else:
            print self.specString, tubeIx, "not found"
            return "Unknown"

                        
	
    def getTubeByString(self, tubeIx):
     	"""     
        Returns list of workspace indices of a tube set that has been specified by string
        It assumes that all the pixels along the tube have consecutive detector IDs 
        
        @param tubeIx:  index of Tube in specified set 
	
	Return value: list of indices
        """
	firstDet, numDet, step = self.getDetectorInfoFromTube( tubeIx )			   
        wkIds = []
        # print " First dectector", firstDet," Last detector", firstDet+numDet-1, "Number of detectors", numDet
        # print "Histograms", self.ws.getNumberHistograms()
        
        # First check we have one detector per histogram/workpsaceID/spectrum
        sampleIndex = 10
        sp = self.ws.getSpectrum(sampleIndex)
        detids = sp.getDetectorIDs()
        numDetsPerWkID = len(detids)
        if( numDetsPerWkID != 1):
            print "We have",numDetsPerWkID,"detectors per workspace index. 1 is required."
            print "cannot obtain range of workspace indices for this tube in this workspace"
            return wkIds
        
        # Go and get workspace Indices
        if(self.continuousIndicesInTube):
            if(step == -1):
               startDet = firstDet - numDet + 1
            else:
               startDet = firstDet
            if( numDet > 0):
                 for i in range (0, self.ws.getNumberHistograms(), numDet):
	             deti = self.ws.getDetector(i)
	             detID = deti.getID()
	             if (detID  >= startDet and detID < startDet+numDet):
	                 iPixel = detID - firstDet
	                 wkIds = range( i - iPixel, i - iPixel + step*numDet, step)
	                 # print "Workspace indices",i-iPixel,"to",i-iPixel+numDet-1

        else: #We can't assume continuous indices within tube, must loop over all indices (there are many). 
            if( numDet > 0):
                 for i in range(self.ws.getNumberHistograms()):
	             sp=self.ws.getSpectrum(i)
	             detids = sp.getDetectorIDs()
	             for j in range(len(detids)):
	                if (detids[j]  >= firstDet and detids[j] < firstDet+numDet):
	                    wkIds.append(i)

        #print  firstDet, numDet
        if (numDet > 0):
	    return wkIds
        else:
            print "specified tube has no detectors."
            self.numTubes = 0
	    return wkIds

	
    def getTube(self, tubeIx):
	"""     
        Returns list of workspace indices of a tube
        
        @param tubeIx:  index of Tube in specified set
	
	Return value: list of indices
        """
        nTubes = self.getNumTubes()
        if( (0 <= tubeIx) & (tubeIx < nTubes) ):
	    return self.getTubeByString(tubeIx)
	else:
	    print "Tube", tubeIx, "out of range 0 to",self.numTubes,"."

	    
        
# IDA F2PY Absorption Corrections Wrapper
## Handle selection of .pyd files for absorption corrections
import platform, sys
from IndirectImport import *
if is_supported_f2py_platform():
    cylabs = import_f2py("cylabs")
else:
    unsupported_message()

from IndirectCommon import *
from mantid.simpleapi import *
from mantid import config, logger, mtd
import math, os.path, numpy as np
mp = import_mantidplot()

def WaveRange(inWS, efixed):
# create a list of 10 equi-spaced wavelengths spanning the input data
    oWS = '__WaveRange'
    ExtractSingleSpectrum(InputWorkspace=inWS, OutputWorkspace=oWS, WorkspaceIndex=0)
    ConvertUnits(InputWorkspace=oWS, OutputWorkspace=oWS, Target='Wavelength',
        EMode='Indirect', EFixed=efixed)
    Xin = mtd[oWS].readX(0)
    xmin = mtd[oWS].readX(0)[0]
    xmax = mtd[oWS].readX(0)[len(Xin)-1]
    ebin = 0.5
    nw1 = int(xmin/ebin)
    nw2 = int(xmax/ebin)+1
    w1 = nw1*ebin
    w2 = nw2*ebin
    wave = []
    nw = 10
    ebin = (w2-w1)/(nw-1)
    for l in range(0,nw):
        wave.append(w1+l*ebin)
    DeleteWorkspace(oWS)
    return wave

def CheckSize(size,geom,ncan,Verbose):
    if geom == 'cyl':
        if (size[1] - size[0]) < 1e-4:
            error = 'Sample outer radius not > inner radius'			
            logger.notice('ERROR *** '+error)
            sys.exit(error)
        else:
            if Verbose:
                message = 'Sam : inner radius = '+str(size[0])+' ; outer radius = '+str(size[1])
                logger.notice(message)
    if geom == 'flt':
        if size[0] < 1e-4:
            error = 'Sample thickness is zero'			
            logger.notice('ERROR *** '+error)
            sys.exit(error)
        else:
            if Verbose:
                logger.notice('Sam : thickness = '+str(size[0]))
    if ncan == 2:
        if geom == 'cyl':
            if (size[2] - size[1]) < 1e-4:
                error = 'Can inner radius not > sample outer radius'			
                logger.notice('ERROR *** '+error)
                sys.exit(error)
            else:
                if Verbose:
                    message = 'Can : inner radius = '+str(size[1])+' ; outer radius = '+str(size[2])
                    logger.notice(message)
        if geom == 'flt':
            if size[1] < 1e-4:
                error = 'Can thickness is zero'			
                logger.notice('ERROR *** '+error)
                sys.exit(error)
            else:
                if Verbose:
                    logger.notice('Can : thickness = '+str(size[1]))

def CheckDensity(density,ncan):
    if density[0] < 1e-5:
        error = 'Sample density is zero'			
        logger.notice('ERROR *** '+error)
        sys.exit(error)
    if ncan == 2:
        if density[1] < 1e-5:
            error = 'Can density is zero'			
            logger.notice('ERROR *** '+error)
            sys.exit(error)

def AbsRun(inputWS, geom, beam, ncan, size, density, sigs, siga, avar, Verbose, Save):
    workdir = config['defaultsave.directory']
    if Verbose:
        logger.notice('Sample run : '+inputWS)
    Xin = mtd[inputWS].readX(0)
    if len(Xin) == 0:				# check that there is data
        error = 'Sample file has no data'			
        logger.notice('ERROR *** '+error)
        sys.exit(error)
    det = GetWSangles(inputWS)
    ndet = len(det)
    efixed = getEfixed(inputWS)
    wavelas = math.sqrt(81.787/efixed) # elastic wavelength
    waves = WaveRange(inputWS, efixed) # get wavelengths
    nw = len(waves)
    CheckSize(size,geom,ncan,Verbose)
    CheckDensity(density,ncan)
    run_name = getWSprefix(inputWS)
    if Verbose:
        message = 'Sam : sigt = '+str(sigs[0])+' ; siga = '+str(siga[0])+' ; rho = '+str(density[0])
        logger.notice(message)
        if ncan == 2:
            message = 'Can : sigt = '+str(sigs[1])+' ; siga = '+str(siga[1])+' ; rho = '+str(density[1])
            logger.notice(message)
        logger.notice('Elastic lambda : '+str(wavelas))
        message = 'Lambda : '+str(nw)+' values from '+str(waves[0])+' to '+str(waves[nw-1])
        logger.notice(message)
        message = 'Detector angles : '+str(ndet)+' from '+str(det[0])+' to '+str(det[ndet-1])
        logger.notice(message)
    eZ = np.zeros(nw)                  # set errors to zero
    name = run_name + geom
    assWS = name + '_ass'
    asscWS = name + '_assc'
    acscWS = name + '_acsc'
    accWS = name + '_acc'
    fname = name +'_Abs'
    wrk = os.path.join(workdir, run_name)
    wrk.ljust(120,' ')
    for n in range(0,ndet):
        if geom == 'flt':
            angles = [avar, det[n]]
            (A1,A2,A3,A4) = FlatAbs(ncan, size, density, sigs, siga, angles, waves)	
            kill = 0
        if geom == 'cyl':
            astep = avar
            if (astep) < 1e-5:
                error = 'Step size is zero'			
                logger.notice('ERROR *** '+error)
                sys.exit(error)
            nstep = int((size[1] - size[0])/astep)
            if nstep < 20:
                error = 'Number of steps ( '+str(nstep)+' ) should be >= 20'			
                logger.notice('ERROR *** '+error)
                sys.exit(error)
            angle = det[n]
            kill, A1, A2, A3, A4 = cylabs.cylabs(astep, beam, ncan, size,
                density, sigs, siga, angle, wavelas, waves, n, wrk, 0)
        if kill == 0:
            if Verbose:
                logger.notice('Detector '+str(n)+' at angle : '+str(det[n])+' * successful')
            if n == 0:
                dataA1 = A1
                dataA2 = A2
                dataA3 = A3
                dataA4 = A4
                eZero =eZ
            else:
                dataA1 = np.append(dataA1,A1)
                dataA2 = np.append(dataA2,A2)
                dataA3 = np.append(dataA3,A3)
                dataA4 = np.append(dataA4,A4)
                eZero = np.append(eZero,eZ)
        else:
            error = 'Detector '+str(n)+' at angle : '+str(det[n])+' *** failed : Error code '+str(kill)
            logger.notice('ERROR *** '+error)
            sys.exit(error)
## Create the workspaces
    dataX = waves * ndet
    qAxis = createQaxis(inputWS)
    CreateWorkspace(OutputWorkspace=assWS, DataX=dataX, DataY=dataA1, DataE=eZero,
        NSpec=ndet, UnitX='Wavelength',
        VerticalAxisUnit='MomentumTransfer', VerticalAxisValues=qAxis)
    CreateWorkspace(OutputWorkspace=asscWS, DataX=dataX, DataY=dataA2, DataE=eZero,
        NSpec=ndet, UnitX='Wavelength',
        VerticalAxisUnit='MomentumTransfer', VerticalAxisValues=qAxis)
    CreateWorkspace(OutputWorkspace=acscWS, DataX=dataX, DataY=dataA3, DataE=eZero,
        NSpec=ndet, UnitX='Wavelength',
        VerticalAxisUnit='MomentumTransfer', VerticalAxisValues=qAxis)
    CreateWorkspace(OutputWorkspace=accWS, DataX=dataX, DataY=dataA4, DataE=eZero,
        NSpec=ndet, UnitX='Wavelength',
        VerticalAxisUnit='MomentumTransfer', VerticalAxisValues=qAxis)
    ## Save output
    group = assWS +','+ asscWS +','+ acscWS +','+ accWS
    GroupWorkspaces(InputWorkspaces=group,OutputWorkspace=fname)
    if Save:
        opath = os.path.join(workdir,fname+'.nxs')
        SaveNexusProcessed(InputWorkspace=fname, Filename=opath)
        if Verbose:
            logger.notice('Output file created : '+opath)
    if ncan > 1:
        return [assWS, asscWS, acscWS, accWS]
    else:
        return [assWS]

def AbsRunFeeder(inputWS, geom, beam, ncan, size, density, sigs, siga, avar,
        plotOpt='None', Verbose=False,Save=False):
    StartTime('CalculateCorrections')
    '''Handles the feeding of input and plotting of output for the F2PY
    absorption correction routine.'''
    workspaces = AbsRun(inputWS, geom, beam, ncan, size, density,
        sigs, siga, avar, Verbose, Save)
    EndTime('CalculateCorrections')
    if ( plotOpt == 'None' ):
        return
    if ( plotOpt == 'Wavelength' or plotOpt == 'Both' ):
        graph = mp.plotSpectrum(workspaces, 0)
    if ( plotOpt == 'Angle' or plotOpt == 'Both' ):
        graph = mp.plotTimeBin(workspaces, 0)
        graph.activeLayer().setAxisTitle(mp.Layer.Bottom, 'Angle')


# FlatAbs - calculate flat plate absorption factors
# 
# For more information See:
#   - MODES User Guide: http://www.isis.stfc.ac.uk/instruments/iris/data-analysis/modes-v3-user-guide-6962.pdf  
#   - C J Carlile, Rutherford Laboratory report, RL-74-103 (1974)  
#
#  Input parameters :
#  sigs - list of scattering  cross-sections
#  siga - list of absorption cross-sections
#  density - list of density
#  ncan - = 0 no can, >1 with can
#  thick - list of thicknesses: sample thickness, can thickness1, can thickness2
#  angles - list of angles
#  waves - list of wavelengths

def Fact(xSection,thickness,sec1,sec2):
    S = xSection*thickness*(sec1-sec2)
    F = 1.0
    if (S == 0.):
        F = thickness
    else:
        S = (1-math.exp(-S))/S
        F = thickness*S
    return F

def calcThicknessAtSec(xSection, thickness, sec):
    sec1, sec2 = sec

    thickSec1 = xSection * thickness * sec1
    thickSec2 = xSection * thickness * sec2

    return thickSec1, thickSec2

def calcFlatAbsCan(ass, canXSection, canThickness1, canThickness2, sampleSec1, sampleSec2, sec):
    nlam = len(canXSection)

    assc = np.ones(nlam)
    acsc = np.ones(nlam)
    acc = np.ones(nlam)

    sec1, sec2 = sec

    #vector version of fact
    vecFact = np.vectorize(Fact)
    f1 = vecFact(canXSection,canThickness1,sec1,sec2)
    f2 = vecFact(canXSection,canThickness2,sec1,sec2)

    canThick1Sec1, canThick1Sec2 = calcThicknessAtSec(canXSection, canThickness1, sec)
    canThick2Sec1, canThick2Sec2 = calcThicknessAtSec(canXSection, canThickness2, sec)

    if (sec2 < 0.):
        val = np.exp(-(canThick1Sec1-canThick1Sec2))
        assc = ass * val

        acc1 = f1
        acc2 = f2 * val

        acsc1 = acc1
        acsc2 = acc2 * np.exp(-(sampleSec1-sampleSec2))
    else:
        val = np.exp(-(canThick1Sec1+canThick2Sec2))
        assc = ass * val

        acc1 = f1 * np.exp(-(canThick1Sec2+canThick2Sec2))
        acc2 = f2 * val

        acsc1 = acc1 * np.exp(-sampleSec2)
        acsc2 = acc2 * np.exp(-sampleSec1)

    canThickness = canThickness1+canThickness2

    if(canThickness > 0.):
        acc = (acc1+acc2)/canThickness
        acsc = (acsc1+acsc2)/canThickness

    return assc, acsc, acc

def FlatAbs(ncan, thick, density, sigs, siga, angles, waves):
    
    PICONV = math.pi/180.

    #can angle and detector angle
    tcan1, theta1 = angles
    canAngle = tcan1*PICONV
    theta = theta1*PICONV

    # tsec is the angle the scattered beam makes with the normal to the sample surface.
    tsec = theta1-tcan1

    nlam = len(waves)

    ass = np.ones(nlam)
    assc = np.ones(nlam)
    acsc = np.ones(nlam)
    acc = np.ones(nlam)

    # case where tsec is close to 90 degrees. CALCULATION IS UNRELIABLE
    if (abs(abs(tsec)-90.0) < 1.0):
        #default to 1 for everything
        return ass, assc, acsc, acc
    else:
        #sample & can scattering x-section
        sampleScatt, canScatt = sigs[:2]
        #sample & can absorption x-section                           
        sampleAbs, canAbs = siga[:2]
        #sample & can density                           
        sampleDensity, canDensity = density[:2]
        #thickness of the sample and can
        samThickness, canThickness1, canThickness2 = thick
        
        tsec = tsec*PICONV

        sec1 = 1./math.cos(canAngle)
        sec2 = 1./math.cos(tsec)

        #list of wavelengths
        waves = np.array(waves)

        #sample cross section
        sampleXSection = (sampleScatt + sampleAbs * waves /1.8) * sampleDensity

        #vector version of fact
        vecFact = np.vectorize(Fact)
        fs = vecFact(sampleXSection, samThickness, sec1, sec2)

        sampleSec1, sampleSec2 = calcThicknessAtSec(sampleXSection, samThickness, [sec1, sec2])

        if (sec2 < 0.):
            ass = fs / samThickness
        else:
            ass= np.exp(-sampleSec2) * fs / samThickness

        useCan = (ncan > 1)
        if useCan:
            #calculate can cross section
            canXSection = (canScatt + canAbs * waves /1.8) * canDensity
            assc, acsc, acc = calcFlatAbsCan(ass, canXSection, canThickness1, canThickness2, sampleSec1, sampleSec2, [sec1, sec2])

    return ass, assc, acsc, acc

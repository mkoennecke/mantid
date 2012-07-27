# IDA F2PY Absorption Corrections Wrapper
## Handle selection of .pyd files for absorption corrections
import platform, sys
from IndirectImport import *
if is_supported_f2py_platform():
    fltabs = import_f2py("fltabs")
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
    name = inputWS[:-3] + geom
    assWS = name + '_ass'
    asscWS = name + '_assc'
    acscWS = name + '_acsc'
    accWS = name + '_acc'
    fname = name +'_Abs'
    wrk = workdir + inputWS[:-4]
    wrk.ljust(120,' ')
    for n in range(0,ndet):
        if geom == 'flt':
            angles = [avar, det[n]]
            kill, A1, A2, A3, A4 = fltabs.fltabs(ncan, size, density, sigs,
                siga, angles, waves, n, wrk, 0)
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
        plotOpt='None'):
    Verbose = True
    Save = True
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

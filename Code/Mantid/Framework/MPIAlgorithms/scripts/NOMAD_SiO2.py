#!/usr/bin/env python
back = 'NOM_1000_event.nxs'#'/SNS/NOM/2011_2_1B_SCI/1/1000/preNeXus/NOM_1000_neutron_event.dat' # 1GB
#van = '/SNS/users/pf9/NOM_989_event.nxs'#'/SNS/NOM/2011_2_1B_SCI/1/989/preNeXus/NOM_989_neutron_event.dat' # 73GB
#dia = '/SNS/users/pf9/NOM_990_event.nxs'#'/SNS/NOM/2011_2_1B_SCI/1/990/preNeXus/NOM_990_neutron_event.dat' # 65GB
sio2 = 'NOM_998_event.nxs'#'/NOM-DAS-FS/2011_2_1B_SCI/NOM_998/NOM_998_neutron_event.dat' # 5.6GB
#calib = 'NOM_calibrate_d739_2011_03_29.cal'
calib = 'NOM_2011_03_07.cal'
#mapping = '/SNS/NOM/2010_2_1B_CAL/calibrations/NOM_TS_2010_12_01.dat'

import os

def focus(filename):
    # make a nice workspace name
    wksp = os.path.split(filename)[-1]
    wksp = '_'.join(wksp.split('_')[0:2])

    LoadEventNexus(Filename=filename, OutputWorkspace=wksp,CompressTolerance=.01)
    wksp = mtd[wksp]
    #SortEvents(wksp)
    #CompressEvents(InputWorkspace=wksp, OutputWorkspace=wksp, Tolerance=.01)
    NormaliseByCurrent(InputWorkspace=wksp, OutputWorkspace=wksp)
    AlignDetectors(InputWorkspace=wksp, OutputWorkspace=wksp,
                   CalibrationFile=calib)
    ConvertUnits(InputWorkspace=wksp, OutputWorkspace=wksp,
                 Target='MomentumTransfer')
    Rebin(InputWorkspace=wksp, OutputWorkspace=wksp, Params=(0.02,0.02,50))
    #ConvertUnits(InputWorkspace=wksp, OutputWorkspace=wksp,
    #             Target='dSpacing')
    DiffractionFocussing(InputWorkspace=wksp, OutputWorkspace=wksp,
                         GroupingWorkspace="grouping", PreserveEvents=False)
    #ConvertUnits(InputWorkspace=wksp, OutputWorkspace=wksp,
    #           Target='MomentumTransfer')
    return wksp
    

if __name__ == "__main__":
    #CreateGroupingWorkspace(InstrumentName='NOMAD', GroupNames='NOMAD', OutputWorkspace="grouping")
    CreateGroupingWorkspace(InstrumentName='NOMAD', OldCalFilename=calib, OutputWorkspace="grouping")
    back = focus(back)
    #sio2 = focus(sio2)
    #van = focus(van)
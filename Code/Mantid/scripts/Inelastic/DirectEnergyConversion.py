"""
Conversion class defined for conversion to deltaE for
'direct' inelastic geometry instruments
   
The class defines various methods to allow users to convert their 
files to DeltaE.

Example:

Assuming we have the follwing data files for MARI. 
NOTE: This assumes that the data path for these runs is in the 
Mantid preferences.

mono-sample: 11015
white: 11060
mono-van: 11001
with sample mass 10g and RMM 435.96

reducer = DirectEnergyConversion('MARI')

# Alter defaults if necessary
reducer.normalise_method = 'monitor-2'
reducer.background = False
reducer.fix_ei = True
reducer.save_formats = ['.spe']
# 
Set parameters for these runs
reducer.map_file = 'mari_res.map'
reducer.energy_bins = '-10,0.1,80'

Run the conversion
deltaE_wkspace = reducer.convert_to_energy(11015, 85, 11060, 11001)
"""
import CommonFunctions as common
import diagnostics
from mantidsimple import *
import glob
import os.path
import math

def setup_reducer(inst_name):
    """
    Given an instrument name or prefix this sets up a converter
    object for the reduction
    """
    try:
        return DirectEnergyConversion(inst_name)
    except RuntimeError, exc:
        raise RuntimeError('Unknown instrument "%s", cannot continue' % inst_name)
    

class DirectEnergyConversion(object):
    """
    Performs a convert to energy assuming the provided instrument is an elastic instrument
    """

    def diagnose(self, white, **kwargs):
        """
            Run diagnostics on the provided workspaces.
            
            This method does some additional processing before moving on to the diagnostics:
              1) Computes the white beam integrals, converting to energy
              2) Computes the background integral using the instrument defined range
              3) Computes a total count from the sample
              
            These inputs are passed to the diagnostics functions
    
            Required inputs:
            
              white  - A workspace, run number or filepath of a white beam run. A workspace is assumed to
                       have simple been loaded and nothing else.
            
            Optional inputs:
              sample - A workspace, run number or filepath of a sample run. A workspace is assumed to
                       have simple been loaded and nothing else. (default = None)
              second_white - If provided an additional set of tests is performed on this. (default = None)
              hard_mask  - A file specifying those spectra that should be masked without testing (default=None)
              tiny        - Minimum threshold for acceptance (default = 1e-10)
              huge        - Maximum threshold for acceptance (default = 1e10)
              bkgd_range - A list of two numbers indicating the background range (default=instrument defaults)
              van_out_lo  - Lower bound defining outliers as fraction of median value (default = 0.01)
              van_out_hi  - Upper bound defining outliers as fraction of median value (default = 100.)
              van_lo      - Fraction of median to consider counting low for the white beam diag (default = 0.1)
              van_hi      - Fraction of median to consider counting high for the white beam diag (default = 1.5)
              van_sig  - Error criterion as a multiple of error bar i.e. to fail the test, the magnitude of the\n"
                          "difference with respect to the median value must also exceed this number of error bars (default=0.0)
              samp_zero    - If true then zeroes in the vanadium data will count as failed (default = True)
              samp_lo      - Fraction of median to consider counting low for the white beam diag (default = 0)
              samp_hi      - Fraction of median to consider counting high for the white beam diag (default = 2.0)
              samp_sig  - Error criterion as a multiple of error bar i.e. to fail the test, the magnitude of the\n"
                          "difference with respect to the median value must also exceed this number of error bars (default=3.3)
              variation  - The number of medians the ratio of the first/second white beam can deviate from
                           the average by (default=1.1)
              bleed_test - If true then the CreatePSDBleedMask algorithm is run
              bleed_maxrate - If the bleed test is on then this is the maximum framerate allowed in a tube
              bleed_pixels - If the bleed test is on then this is the number of pixels ignored within the
                             bleed test diagnostic
              print_results - If True then the results are printed to the screen
        """
        lhs_names = lhs_info('names')
        if len(lhs_names) > 0:
            var_name = lhs_names[0]
        else:
            var_name = None

        # Check for any keywords that have not been supplied and put in the defaults
        for par in self.diag_params:
            arg = par.lstrip('diag_')
            if arg not in kwargs:
                kwargs[arg] = getattr(self, par)
        
        # Get the white beam vanadium integrals
        whiteintegrals = self.do_white(white, None, None) # No grouping yet
        if 'second_white' in kwargs:
            second_white = kwargs['second_white']
            if second_white is None:
                del kwargs['second_white']
            else:
                other_whiteintegrals = self.do_white(second_white, None, None) # No grouping yet
                kwargs['second_white'] = other_whiteintegrals

        # Get the background/total counts from the sample if present
        if 'sample' in kwargs:
            sample = kwargs['sample']
            del kwargs['sample']
            # If the bleed test is requested then we need to pass in the sample_run as well
            if kwargs.get('bleed_test', False):
                kwargs['sample_run'] = sample
            
            # Set up the background integrals
            result_ws = common.load_runs(sample)
            self.normalise(result_ws, result_ws, self.normalise_method)
            if 'bkgd_range' in kwargs:
                bkgd_range = kwargs['bkgd_range']
                del kwargs['bkgd_range']
            else:
                bkgd_range = self.background_range
            background_int = Integration(result_ws, OutputWorkspace='background_int',\
                                         RangeLower=bkgd_range[0],RangeUpper=bkgd_range[1], \
                                         IncludePartialBins=True).workspace()
            total_counts = Integration(result_ws, OutputWorkspace='total_counts', IncludePartialBins=True).workspace()
            background_int = ConvertUnits(background_int, background_int, "Energy", AlignBins=0).workspace()
            background_int *= 1.7016e8
            diagnostics.normalise_background(background_int, whiteintegrals, kwargs.get('second_white',None))
            kwargs['background_int'] = background_int
            kwargs['sample_counts'] = total_counts
        
        # If we have a hard_mask, check the instrument name is defined
        if 'hard_mask' in kwargs:
            if 'instrument_name' not in kwargs:
                kwargs['instrument_name'] = self.instr_name
        
        # Check how we should run diag
        if self.diag_spectra is None:
            # Do the whole lot at once
            diagnostics.diagnose(whiteintegrals, **kwargs)
        else:
            banks = self.diag_spectra.split(";")
            bank_spectra = []
            for b in banks:
                token = b.split(",")  # b = "(,)"
                if len(token) != 2: 
                    raise ValueError("Invalid bank spectra specification in diag %s" % self.diag_spectra)
                start = int(token[0].lstrip('('))
                end = int(token[1].rstrip(')'))
                bank_spectra.append((start,end))
            
            for index, bank in enumerate(bank_spectra):
                kwargs['start_index'] = bank[0] - 1
                kwargs['end_index'] = bank[1] - 1
                diagnostics.diagnose(whiteintegrals, **kwargs)
                
        if 'sample_counts' in kwargs:
            DeleteWorkspace('background_int')
            DeleteWorkspace('total_counts')
        if 'second_white' in kwargs:
            DeleteWorkspace(kwargs['second_white'])
        # Return a mask workspace
        diag_mask = ExtractMask(whiteintegrals, OutputWorkspace='diag_mask').workspace()
        DeleteWorkspace(whiteintegrals)

        if var_name is not None and var_name != str(diag_mask):
            result = RenameWorkspace(str(diag_mask), var_name).workspace()
        else:
            result = diag_mask
        self.spectra_masks = result
        return result
    
    def do_white(self, white_run, spectra_masks, map_file): 
        """
        Normalise to a specified white-beam run
        """
        whitews_name = common.create_resultname(white_run, suffix='-white')
        if whitews_name in mtd:
            DeleteWorkspace(whitews_name)
        # Load
        white_data = self.load_data(white_run)
        # Normalise
        white_ws = self.normalise(white_data, whitews_name, self.normalise_method)
        # Units conversion
        ConvertUnits(white_ws, white_ws, "Energy", AlignBins=0)
        # This both integrates the workspace into one bin spectra and sets up common bin boundaries for all spectra
        low = self.wb_integr_range[0]
        upp = self.wb_integr_range[1]
        if low > upp:
            raise ValueError("White beam integration range is inconsistent. low=%d, upp=%d" % (low,upp))
        delta = 2.0*(upp - low)
        Rebin(white_ws, white_ws, [low, delta, upp])
        # Why aren't we doing this...
        #Integration(white_ws, white_ws, RangeLower=low, RangeUpper=upp)

        # Masking and grouping
        white_ws = self.remap(white_ws, spectra_masks, map_file)

        # White beam scale factor
        white_ws *= self.wb_scale_factor

        return white_ws

    def mono_van(self, mono_van, ei_guess, white_run=None, map_file=None,
                 spectra_masks=None, result_name=None, Tzero=None):
        """Convert a mono vanadium run to DeltaE.
        If multiple run files are passed to this function, they are summed into a run and then processed         
        """
        # Load data
        sample_data = self.load_data(mono_van)
        # Create the result name if necessary
        if result_name is None:
            result_name = common.create_resultname(mono_van)

        monovan = self._do_mono(sample_data, sample_data, result_name, ei_guess, 
                                white_run, map_file, spectra_masks, Tzero)
        # Normalize by vanadium sample weight
        monovan /= float(self.van_mass)/float(self.van_rmm)
        return monovan

    def mono_sample(self, mono_run, ei_guess, white_run=None, map_file=None,
                    spectra_masks=None, result_name=None, Tzero=None):
        """Convert a mono-chromatic sample run to DeltaE.
        If multiple run files are passed to this function, they are summed into a run and then processed
        """
        # Load data
        sample_data = self.load_data(mono_run)
        # Create the result name if necessary
        if result_name is None:
            result_name = common.create_resultname(mono_run, prefix=self.instr_name)

        return self._do_mono(sample_data, sample_data, result_name, ei_guess, 
                                  white_run, map_file, spectra_masks, Tzero)


# -------------------------------------------------------------------------------------------
#         This actually does the conversion for the mono-sample and mono-vanadium runs
#  
# -------------------------------------------------------------------------------------------        
    def _do_mono(self, data_ws, monitor_ws, result_name, ei_guess, 
                 white_run=None, map_file=None, spectra_masks=None, Tzero=None):
        """
        Convert units of a given workspace to deltaE, including possible 
        normalisation to a white-beam vanadium run.
        """

        # Special load monitor stuff.    
        if (self.instr_name == "CNCS" or self.instr_name == "HYSPEC"):
            self.fix_ei = True
            ei_value = ei_guess
            if (self.instr_name == "HYSPEC"):
                Tzero=25.0 + 85.0 / (1+math.pow((ei_value/27.0),4.0))
                self.log("Determined T0 of %s for HYSPEC" % str(Tzero))
            if (Tzero is None):
                tzero = (0.1982*(1+ei_value)**(-0.84098))*1000.0
            else:
                tzero = Tzero
            # apply T0 shift
            ChangeBinOffset(data_ws, result_name, -tzero)
            mon1_peak = 0.0
        elif (self.instr_name == "ARCS" or self.instr_name == "SEQUOIA"):
            if 'Filename' in data_ws.getRun(): mono_run = data_ws.getRun()['Filename'].value
            else: raise RuntimeError('Cannot load monitors for event reduction. Unable to determine Filename from mono workspace, it should have been added as a run log.')
                 
	    mtd.sendDebugMessage("mono_run = %s (%s)" % (mono_run,type(mono_run)))
          
            if mono_run.endswith("_event.nxs"):
                loader=LoadNexusMonitors(Filename=mono_run, OutputWorkspace="monitor_ws")    
            elif mono_run.endswith("_event.dat"):
                InfoFilename = mono_run.replace("_neutron_event.dat", "_runinfo.xml")
                loader=LoadPreNexusMonitors(RunInfoFilename=InfoFilename,OutputWorkspace="monitor_ws")
            
            monitor_ws = loader.workspace()
            
            try:
                alg = GetEi(InputWorkspace=monitor_ws, Monitor1Spec=int(self.ei_mon_spectra[0]), Monitor2Spec=int(self.ei_mon_spectra[1]), EnergyEstimate=ei_guess,FixEi=self.fix_ei)
                TzeroCalculated = float(alg.getPropertyValue("Tzero"))
                ei_calc = monitor_ws.getRun().getLogData("Ei").value
            except:
                self.log("Error in GetEi. Using entered values.")
                #monitor_ws.getRun()['Ei'] = ei_value
                ei_value = ei_guess
                AddSampleLog(monitor_ws, 'Ei', ei_value, "Number")
                ei_calc = None
                TzeroCalculated = Tzero
                
            # Set the tzero to be the calculated value
            if (TzeroCalculated is None):
                tzero = 0.0
            else:    
                tzero = TzeroCalculated
            
            # If we are fixing, then use the guess if given
            if (self.fix_ei):
                ei_value = ei_guess
                # If a Tzero has been entered, use it, if we are fixing.
                if (Tzero is not None):
                    tzero = Tzero
            else:
                if (ei_calc is not None):
                    ei_value = ei_calc
                else:
                    ei_value = ei_guess
            
            mon1_peak = 0.0
            # apply T0 shift
            ChangeBinOffset(data_ws, result_name, -tzero)
        else:
            # Do ISIS stuff for Ei
            # Both are these should be run properties really
            ei_value, mon1_peak = self.get_ei(monitor_ws, result_name, ei_guess)

        # As we've shifted the TOF so that mon1 is at t=0.0 we need to account for this in FlatBackground and normalisation
        bin_offset = -mon1_peak

        # Get the workspace the converted data will end up in
        result_ws = mtd[result_name]
        
        # For event mode, we are going to histogram in energy first, then go back to TOF
        if (self.facility == "SNS"):
            if self.background == True:
                # Extract the time range for the background determination before we throw it away
                background_bins = "%s,%s,%s" % (self.background_range[0] + bin_offset, (self.background_range[1]-self.background_range[0]), self.background_range[1] + bin_offset)
                Rebin(result_ws, "background_origin_ws", background_bins)
            # Convert to Et
            ConvertUnits(result_ws, "_tmp_energy_ws", Target="DeltaE",EMode="Direct", EFixed=ei_value)
            RenameWorkspace("_tmp_energy_ws", result_ws)
            # Histogram
            Rebin(result_ws, "_tmp_rebin_ws", self.energy_bins, PreserveEvents=False)
            RenameWorkspace("_tmp_rebin_ws", result_ws)
            # Convert back to TOF
            ConvertUnits(result_ws, result_ws, Target="TOF",EMode="Direct", EFixed=ei_value)
        else:
            # TODO: This algorithm needs to be separated so that it doesn't actually
            # do the correction as well so that it can be moved next to LoadRaw where
            # it belongs
            if self.det_cal_file == None:
                if 'Filename' in result_ws.getRun(): 
                    filename = result_ws.getRun()['Filename'].value
                else:
                    raise RuntimeError('Cannot run LoadDetectorInfo: "Filename" property not found on input mono workspace')
                if self.relocate_dets: self.log('Moving detectors to positions specified in RAW file.')
            	LoadDetectorInfo(result_ws, result_ws.getRun()['Filename'].value, self.relocate_dets)
            else:
                self.log('Loading detector info from file ' + self.det_cal_file)
                self.log('Raw file detector header is superceeded') 
                if self.relocate_dets: self.log('Moving detectors to positions specified in cal file.')
                LoadDetectorInfo(result_ws, self.det_cal_file, self.relocate_dets)

        if self.background == True:
            # Remove the count rate seen in the regions of the histograms defined as the background regions, if the user defined a region
            ConvertToDistribution(result_ws)    
            if (self.facility == "SNS"):
                FlatBackground("background_origin_ws", "background_ws", self.background_range[0] + bin_offset, self.background_range[1] + bin_offset, '', 'Mean', 'Return Background')
                # Delete the raw data background region workspace
                mtd.deleteWorkspace("background_origin_ws")
                # Convert to distribution to make it compatible with the data workspace (result_ws).
                ConvertToDistribution("background_ws") 
                # Subtract the background
                Minus(result_ws, "background_ws", result_ws)
                # Delete the determined background 
                mtd.deleteWorkspace("background_ws")
            else:
                FlatBackground(result_ws, result_ws, self.background_range[0] + bin_offset, self.background_range[1] + bin_offset, '', 'Mean')
            ConvertFromDistribution(result_ws)  

        # Normalise using the chosen method
        # TODO: This really should be done as soon as possible after loading
        self.normalise(result_ws, result_ws, self.normalise_method, range_offset=bin_offset)

        # This next line will fail the SystemTests
        #ConvertUnits(result_ws, result_ws, Target="DeltaE",EMode='Direct', EFixed=ei_value)
        # But this one passes...
        ConvertUnits(result_ws, result_ws, Target="DeltaE",EMode='Direct')
        
        if not self.energy_bins is None:
            Rebin(result_ws, result_ws, self.energy_bins,PreserveEvents=False)
        
        if self.apply_detector_eff:
            if (self.facility == "SNS"):
                # Need to be in lambda for detector efficiency correction
                ConvertUnits(result_ws, result_ws, Target="Wavelength", EMode="Direct", EFixed=ei_value)
                He3TubeEfficiency(result_ws, result_ws)
                ConvertUnits(result_ws, result_ws, Target="DeltaE",EMode='Direct', EFixed=ei_value)
            else:
                DetectorEfficiencyCor(result_ws, result_ws)

        # Ki/Kf Scaling...
        if self.apply_kikf_correction:
            # TODO: Write log message
            CorrectKiKf(result_ws, result_ws, EMode='Direct')

        # Make sure that our binning is consistent
        if not self.energy_bins is None:
            Rebin(result_ws, result_ws, self.energy_bins)
        
        # Masking and grouping
        result_ws = self.remap(result_ws, spectra_masks, map_file)

        ConvertToDistribution(result_ws)
        # White beam correction
        if white_run is not None:
            white_ws = self.do_white(white_run, spectra_masks, map_file)
            result_ws /= white_ws
        
        # Overall scale factor
        result_ws *= self.scale_factor
        return result_ws

#-------------------------------------------------------------------------------

    def convert_to_energy(self, mono_run, ei, white_run=None, mono_van=None,\
                          abs_ei=None, abs_white_run=None, save_path=None, Tzero=None, \
                          motor=None, offset=None):
        """
        One-shot function to convert the given runs to energy
        """
        # Check if we need to perform the absolute normalisation first
        if not mono_van is None:
            if abs_ei is None:
                abs_ei = ei
            mapping_file = self.abs_map_file
            spectrum_masks = self.spectra_masks 
            monovan_wkspace = self.mono_van(mono_van, abs_ei, abs_white_run, mapping_file, spectrum_masks)
            
            # TODO: Need a better check than this...
            if (abs_white_run is None):
                self.log("Performing Normalisation to Mono Vanadium.")
                norm_factor = self.calc_average(monovan_wkspace)
            else:
                self.log("Performing Absolute Units Normalisation.")
                # Perform Abs Units...
                norm_factor = self.monovan_abs(monovan_wkspace)
            mtd.deleteWorkspace(monovan_wkspace.getName())
        else:
            norm_factor = None

        # Figure out what to call the workspace 
        result_name = mono_run
        if not result_name is None:
            result_name = common.create_resultname(mono_run)
        
        # Main run file conversion
        sample_wkspace = self.mono_sample(mono_run, ei, white_run, self.map_file,
                                          self.spectra_masks, result_name, Tzero)
        if not norm_factor is None:
            sample_wkspace /= norm_factor
        
        #calculate psi from sample environment motor and offset 
        if (offset is None):
            self.motor_offset = 0
        else:
            self.motor_offset = float(offset)
        
        self.motor=0
        if not (motor is None):
        # Check if motor name exists    
            if sample_wkspace.getRun().hasProperty(motor):
                self.motor=sample_wkspace.getRun()[motor].value[0]
                self.log("Motor value is %s" % self.motor)
            else:
                self.log("Could not find such sample environment log. Will use psi=offset")
        self.psi = self.motor+self.motor_offset
        # Save then finish
        self.save_results(sample_wkspace, save_path)
        # Clear loaded raw data to free up memory
        common.clear_loaded_data()
        
        return sample_wkspace

#----------------------------------------------------------------------------------
#                        Reduction steps
#----------------------------------------------------------------------------------

     
    def get_ei(self, input_ws, resultws_name, ei_guess):
        """
        Calculate incident energy of neutrons
        """
        fix_ei = str(self.fix_ei).lower()
        if fix_ei == 'true':
            fix_ei = True
        elif fix_ei == 'false':
            fix_ei = False
        elif fix_ei == 'fixei':
            fix_ei = True
        else:
            raise TypeError('Unknown option passed to get_ei "%s"' % fix_ei)

        # Calculate the incident energy
        alg = GetEi(InputWorkspace=input_ws, Monitor1Spec=int(self.ei_mon_spectra[0]), Monitor2Spec=int(self.ei_mon_spectra[1]), EnergyEstimate=ei_guess,FixEi=fix_ei)
        mon1_peak = float(alg.getPropertyValue("FirstMonitorPeak"))
        mon1_index = int(alg.getPropertyValue("FirstMonitorIndex"))
        ei = input_ws.getSampleDetails().getLogData("Ei").value
        # Adjust the TOF such that the first monitor peak is at t=0
        ChangeBinOffset(input_ws, resultws_name, -mon1_peak)
        mon1_det = input_ws.getDetector(mon1_index)
        mon1_pos = mon1_det.getPos()
        src_name = input_ws.getInstrument().getSource().getName()
        MoveInstrumentComponent(resultws_name, src_name, X=mon1_pos.getX(), Y=mon1_pos.getY(), Z=mon1_pos.getZ(), RelativePosition=False)
        return ei, mon1_peak

    def remap(self, result_ws, spec_masks, map_file):
        """
        Mask and group detectors based on input parameters
        """
        if not spec_masks is None:
            MaskDetectors(result_ws, MaskedWorkspace=spec_masks)
        if not map_file is None:
            GroupDetectors(result_ws, result_ws, map_file, KeepUngroupedSpectra=0, Behaviour='Average')

        return mtd[str(result_ws)]

    def normalise(self, data_ws, result_ws, method, range_offset=0.0):
        """
        Apply normalisation using specified source
        """
        # Make sure we don't call this twice
        method = method.lower()
        done_log = "DirectInelasticReductionNormalisedBy"
        if done_log in data_ws.getRun() or method == 'none':
            if str(data_ws) != str(result_ws):
                CloneWorkspace(InputWorkspace=data_ws, OutputWorkspace=result_ws)
            output = mtd[str(result_ws)]
        elif method == 'monitor-1':
            range_min = self.mon1_norm_range[0] + range_offset
            range_max = self.mon1_norm_range[1] + range_offset
            NormaliseToMonitor(InputWorkspace=data_ws, OutputWorkspace=result_ws, MonitorSpectrum=int(self.mon1_norm_spec), 
                               IntegrationRangeMin=range_min, IntegrationRangeMax=range_max,IncludePartialBins=True)
            output = mtd[str(result_ws)]
        elif method == 'current':
            NormaliseByCurrent(InputWorkspace=data_ws, OutputWorkspace=result_ws)
            output = mtd[str(result_ws)]
        else:
            raise RuntimeError('Normalisation scheme ' + reference + ' not found. It must be one of monitor-1, current, peak or none')

        # Add a log to the workspace to say that the normalisation has been done
        AddSampleLog(Workspace=output, LogName=done_log,LogText=method)
        return output
            
    def calc_average(self, data_ws):
        """
        Compute the average Y value of a workspace.
        
        The average is computed by collapsing the workspace to a single bin per spectra then masking
        masking out detectors given by the FindDetectorsOutsideLimits and MedianDetectorTest algorithms.
        The average is then the computed as the using the remainder and factoring in their errors as weights, i.e.
        
            average = sum(Yvalue[i]*weight[i]) / sum(weights)
            
        where only those detectors that are unmasked are used and the weight[i] = 1/errorValue[i].
        """
        e_low = self.monovan_integr_range[0]
        e_upp = self.monovan_integr_range[1]
        if e_low > e_upp:
            raise ValueError("Inconsistent mono-vanadium integration range defined!")
        Rebin(data_ws, data_ws, [e_low, 2.*(e_upp-e_low), e_upp])
        ConvertToMatrixWorkspace(data_ws, data_ws)

        args = {}
        args['tiny'] = self.diag_tiny
        args['huge'] = self.diag_huge
        args['van_out_lo'] = self.monovan_lo_bound
        args['van_out_hi'] = self.monovan_hi_bound
        args['van_lo'] = self.monovan_lo_frac
        args['van_hi'] = self.monovan_hi_frac
        args['van_sig'] = self.diag_samp_sig

        diagnostics.diagnose(data_ws, **args)
        monovan_masks = ExtractMask(data_ws, OutputWorkspace='monovan_masks').workspace()
        MaskDetectors(data_ws, MaskedWorkspace=monovan_masks)
        DeleteWorkspace(Workspace=monovan_masks)

        ConvertFromDistribution(data_ws)
        nhist = data_ws.getNumberHistograms()
        average_value = 0.0
        weight_sum = 0.0
        for i in range(nhist):
            try:
                det = data_ws.getDetector(i)
            except Exception:
                continue
            if det.isMasked():
                continue
            y_value = data_ws.readY(i)[0]
            if y_value != y_value:
                continue
            weight = 1.0/data_ws.readE(i)[0]
            average_value += y_value * weight
            weight_sum += weight
        return average_value / weight_sum

    def monovan_abs(self, ei_workspace):
        """Calculates the scaling factor required for normalisation to absolute units.
        The given workspace must contain an Ei value. This will have happened if GetEi
        has been run
        """
        absnorm_factor = self.calc_average(ei_workspace)
        #  Scale by vanadium cross-section which is energy dependent up to a point
        run = ei_workspace.getRun()
        try:
            ei_prop = run['Ei']
        except KeyError:
            raise RuntimeError('The given workspace "%s" does not contain an Ei value. Run GetEi first.' % str(ei_workspace))
        
        ei_value = ei_prop.value
        if ei_value >= 200.0:
            xsection = 421.0
        else:
            xsection = 400.0 + (ei_value/10.0)
        absnorm_factor /= xsection
        return absnorm_factor * (float(self.sample_mass)/float(self.sample_rmm))

    def save_results(self, workspace, save_path, formats = None):
        """
        Save the result workspace to the specfied filename using the list of formats specified in 
        formats. If formats is None then the default list is used
        """
        if save_path is None:
            save_path = workspace.getName()
        elif os.path.isdir(save_path):
            save_path = os.path.join(save_path, workspace.getName())
        elif save_path == '':
            raise ValueError('Empty filename is not allowed for saving')
        else:
            pass

        if formats is None:
            formats = self.save_formats
        if type(formats) == str:
            formats = [formats]
        #Make sure we just have a file stem
        save_path = os.path.splitext(save_path)[0]
        for ext in formats:
            filename = save_path + ext
            if ext == '.spe':
                SaveSPE(workspace, filename)
            elif ext == '.nxs':
                SaveNexus(workspace, filename)
            elif ext == '.nxspe':
                SaveNXSPE(workspace, filename, KiOverKfScaling=self.apply_kikf_correction,Psi=self.psi)
            else:
                self.log('Unknown file format "%s" encountered while saving results.')
    

    #-------------------------------------------------------------------------------
    def load_data(self, runs):
        """
        Load a run or list of runs. If a list of runs is given then
        they are summed into one.
        """
        result_ws = common.load_runs(runs, sum=True)
        self.setup_mtd_instrument(result_ws)
        return result_ws

    #---------------------------------------------------------------------------
    # Behind the scenes stuff
    #---------------------------------------------------------------------------

    def __init__(self, instr_name):
        """
        Constructor
        """
        self._to_stdout = True
        self._log_to_mantid = False
        self._idf_values_read = False
        # Instrument and default parameter setup
        self.initialise(instr_name)

    def initialise(self, instr_name):
        """
        Initialise the attributes of the class
        """
        # Instrument name might be a prefix, query Mantid for the full name
        self.instr_name = mtd.settings.facility().instrument(instr_name).name()
        mtd.settings['default.instrument'] = self.instr_name
        self.setup_mtd_instrument()
        # Initialize the default parameters from the instrument as attributes of the
        # class
        self.init_params()

    def setup_mtd_instrument(self, workspace = None):
        if workspace != None:
            self.instrument = workspace.getInstrument()
        else:
            # Load an empty instrument if one isn't already there
            idf_dir = mtd.getConfigProperty('instrumentDefinition.directory')
            instr_pattern = os.path.join(idf_dir,self.instr_name + '*_Definition.xml')
            idf_files = glob.glob(instr_pattern)
            if len(idf_files) > 0:
                tmp_ws_name = '__empty_' + self.instr_name
                if not mtd.workspaceExists(tmp_ws_name):
                    LoadEmptyInstrument(idf_files[0],tmp_ws_name)
                self.instrument = mtd[tmp_ws_name].getInstrument()
            else:
                self.instrument = None
                raise RuntimeError('Cannot load instrument for prefix "%s"' % self.instr_name)
        # Initialise IDF parameters
        self.init_idf_params()

        
    def init_params(self):
        """
        Attach analysis arguments that are particular to the ElasticConversion 
        """
        self.save_formats = ['.spe','.nxs','.nxspe']
        self.fix_ei=False
        self.energy_bins = None
        self.background = False
        self.normalise_method = 'monitor-1'
        self.map_file = None
        
        if (self.instr_name == "CNCS" or self.instr_name == "ARCS" or self.instr_name == "SEQUOIA" or self.instr_name == "HYSPEC"):
            self.facility = "SNS"
            self.normalise_method  = 'current'
        else:
            self.facility = str(mtd.settings.facility())
        
        # The Ei requested
        self.ei_requested = None
        self.monitor_workspace = None
        
        self.time_bins = None
                
        # Motor names
        self.motor = None
        self.motor_offset = None
        self.psi = None
                
        # Detector diagnosis
        self.spectra_masks = None
        
        # Absolute normalisation
        self.abs_map_file = None
        self.abs_spectra_masks = None
        self.sample_mass = 1.0
        self.sample_rmm = 1.0
        
        # Detector Efficiency Correction
        self.apply_detector_eff = True
        
        # Ki/Kf factor correction
        self.apply_kikf_correction = True
    	
    	# Detector calibration file
    	self.det_cal_file = None
        # Option to move detector positions based on the information
        self.relocate_dets = False
     
    def init_idf_params(self):
        """
        Initialise the parameters from the IDF file if necessary
        """
        if self._idf_values_read == True:
            return

        self.ei_mon_spectra = [int(self.get_default_parameter("ei-mon1-spec")), int(self.get_default_parameter("ei-mon2-spec"))]
        self.scale_factor = self.get_default_parameter("scale-factor")
        self.wb_scale_factor = self.get_default_parameter("wb-scale-factor")
        self.wb_integr_range = [self.get_default_parameter("wb-integr-min"), self.get_default_parameter("wb-integr-max")]
        self.mon1_norm_spec = int(self.get_default_parameter("norm-mon1-spec"))
        self.mon1_norm_range = [self.get_default_parameter("norm-mon1-min"), self.get_default_parameter("norm-mon1-max")]
        self.background_range = [self.get_default_parameter("bkgd-range-min"), self.get_default_parameter("bkgd-range-max")]
        self.monovan_integr_range = [self.get_default_parameter("monovan-integr-min"), self.get_default_parameter("monovan-integr-max")]
        self.van_mass = self.get_default_parameter("vanadium-mass")
        self.van_rmm = self.get_default_parameter("vanadium-rmm")

        # Diag 
        self.diag_params = ['diag_tiny', 'diag_huge', 'diag_samp_zero', 'diag_samp_lo', 'diag_samp_hi','diag_samp_sig',\
                            'diag_van_out_lo', 'diag_van_out_hi', 'diag_van_lo', 'diag_van_hi', 'diag_van_sig', 'diag_variation']
        # Add an attribute for each of them
        for par in self.diag_params:
            setattr(self, par, self.get_default_parameter(par))
            
        # Bleed options
        try:
            self.diag_bleed_test = self.get_default_parameter('diag_bleed_test')
            self.diag_bleed_maxrate = self.get_default_parameter('diag_bleed_maxrate')
            self.diag_bleed_pixels = int(self.get_default_parameter('diag_bleed_pixels'))
            self.diag_params.extend(['diag_bleed_maxrate','diag_bleed_pixels'])
        except ValueError:
            self.diag_bleed_test = False
        self.diag_params.append('diag_bleed_test')

        # Do we have specified spectra to diag over
        try:
            self.diag_spectra = self.instrument.getStringParameter("diag_spectra")[0]
        except Exception:
            self.diag_spectra = None
        
        # Absolute units
        self.monovan_lo_bound = self.get_default_parameter('monovan_lo_bound')
        self.monovan_hi_bound = self.get_default_parameter('monovan_hi_bound')
        self.monovan_lo_frac = self.get_default_parameter('monovan_lo_frac')
        self.monovan_hi_frac = self.get_default_parameter('monovan_hi_frac')

        # Mark IDF files as read
        self._idf_values_read = True

    def get_default_parameter(self, name):
        if self.instrument is None:
            raise ValueError("Cannot init default parameter, instrument has not been loaded.")
        values = self.instrument.getNumberParameter(name)
        if len(values) != 1:
            raise ValueError('Instrument parameter file does not contain a definition for "%s". Cannot continue' % name)
        return values[0]
    
    def log(self, msg):
        """Send a log message to the location defined
        """
        if self._to_stdout:
            print msg
        if self._log_to_mantid:
            mtd.sendLogMessage(msg)

#-----------------------------------------------------------------

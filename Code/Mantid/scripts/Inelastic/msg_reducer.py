# msg_reducer.py
# Reducers for use by ISIS Molecular Spectroscopy Group
import os.path

from mantid.simpleapi import *
from mantid.kernel import config
import reduction.reducer as reducer
import inelastic_indirect_reduction_steps as steps

class MSGReducer(reducer.Reducer):
    """This is the base class for the reducer classes to be used by the ISIS
    Molecular Spectroscopy Group (MSG). It exists to serve the functions that
    are common to both spectroscopy and diffraction workflows in the hopes of
    providing a semi-consistent interface to both.
    """
    
    _instrument_name = None #: Name of the instrument used in experiment
    _sum = False #: Whether to sum input files or treat them sequentially
    _monitor_index = None #: Index of Monitor specturm
    _multiple_frames = False
    _detector_range = [-1, -1]
    _masking_detectors = []
    _parameter_file = None
    _rebin_string = None
    _fold_multiple_frames = True
    _save_formats = []
    _info_table_props = None
    _extra_load_opts = {}
    
    def __init__(self):
        super(MSGReducer, self).__init__()
        
    def pre_process(self):
        self._reduction_steps = []
        
        loadData = steps.LoadData()
        loadData.set_ws_list(self._data_files)
        loadData.set_sum(self._sum)
        loadData.set_monitor_index(self._monitor_index)
        loadData.set_detector_range(self._detector_range[0],
            self._detector_range[1])
        loadData.set_parameter_file(self._parameter_file)
        loadData.set_extra_load_opts(self._extra_load_opts)
        loadData.execute(self, None)
        
        self._multiple_frames = loadData.is_multiple_frames()
        self._masking_detectors = loadData.get_mask_list()
        
        if( self._info_table_props is not None ):
            wsNames = loadData.get_ws_list().keys()
            wsNameList = ", ".join(wsNames)
            propsList = ", ".join(self._info_table_props)
            CreateLogPropertyTable(
                OutputWorkspace="RunInfo",
                InputWorkspaces=wsNameList,
                LogPropertyNames=propsList,
                GroupPolicy="First")
        
        if ( self._sum ):
            self._data_files = loadData.get_ws_list()
            
        self._setup_steps()
    
    def create_info_table(self):
        """Sets the names of properties retrieve from the log when creating
        an info table.
        """
        
        inst_name = config.getInstrument().name()
        inst = mtd["__empty_" + inst_name].getInstrument()
        props = inst.getStringParameter('Workflow.InfoTable')
        
        if props:
            self._info_table_props = props
        else:
            logger.error("Instrument does not have Workflow.InfoTable " +
                "defined in its parameter file.  Unable to create info table for runs.")

    def set_detector_range(self, start, end):
        """Sets the start and end detector points for the reduction process.
        These numbers are to be the *workspace index*, not the spectrum number.
        Example:
            reducer.set_detector_range(2,52)
        """
        if ( not isinstance(start, int) ) or ( not isinstance(end, int) ):
            raise TypeError("start and end must be integer values")
        self._detector_range = [ start, end ]
        
    def set_fold_multiple_frames(self, value):
        """When this is set to False, the reducer will not run the FoldData
        reduction step or any step which appears after it in the reduction
        chain.
        This will only affect data which would ordinarily have used this
        function (ie TOSCA on multiple frames).
        """
        if not isinstance(value, bool):
            raise TypeError("value must be of boolean type")
        self._fold_multiple_frames = value
        
    def set_instrument_name(self, instrument):
        """Unlike the SANS reducers, we do not create a class to describe the
        instruments. Instead, we load the instrument and parameter file and
        query it for information.
        Raises:
            * ValueError if an instrument name is not provided.
            * RuntimeError if IDF could not be found or is invalid.
            * RuntimeError if workspace index of the Monitor could not be
                determined.
        Example use:
            reducer.set_instrument_name("IRIS")
        """
        if not isinstance(instrument, str):
            raise ValueError("Instrument name must be given.")
        self._instrument_name = instrument
        self._load_empty_instrument()
        self._get_monitor_index()
        if ( self._monitor_index is None ):
            raise RuntimeError("Could not find Monitor in Instrument.")
        
    def set_parameter_file(self, file):
        """Sets the parameter file to be used in the reduction. The parameter
        file will contain some settings that are used throughout the reduction
        process.
        Note: This is *not* the base parameter file, ie "IRIS_Parameters.xml"
        but, rather, the additional parameter file.
        """
        if self._instrument_name is None:
            raise ValueError("Instrument name not set.")
        self._parameter_file = \
            os.path.join(config["parameterDefinition.directory"], file)
        LoadParameterFile(Workspace=self._workspace_instrument,Filename= 
            self._parameter_file)

    def set_rebin_string(self, rebin):
        """Sets the rebin string to be used with the Rebin algorithm.
        """
        if not isinstance(rebin, str):
            raise TypeError("rebin variable must be of string type")
        self._rebin_string = rebin
        
    def set_sum_files(self, value):
        """Mark whether multiple runs should be summed together for the process
        or treated individually.
        The default value for this is False.
        """
        if not isinstance(value, bool):
            raise TypeError("value must be either True or False (boolean)")
        self._sum = value
        
    def set_save_formats(self, formats):
        """Selects the save formats in which to export the reduced data.
        formats should be a list object of strings containing the file
        extension that signifies the type.
        For example:
            reducer.set_save_formats(['nxs', 'spe'])
        Tells the reducer to save the final result as a NeXuS file, and as an
        SPE file.
        Please see the documentation for the SaveItem reduction step for more
        details.
        """
        if not isinstance(formats, list):
            raise TypeError("formats variable must be of list type")
        self._save_formats = formats

    def append_load_option(self, name, value):
        """
           Additional options for the Load call, require name & value
           of property
        """
        self._extra_load_opts[name] = value
        
    def get_result_workspaces(self):
        """Returns a Python list object containing the names of the workspaces
        processed at the last reduction step. Using this, you can incorporate
        the reducer into your own scripts.
        It will only be effective after the reduce() function has run.
        Example:
            wslist = reducer.get_result_workspaces()
            plotSpectrum(wslist, 0) # Plot the first spectrum of each of
                    # the result workspaces
        """
        nsteps = len(self._reduction_steps)
        for i in range(0, nsteps):
            try:
                step = self._reduction_steps[nsteps-(i+1)]
                return step.get_result_workspaces()
            except AttributeError:
                pass
            except IndexError:
                raise RuntimeError("None of the reduction steps implement "
                    "the get_result_workspaces() method.")

    def _load_empty_instrument(self):
        """Returns an empty workspace for the instrument.
        Raises: 
            * ValueError if no instrument is selected.
            * RuntimeError if there is a problem with the IDF.
        """
        if self._instrument_name is None:
            raise ValueError('No instrument selected.')
        self._workspace_instrument = '__empty_' + self._instrument_name
        if not mtd.doesExist(self._workspace_instrument):
            idf_dir = config.getString('instrumentDefinition.directory')
            idf = idf_dir + self._instrument_name + '_Definition.xml'
            try:
                LoadEmptyInstrument(Filename=idf,OutputWorkspace= self._workspace_instrument)
            except RuntimeError:
                raise ValueError('Invalid IDF')
        return mtd[self._workspace_instrument]

    def _get_monitor_index(self):
        """Determine the workspace index of the first monitor spectrum.
        """
        workspace = self._load_empty_instrument()
        for counter in range(0, workspace.getNumberHistograms()):
            try:
                detector = workspace.getDetector(counter)
            except RuntimeError:
                pass
            if detector.isMonitor():
                self._monitor_index = counter
                return

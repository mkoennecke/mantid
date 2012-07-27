"""
    Instrument interface factory.
    This module is responsible for the association between an instrument name
    and its corresponding interface class. 
"""
#from hfir_interface import HFIRInterface as HFIRInterfaceOld
#from eqsans_interface import EQSANSInterface
from hfir_interface_dev import HFIRInterface
from eqsans_interface_dev import EQSANSInterface
from example_interface import ExampleInterface
from reflectometer_l_interface_dev import REFLInterface
from reflectometer_m_interface_dev import REFMInterface
from reflectometer_sf_interface_dev import REFLSFInterface
from dgs_interface_dev import DgsInterface

INSTRUMENT_DICT = {"HFIR": {"BIOSANS": HFIRInterface, 
                            "GPSANS": HFIRInterface},
                   "SNS":  {"ARCS": DgsInterface,
                            "CNCS": DgsInterface,
                            "EQSANS": EQSANSInterface,
                            "HYSPEC": DgsInterface,
                            "REFL": REFLInterface,
                            "REFLSF": REFLSFInterface,
                            "REFM": REFMInterface,
                            "SEQUOIA": DgsInterface,
                            "Example": ExampleInterface}
                  }               

def instrument_factory(instrument_name, settings=None):
    for facility in INSTRUMENT_DICT:
        for instrument in INSTRUMENT_DICT[facility]:
            if str(instrument_name).strip()==instrument:
                return INSTRUMENT_DICT[facility][instrument](instrument, settings=settings)

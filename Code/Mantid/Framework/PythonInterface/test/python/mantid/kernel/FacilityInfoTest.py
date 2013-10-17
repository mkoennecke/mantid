import unittest
from mantid.kernel import FacilityInfo, InstrumentInfo, ConfigService

class FacilityInfoTest(unittest.TestCase):

    def test_construction_raies_an_error(self):
        self.assertRaises(RuntimeError, FacilityInfo)

    def _get_test_facility(self):
        return ConfigService.getFacility("ISIS")
    
    def test_attributes_are_as_expected(self):
        test_facility = self._get_test_facility()
        
        self.assertEquals(test_facility.name(), "ISIS")
        self.assertEquals(test_facility.zeroPadding(), 5)
        self.assertEquals(test_facility.delimiter(), "")
        self.assertEquals(len(test_facility.extensions()), 7)
        self.assertEquals(test_facility.preferredExtension(), ".nxs")
        self.assertEquals(test_facility.getSoapEndPoint(), "https://facilities01.esc.rl.ac.uk:443/ICATService/ICAT")
        self.assertEquals(len(test_facility.archiveSearch()), 1)
        self.assertEquals(len(test_facility.instruments()), 32)
        self.assertEquals(len(test_facility.instruments("Neutron Diffraction")), 11)
        self.assertTrue(isinstance(test_facility.instrument("WISH"), InstrumentInfo))
        self.assertEquals(test_facility.catalogName(), "ICat3Catalog")
        self.assertEquals(test_facility.liveListener(), "ISISHistoDataListener")

if __name__ == '__main__':
    unittest.main()

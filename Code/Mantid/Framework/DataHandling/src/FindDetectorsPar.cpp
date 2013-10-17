/*WIKI* 

Identifies geometrical parameters of detectors and groups of detectors after the workspaces were grouped using ASCII or XML map file. 
Located in DataHangdling\Instrument\Detectors group and intended to be used as Child Algorithm of saveNXSPE algorithm, though can be deployed independently. Dynamic casting from iAlgorithm and accessors functions return calculated parameters to saveNXSPE when FindDetectorsPar used as the Child Algorithm of saveNXSPE procedure; 


Internal Child Algorithm identifies the group topology, namely if a group of detectors is arranged into a rectangular shape or in a ring. The algorithm calculates the geometrical centre of the detectors group and 6 points, located within +-1/4 width of the first detector of the group. If the centre or any of these points belong to the group of the detectors itself, the group assumed to have a rectangular topology, and if not -- the cylindrical one (ring).

Single detector defined to have the rectangular shape.

After identifying the topology, the parameters are calculated using formulas for angles in Cartesian or Cylindrical coordinate systems accordingly

== [[SavePAR|par]] and [[SavePHX|phx]] files ==

These files are ascii files which are used to describe the combined detectors geometry defined by map files. There are no reasons for you to use it unless this Mantid algorithm is working unsatisfactory for you. In this case you can quickly modify and use par file until this algorithm is modified. It is your responsibility then to assure the correspondence between mapped detectors and parameters in the par file. 

The par files are simple ASCII files with the following columns:

        1st column      sample-detector distance (m)
        2nd  "          scattering angle (deg)
        3rd  "          azimuthal angle (deg)   (west bank = 0 deg, north bank = -90 deg etc.)   (Note the reversed sign convention cf .phx files)
        4th  "          width  (m)
        5th  "          height (m)


When processed by this algorithm, 4th and 5th column are transformed into angular values.

[[SavePHX|Phx]] files are Mslice phx files, which do not contain secondary flight path. This path is calculated by the algorithm from the data in the instrument description and the angular values are calculated as in nxspe file. There are no reason to use phx files to build nxspe files at the moment unless you already have one and need to repeat your previous results with Mantid.


*WIKI*/
#include "MantidDataHandling/FindDetectorsPar.h"
#include "MantidKernel/Logger.h"
#include "MantidKernel/ArrayProperty.h"
#include "MantidAPI/FileProperty.h"
#include "MantidKernel/Exception.h"
#include "MantidGeometry/Objects/BoundingBox.h"
#include "MantidGeometry/Instrument/DetectorGroup.h"
#include "MantidAPI/WorkspaceValidators.h"
#include "MantidAPI/AnalysisDataService.h"
#include "MantidAPI/ITableWorkspace.h"
#include "MantidAPI/TableRow.h"
#include <Poco/File.h>
#include <limits>
#include <iostream>

namespace Mantid
{
namespace DataHandling
{
// Register the algorithm into the algorithm factory
DECLARE_ALGORITHM(FindDetectorsPar)

    //----------------------------------------------------------------
Kernel::Logger& FindDetectorsPar::g_log=Kernel::Logger::get("DataHandling");

/// Sets documentation strings for this algorithm
void FindDetectorsPar::initDocs()
{
  this->setWikiSummary("Calculates angular positions and sizes of detectors and groups of detectors after the workspace was grouped using ASCII or XML map file. "
                       "The results can be used to identify the positions of detectors in reciprocal space. Primary usage -- Child Algorithm of [[SaveNXSPE]], [[SavePAR]] or [[SavePHX]] algorithm.");
  this->setOptionalMessage("The algorithm returns the angular parameters and second flight path for a workspace detectors (data, usually availble in par or phx file)");
}


using namespace Kernel;
using namespace API;
// nothing here according to mantid
FindDetectorsPar::FindDetectorsPar():
return_linear_ranges(false)
{};
FindDetectorsPar::~FindDetectorsPar(){};

void FindDetectorsPar::init()
{
  auto wsValidator = boost::make_shared<CompositeValidator>() ;
  wsValidator->add<API::InstrumentValidator>();
  wsValidator->add<API::CommonBinsValidator>();
  // input workspace
  declareProperty(
     new WorkspaceProperty<>("InputWorkspace","", Direction::Input,wsValidator),
    "The name of the workspace that will be used as input for the algorithm" );
 //
  declareProperty("ReturnLinearRanges",false,"if set to true, the algorithm would return linear detector's ranges (dx,dy) rather then angular ranges (dAzimuthal,dPolar)");
 // optional par or phx file
  std::vector<std::string> fileExts(2);
    fileExts[0]=".par";
    fileExts[1]=".phx";
  
    declareProperty(new FileProperty("ParFile","not_used.par",FileProperty::OptionalLoad, fileExts),
    "An optional file that contains of the list of angular parameters for the detectors and detectors groups;\n"
    "If specified, will use data from file instead of the data, calculated from the instument description");

   //
  declareProperty("OutputParTable","","If not empty, a name of a table workspace which "
      " will contain the calculated par or phx values for the detectors");


}

void 
FindDetectorsPar::exec()
{

 // Get the input workspace
  const MatrixWorkspace_sptr inputWS = this->getProperty("InputWorkspace");
  if(inputWS.get()==NULL){
      throw(Kernel::Exception::NotFoundError("can not obtain InoputWorkspace for the algorithm to work",""));
  }
 // Number of spectra
  const size_t nHist = inputWS->getNumberHistograms();

  // try to load par file if one is availible
  std::string fileName = this->getProperty("ParFile");
  if(!(fileName.empty()||fileName=="not_used.par")){
     if(!Poco::File(fileName).exists()){
         g_log.error()<<" FindDetectorsPar: attempting to load par file: "<<fileName<<" but it does not exist\n";
         throw(Kernel::Exception::FileError(" file not exist",fileName));
     }
     size_t nPars = loadParFile(fileName);
     if(nPars == nHist){
          this->populate_values_from_file(inputWS);
          this->set_output_table();
          return;
     }else{
          g_log.warning()<<" number of parameters in the file: "<<fileName<<"  not equal to the number of histograms in the workspace"
                              << inputWS->getName()<<std::endl;
          g_log.warning()<<" calculating detector parameters algorithmically\n";
     }
  
  }
  return_linear_ranges = this->getProperty("ReturnLinearRanges");
  
   
  // Get a pointer to the sample
   Geometry::IObjComponent_const_sptr sample =inputWS->getInstrument()->getSample();


  azimuthal.assign(nHist,std::numeric_limits<double>::quiet_NaN());
  polar.assign(nHist,std::numeric_limits<double>::quiet_NaN());
  azimuthal_width.assign(nHist,std::numeric_limits<double>::quiet_NaN());
  polar_width.assign(nHist,std::numeric_limits<double>::quiet_NaN());
  secondary_flightpath.assign(nHist,std::numeric_limits<double>::quiet_NaN());
  det_ID.assign(nHist,std::numeric_limits<size_t>::quiet_NaN());
  this->nDetectors = 0;

  Progress progress(this,0,1,100);
   const int progStep = (int)(ceil(double(nHist)/100.0));

   // Loop over the spectra
   size_t ic(0);
   for (size_t i = 0; i < nHist; i++)
   {
     Geometry::IDetector_const_sptr spDet;
     try{
        spDet= inputWS->getDetector(i);
     }catch(Kernel::Exception::NotFoundError &){
        continue;
     }
 
    // Check that we aren't writing a monitor...
    if (spDet->isMonitor())continue;   
     det_ID[ic] = spDet->getID();

     Kernel::V3D groupCentre;  
     Geometry::det_topology group_shape= spDet->getTopology(groupCentre);
     if(group_shape == Geometry::cyl){  // we have a ring;
            calc_cylDetPar(spDet,sample,groupCentre,azimuthal[ic], polar[ic], 
                           azimuthal_width[ic], polar_width[ic],secondary_flightpath[ic]);
     }else{  // we have a detector or a rectangular shape
           calc_rectDetPar(inputWS,spDet,sample,groupCentre,azimuthal[ic],polar[ic],
                           azimuthal_width[ic],polar_width[ic],secondary_flightpath[ic]);
     }
     ic++ ;
     // make regular progress reports and check for canceling the algorithm
     if ( i % progStep == 0 ){
            progress.report();
     }

   }
   nDetectors = ic;
   this->set_output_table();
   

}
// Constant for converting Radians to Degrees
const double rad2deg = 180.0 / M_PI;
// functions defines the ouptput table with parameters 
void 
FindDetectorsPar::set_output_table()
{
  std::string output = getProperty("OutputParTable");
  if(output.empty())return;
    // Store the result in a table workspace
   try{
      declareProperty(new WorkspaceProperty<API::ITableWorkspace>("OutputParTableWS","",Direction::Output));
   }catch(std::exception &err){
         g_log.information()<<" findDetecotorsPar: unsuccessfully declaring property: OutputParTableWS\n";
         g_log.information()<<" findDetecotorsPar: the reason is: "<<err.what()<<std::endl;
   }

     // Set the name of the new workspace
    setPropertyValue("OutputParTableWS",output);

     Mantid::API::ITableWorkspace_sptr m_result = Mantid::API::WorkspaceFactory::Instance().createTable("TableWorkspace");
     m_result->addColumn("double","twoTheta");
     m_result->addColumn("double","azimuthal");
     m_result->addColumn("double","secondary_flightpath");
     if(return_linear_ranges){
        m_result->addColumn("double","det_width");
        m_result->addColumn("double","det_height");
     }else{
        m_result->addColumn("double","polar_width");
        m_result->addColumn("double","azimuthal_width");
     }
     m_result->addColumn("long64","detID");
 
     for(size_t i=0;i<nDetectors;i++){
         Mantid::API::TableRow row = m_result->appendRow();
         row << polar[i] << azimuthal[i] << secondary_flightpath[i] << polar_width[i] << azimuthal_width[i] << (int64_t)det_ID[i];
     }
     setProperty("OutputParTableWS",m_result);
     API::AnalysisDataService::Instance().addOrReplace(output,m_result);

}

void 
FindDetectorsPar::calc_cylDetPar(const Geometry::IDetector_const_sptr spDet,const Geometry::IObjComponent_const_sptr sample,
                                 const Kernel::V3D &GroupCenter,
                                 double &azim, double &polar, double &azim_width, double &polar_width,double &dist)
{
        // polar values are constants for ring;
        azim_width= 0;
        azim      = 0;

        // accumulators;
        double d1_min(FLT_MAX);
        double d1_max(-FLT_MAX);
        double d1_sum(0);
        double dist_sum(0);
       

        std::vector<Kernel::V3D> coord(3);

        // get vector leading from the sample to the ring centre 
        Kernel::V3D Observer      = sample->getPos();
        coord[1]  = (GroupCenter-Observer);
        double d0 = coord[1].norm();
        coord[1] /= d0;
        // access contributed detectors;
        Geometry::DetectorGroup_const_sptr pDetGroup = boost::dynamic_pointer_cast<const Geometry::DetectorGroup>(spDet);
        if(!pDetGroup){
             g_log.error()<<"calc_cylDetPar: can not downcast IDetector_sptr to detector group for det->ID: "<<spDet->getID()<<std::endl;
             throw(std::bad_cast());
        }
        std::vector<Geometry::IDetector_const_sptr> pDets = pDetGroup->getDetectors();
        Geometry::BoundingBox bbox;

        // loop through all detectors in the group 
        for(size_t i=0;i<pDets.size();i++){
            Kernel::V3D center= pDets[i]->getPos();
            coord[0]  = center-GroupCenter;
            double d1 = coord[0].norm();
            coord[0] /= d1;
            coord[2]  = coord[0].cross_prod(coord[1]);

             // obtain the bounding box, aligned accordingly to the coordinates;
            bbox.nullify();
            bbox.setBoxAlignment(center,coord);
            pDets[i]->getBoundingBox(bbox);

            double d_min = d1+bbox.xMin();  if(d_min<d1_min)d1_min = d_min;
            double d_max = d1+bbox.xMax();  if(d_max>d1_max)d1_max = d_max;
            double d_azim = (bbox.zMax()-bbox.zMin())/d1;
            azim_width+=d_azim;

            d1_sum   +=d1;
            dist_sum +=d1*d1+d0*d0;
        }
        double dNdet= double(pDets.size());
        dist        = sqrt(dist_sum/dNdet);
    if(return_linear_ranges){
        polar_width  = d1_max-d1_min;          // the width of the detector's ring;
        azim_width   = 2*M_PI*(d1_sum/dNdet);  // the length of the detector's ring
    }else{

        polar_width = (atan2(d1_max,d0)-atan2(d1_min,d0))*rad2deg;
        polar       = atan2(d1_sum/dNdet,d0)*rad2deg;
     
        azim_width *= rad2deg;
    }        
}

void 
FindDetectorsPar::calc_rectDetPar(const API::MatrixWorkspace_sptr inputWS, 
                                 const Geometry::IDetector_const_sptr spDet,const Geometry::IObjComponent_const_sptr sample,
                                 const Kernel::V3D &GroupCentre,
                                 double &azim, double &polar, double &azim_width, double &polar_width,double &dist)
{
    // Get Sample->Detector distance
     dist     =  spDet->getDistance(*sample);
     polar    =  inputWS->detectorTwoTheta(spDet)*rad2deg;
     azim     =  spDet->getPhi()*rad2deg;    
    // Now let's work out the detector widths on basis of bounding box tangential to the 2Theta=const ring;
     Kernel::V3D beamDetVector(GroupCentre.X(),GroupCentre.Y(),0);  // group centre minus the projection of this centre to the beamline
     beamDetVector.normalize();
     std::vector<Kernel::V3D> coord(3);
     coord[0]  = beamDetVector;
     coord[1]  = Kernel::V3D(0,0,1); // along beamline, which is always oz; (can be amended)
     coord[2]  = coord[0].cross_prod(coord[1]);  // tangential to the ring and anticloakwise;
  

    // Get the bounding box
    Geometry::BoundingBox bbox;
    bbox.setBoxAlignment(GroupCentre,coord);

    spDet->getBoundingBox(bbox);
    double xsize = bbox.xMax() - bbox.xMin();
    double ysize = bbox.zMax() - bbox.zMin(); // bounding box has been rotated according to coord above, so z is along coord[2]

    if(return_linear_ranges){
        polar_width  = xsize;  // width
        azim_width   = ysize;  // height
    }else{
        polar_width  = 2*rad2deg*atan2((xsize/2.0), dist);
        azim_width   = 2*rad2deg*atan2((ysize/2.0), dist);
    }
}

//
size_t 
FindDetectorsPar::loadParFile(const std::string &fileName){
    // load ASCII par or phx file
    std::ifstream dataStream;
    std::vector<double> result;
    this->current_ASCII_file = get_ASCII_header(fileName,dataStream);
    load_plain(dataStream,result,current_ASCII_file);

    this->nDetectors = current_ASCII_file.nData_records;

    dataStream.close();
    // transfer par data into internal algorithm parameters;
    azimuthal.resize(nDetectors);
    polar.resize(nDetectors);
    det_ID.resize(nDetectors);

    int Block_size,shift;
    
    if(current_ASCII_file.Type==PAR_type){
        Block_size  = 5; // this value coinside with the value defined in load_plain
        shift       = 0;
        width.resize(nDetectors);
        height.resize(nDetectors);
        secondary_flightpath.resize(nDetectors,std::numeric_limits<double>::quiet_NaN());
   
        for(size_t i=0;i<nDetectors;i++){
           azimuthal[i]            =result[shift+2+i*Block_size];
           polar[i]                =result[shift+1+i*Block_size];
           width[i]                =result[shift+3+i*Block_size];
           height[i]               =result[shift+4+i*Block_size]; 
           secondary_flightpath[i] =result[shift+0+i*Block_size];
           det_ID[i]               = i+1;
        }

    }else if(current_ASCII_file.Type==PHX_type){
         Block_size = 6; // this value coinside with the value defined in load_plain
         shift      = 1;
         azimuthal_width.resize(nDetectors);
         polar_width.resize(nDetectors);
         for(size_t i=0;i<nDetectors;i++){
             azimuthal[i]       =result[shift+2+i*Block_size];
             polar[i]           =result[shift+1+i*Block_size];
             azimuthal_width[i] =result[shift+4+i*Block_size];
             polar_width[i]     =result[shift+3+i*Block_size]; 
             det_ID[i]               = i+1;
         }
    }else{
        g_log.error()<<" unsupported type of ASCII parameter file: "<<fileName<<std::endl;
        throw(std::invalid_argument("unsupported ASCII file type"));
    }

    return nDetectors;
}
// 
void 
FindDetectorsPar::populate_values_from_file(const API::MatrixWorkspace_sptr & inputWS)
{
    size_t nHist = inputWS->getNumberHistograms();

    if(this->current_ASCII_file.Type == PAR_type){
        // in this case data in azimuthal width and polar width are in fact real sizes in meters; have to transform it in into angular values
        azimuthal_width.resize(nHist);
        polar_width.resize(nHist);
        for (size_t i = 0; i < nHist; i++){
            if((azimuthal[i]>-45&&azimuthal[i]<45)||(azimuthal[i]>135)||(azimuthal[i]<-135)){
                azimuthal_width[i]=atan2(height[i],secondary_flightpath[i])*rad2deg;
                polar_width[i]    =atan2(width[i],secondary_flightpath[i])*rad2deg;
            }else{
                azimuthal_width[i]=atan2(width[i],secondary_flightpath[i])*rad2deg;
                polar_width[i]    =atan2(height[i],secondary_flightpath[i])*rad2deg;
            }
        }
        height.resize(0);
        width.resize(0);
    }else{

       Geometry::IObjComponent_const_sptr sample =inputWS->getInstrument()->getSample();
       secondary_flightpath.resize(nHist);
     // Loop over the spectra
     for (size_t i = 0; i < nHist; i++){
            Geometry::IDetector_const_sptr spDet;
            try{
                spDet= inputWS->getDetector(i);
            }catch(Kernel::Exception::NotFoundError &){
                continue;
            }
        // Check that we aren't writing a monitor...
        if (spDet->isMonitor())continue;
        /// this is the only value, which is not defined in phx file, so we calculate it   
           secondary_flightpath[i]     =  spDet->getDistance(*sample);
     }

    }
}
//
int 
FindDetectorsPar::count_changes(const char *const Buf,size_t buf_size)
{
    bool is_symbol(false),is_space(true);
    int  space_to_symbol_change(0),symbol_to_space_change(0);
    size_t symbols_start(0);
    // supress leading spaces;
    for(size_t i=0;i<buf_size;i++){
       if(Buf[i]==0)break;
       if(Buf[i]==' '){
            continue;
       }else{
          symbols_start=i;
          break;
       }
    }
    // calculate number of changes from space to symbol assuming start from symbol;
    for(size_t i=symbols_start;i<buf_size;i++){
        if(Buf[i]==0)break;
        if(Buf[i]>='+'&&Buf[i]<='z'){  // this is a symbol
            if(is_space){
                is_space=false;
                space_to_symbol_change++;
            }
            is_symbol=true;
        }
        if(Buf[i]==' '){  // this is a space
            if(is_symbol){
                is_symbol=false;
                symbol_to_space_change++;
            }
            is_space =true;
        }
    }
    return space_to_symbol_change;
}

/**! The function reads line from inout stream and puts it into buffer. 
*   It behaves like std::ifstream getline but the getline reads additional symbol from a row in a Unix-formatted file under windows;
*/
size_t
FindDetectorsPar::get_my_line(std::ifstream &in, char *buf, size_t buf_size,const char DELIM)
{
    size_t i;
    for(i=0;i<buf_size;i++){
        in.get(buf[i]);
        if(buf[i]==DELIM){	
            buf[i]=0;
            return i;
        }
    }
    buf[buf_size-1]=0;
    g_log.information()<<" data obtained from ASCII data file trunkated to "<<buf_size<<" characters\n";
    return buf_size;
}
/**!
 *  The function loads ASCII file header and tries to identify the type of the header.
 *  Possible types are
 *  SPE, PAR or PHS
 *
 *  if none three above identified, returns "undefined" type
 *  it also returns the FileTypeDescriptor, which identifyes the position of the data in correcponding ASCII file 
 *  plus characteristics of the data extracted from correspondent data header. 
*/
FileTypeDescriptor 
FindDetectorsPar::get_ASCII_header(std::string const &fileName, std::ifstream &data_stream)
{
    std::vector<char> BUF(1024);
    FileTypeDescriptor file_descriptor;
    file_descriptor.Type = NumFileTypes; // set the autotype to invalid

    data_stream.open(fileName.c_str(),std::ios_base::in|std::ios_base::binary);
    if(!data_stream.is_open()){
        g_log.error()<<" can not open existing ASCII data file: "<<fileName<<std::endl;
        throw(Kernel::Exception::FileError(" Can not open existing input data file",fileName));
    }
    // let's identify the EOL symbol; As the file may have been prepared on different OS, from where you are reading it 
    // and no conversion have been performed; 
    char symbol;
    data_stream.get(symbol);
    while(symbol>0x1F){
        data_stream.get(symbol);
    }
    char EOL;
    if(symbol==0x0D){ // Win or old Mac file
            data_stream.get(symbol);
            if(symbol==0x0A){ // Windows file
                EOL=0x0A;
            }else{            // Mac
                EOL=0x0D;
                data_stream.putback(symbol);
            }
    }else if(symbol==0x0A){   // unix file. 
        EOL=0x0A;
    }else{
        g_log.error()<<" Error reading the first row of the input ASCII data file: "<<fileName<<" as it contains unprintable characters\n";
        throw(Kernel::Exception::FileError(" Error reading the first row of the input ASCII data file, as it contains unprintable characters",fileName));
    }

    file_descriptor.line_end=EOL;
    data_stream.seekg(0,std::ios::beg);


    get_my_line(data_stream,&BUF[0],BUF.size(),EOL);
    if(!data_stream.good()){  
        g_log.error()<<" Error reading the first row of the input data file "<<fileName<<", It may be bigger then 1024 symbols\n";
        throw(Kernel::Exception::FileError(" Error reading the first row of the input data file, It may be bigger then 1024 symbols",fileName));
    }

    //let's find if there is one or more groups of symbols inside of the buffer;
    int space_to_symbol_change=count_changes(&BUF[0],BUF.size());
    if(space_to_symbol_change>1){  // more then one group of symbols in the string, spe file
        int nData_records(0),nData_blocks(0);
        // cppcheck-suppress invalidscanf
        int nDatas = sscanf(&BUF[0]," %d %d ",&nData_records,&nData_blocks);
        file_descriptor.nData_records = (size_t)nData_records;
        file_descriptor.nData_blocks  = (size_t)nData_blocks;
        if(nDatas!=2){    			
            g_log.error()<<" File "<<fileName<<" iterpreted as SPE but does not have two numbers in the first row\n";
            throw(Kernel::Exception::FileError(" File iterpreted as SPE but does not have two numbers in the first row",fileName));
        }
        file_descriptor.Type=SPE_type;
        get_my_line(data_stream,&BUF[0],BUF.size(),EOL);
        if(BUF[0]!='#'){ 
            g_log.error()<<" File "<<fileName<<"iterpreted as SPE does not have symbol # in the second row\n";
            throw(Kernel::Exception::FileError(" File iterpreted as SPE does not have symbol # in the second row",fileName));
        }
        file_descriptor.data_start_position = data_stream.tellg(); // if it is SPE file then the data begin after the second line;
    }else{
        file_descriptor.data_start_position = data_stream.tellg(); // if it is PHX or PAR file then the data begin after the first line;
        file_descriptor.nData_records       = atoi(&BUF[0]);
        file_descriptor.nData_blocks        = 0;

        // let's ifendify now if is PHX or PAR file;
        data_stream.getline(&BUF[0],BUF.size(),EOL);

        int space_to_symbol_change=count_changes(&BUF[0],BUF.size());
        if(space_to_symbol_change==6||space_to_symbol_change==5){       // PAR file
                file_descriptor.Type         = PAR_type;
                file_descriptor.nData_blocks = space_to_symbol_change;
        }else if(space_to_symbol_change==7){ // PHX file
                file_descriptor.Type=PHX_type;
                file_descriptor.nData_blocks = space_to_symbol_change;
        }else{   // something unclear or damaged
            g_log.error()<<" can not identify format of the input data file "<<fileName<<std::endl;
            throw(Kernel::Exception::FileError(" can not identify format of the input data file",fileName));
        }

    }
    return file_descriptor;
}

/*!
 *  function to load PHX or PAR file
 *  the file should be already opened and the FILE_TYPE structure properly defined using
 *  get_ASCII_header function
*/
static std::vector<char> BUF(1024,0);
void 
FindDetectorsPar::load_plain(std::ifstream &stream,std::vector<double> &Data,FileTypeDescriptor const &FILE_TYPE)
{

    char par_format[]=" %g %g %g %g %g";
    char phx_format[]=" %g %g %g %g %g %g";
    float data_buf[7];
    char *format;
    int BlockSize;
    char EOL = FILE_TYPE.line_end;

    switch(FILE_TYPE.Type){
        case(PAR_type):{
            format = par_format;
            BlockSize=5;
            break;
                        }
        case(PHX_type):{
            format = phx_format;
            BlockSize=6;
            break;
                        }
        default:		{
            g_log.error()<< " trying to load data in FindDetectorsPar::load_plain but the data type is not recognized\n";
            throw(std::invalid_argument(" trying to load data but the data type is not recognized"));
                        }
    }
    Data.resize(BlockSize*FILE_TYPE.nData_records);

    stream.seekg(FILE_TYPE.data_start_position,std::ios_base::beg);
    if(!stream.good()){		
        g_log.error()<<" can not rewind the file to the initial position where the data begin\n";
        throw(std::invalid_argument(" can not rewind the file to the initial position where the data begin"));
    }

    int nRead_Data(0);
    for(unsigned int i=0;i<FILE_TYPE.nData_records;i++){
        stream.getline(&BUF[0],BUF.size(),EOL);
        if(!stream.good()){	
            g_log.error()<<" error reading input file\n";
            throw(std::invalid_argument(" error reading input file"));
        }

        switch(FILE_TYPE.Type){
            case(PAR_type):{
                nRead_Data= sscanf(&BUF[0],format,data_buf,data_buf+1,data_buf+2,data_buf+3,data_buf+4);
                break;
                            }
            case(PHX_type):{
                nRead_Data= sscanf(&BUF[0],format,data_buf,data_buf+1,data_buf+2,data_buf+3,data_buf+4,data_buf+5);
                break;
                            }
            default:{
                 g_log.error()<<" unsupported value of FILE_TYPE.Type: "<<FILE_TYPE.Type<<std::endl;
            throw(std::invalid_argument(" unsupported value of FILE_TYPE.Type"));

                    }
        }
        if(nRead_Data!=BlockSize){
            g_log.error()<<" Error reading data at file, row "<<i+1<<" column "<<nRead_Data<<" from total "<<FILE_TYPE.nData_records<<" rows, "<<BlockSize<<" columns\n";
            throw(std::invalid_argument("error while interpreting data "));

        }
        for(int j=0;j<nRead_Data;j++){
            Data[i*BlockSize+j]=(double)data_buf[j];
        }

    }
}

}// end DataHandling namespace
}// end MantidNamespace

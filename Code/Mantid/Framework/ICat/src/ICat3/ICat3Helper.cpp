// WorkspaceFactory include must be first otherwise you get a bizarre Poco-related compilation error on Windows
#include "MantidAPI/WorkspaceFactory.h"
#if GCC_VERSION >= 40800 // 4.8.0
    GCC_DIAG_OFF(literal-suffix)
#endif
#include "MantidICat/ICat3/ICat3Helper.h"
#include "MantidICat/Session.h"
#include "MantidICat/ICat3/ICat3ErrorHandling.h"
#include <iomanip>
#include <time.h>
#include <boost/lexical_cast.hpp>

namespace Mantid
{
  namespace ICat
  {
    using namespace Kernel;
    using namespace API;
    using namespace ICat3;

    /* This method calls ICat API searchbydavanced and do the basic run search
     * @param icat :: Proxy object for ICat
     * @param request :: request object
     * @param response :: response object
     */
    int CICatHelper::doSearch(ICATPortBindingProxy& icat,boost::shared_ptr<ns1__searchByAdvanced>& request,ns1__searchByAdvancedResponse& response)
    {
      setICATProxySettings(icat);

      clock_t start=clock();
      int ret_advsearch=icat.searchByAdvanced(request.get(),&response);
      if(ret_advsearch!=0)
      {

        CErrorHandling::throwErrorMessages(icat);
      }
      clock_t end=clock();
      float diff = float(end -start)/CLOCKS_PER_SEC;
      g_log.information()<<" Time taken to do  search is "<<diff<< "  seconds "<<std::endl;
      return ret_advsearch;
    }

    /** This method saves the search response( investigations )data to a table workspace
     *  @param response :: const reference to response object
     *  @param outputws :: shared pointer to output workspace
     */
    void  CICatHelper::saveSearchRessults(const ns1__searchByAdvancedPaginationResponse& response,API::ITableWorkspace_sptr& outputws)
    {
      //create table workspace

      //API::ITableWorkspace_sptr outputws =createTableWorkspace();

      outputws->addColumn("long64","InvestigationId");
      outputws->addColumn("str","Proposal");
      outputws->addColumn("str","Title");
      outputws->addColumn("str","Instrument");
      outputws->addColumn("str","Run Range");

      try
      {
        saveInvestigations(response.return_,outputws);
      }
      catch(std::runtime_error& )
      {
        throw std::runtime_error("Error when saving  the ICat Search Results data to Workspace");
      }

    }
    /** This method saves investigations  to a table workspace
     *  @param investigations :: a vector containing investigation data
     *  @param outputws :: shared pointer to output workspace
     */
    void CICatHelper::saveInvestigations(const std::vector<ns1__investigation*>& investigations,API::ITableWorkspace_sptr& outputws)
    {

      try
      {
        std::vector<ns1__investigation*>::const_iterator citr;
        for (citr=investigations.begin();citr!=investigations.end();++citr)
        {
          API::TableRow t = outputws->appendRow();

          //investigation id
          savetoTableWorkspace((*citr)->id,t);

          //Proposal
          savetoTableWorkspace((*citr)->invNumber,t);

          //title
          savetoTableWorkspace((*citr)->title,t);

          //Instrument
          savetoTableWorkspace((*citr)->instrument,t);

          //run range
          savetoTableWorkspace((*citr)->invParamValue,t);
        }
      }
      catch(std::runtime_error& )
      {
        throw std::runtime_error("Error when saving  the ICat Search Results data to Workspace");
      }
    }

    /** This method saves investigations  to a table workspace
     *  @param investigation :: pointer to a single investigation data
     *  @param t :: reference to a row in a table workspace
     */
    void CICatHelper::saveInvestigatorsNameandSample(ns1__investigation* investigation,API::TableRow& t)
    {

      try
      {

        //   abstract
        savetoTableWorkspace(investigation->invAbstract,t);

        std::vector<ns1__investigator*>investigators;
        investigators.assign(investigation->investigatorCollection.begin(),investigation->investigatorCollection.end());

        std::string fullname; std::string* facilityUser=NULL;
        //for loop for getting invetigator's first and last name
        std::vector<ns1__investigator*>::const_iterator invstrItr;
        for(invstrItr=investigators.begin();invstrItr!=investigators.end();++invstrItr)
        {
          std::string firstname;std::string lastname;std::string name;
          if((*invstrItr)->ns1__facilityUser_)
          {

            if((*invstrItr)->ns1__facilityUser_->firstName)
            {
              firstname = *(*invstrItr)->ns1__facilityUser_->firstName;
            }
            if((*invstrItr)->ns1__facilityUser_->lastName)
            {
              lastname = *(*invstrItr)->ns1__facilityUser_->lastName;
            }
            name = firstname+" "+ lastname;
          }
          if(!fullname.empty())
          {
            fullname+=",";
          }
          fullname+=name;
        }//end of for loop for investigator's name.

        if(!fullname.empty())
        {
          facilityUser = new std::string;
          facilityUser->assign(fullname);
        }

        //invetigator name
        savetoTableWorkspace(facilityUser,t);

        std::vector<ns1__sample*>samples;
        std::string *samplenames =NULL;
        samples.assign(investigation->sampleCollection.begin(),investigation->sampleCollection.end());
        std::string sNames;
        //for loop for samples name.
        std::vector<ns1__sample*>::const_iterator sItr;
        for(sItr=samples.begin();sItr!=samples.end();++sItr)
        {
          std::string sName;
          if((*sItr)->name)
          {
            sName=*((*sItr)->name);
          }
          if(!sNames.empty())
          {
            sNames+=",";
          }
          sNames+=sName;
        }
        if(!sNames.empty())
        {
          samplenames = new std::string;
          samplenames->assign(sNames);
        }
        savetoTableWorkspace(samplenames,t);
      }
      catch(std::runtime_error& )
      {
        throw std::runtime_error("Error when saving  the ICat Search Results data to Workspace");
      }
    }

    /** This method loops through the response return_vector and saves the datafile details to a table workspace
     * @param response :: const reference to response object
     * @returns shared pointer to table workspace which stores the data
     */
    API::ITableWorkspace_sptr CICatHelper::saveFileSearchResponse(const ns1__searchByAdvancedResponse& response)
    {
      //create table workspace
      API::ITableWorkspace_sptr outputws =createTableWorkspace();
      //add columns
      outputws->addColumn("str","Name");
      outputws->addColumn("str","Location");
      outputws->addColumn("str","Create Time");
      outputws->addColumn("long64","Id");

      std::vector<ns1__investigation*> investVec;
      investVec.assign(response.return_.begin(),response.return_.end());

      try
      {
        std::vector<ns1__investigation*>::const_iterator inv_citr;
        for (inv_citr=investVec.begin();inv_citr!=investVec.end();++inv_citr)
        {
          std::vector<ns1__dataset*> datasetVec;
          datasetVec.assign((*inv_citr)->datasetCollection.begin(),(*inv_citr)->datasetCollection.end());

          std::vector<ns1__dataset*>::const_iterator dataset_citr;
          for(dataset_citr=datasetVec.begin();dataset_citr!=datasetVec.end();++dataset_citr)
          {
            std::vector<ns1__datafile * >datafileVec;
            datafileVec.assign((*dataset_citr)->datafileCollection.begin(),(*dataset_citr)->datafileCollection.end());

            std::vector<ns1__datafile * >::const_iterator datafile_citr;
            for(datafile_citr=datafileVec.begin();datafile_citr!=datafileVec.end();++datafile_citr)
            {

              API::TableRow t = outputws->appendRow();
              savetoTableWorkspace((*datafile_citr)->name,t);
              savetoTableWorkspace((*datafile_citr)->location,t);

              if((*datafile_citr)->datafileCreateTime!=NULL)
              {
                time_t  crtime=*(*datafile_citr)->datafileCreateTime;
                char temp [25];
                strftime (temp,25,"%Y-%b-%d %H:%M:%S",localtime(&crtime));
                std::string ftime(temp);
                std::string *creationtime=new std::string ;
                creationtime->assign(ftime);
                savetoTableWorkspace(creationtime,t);
                savetoTableWorkspace((*datafile_citr)->id,t);
              }

            }//end of for loop for data files iteration

          }//end of for loop for datasets iteration


        }// end of for loop investigations iteration.
      }
      catch(std::runtime_error& )
      {
        throw;
      }

      return outputws;
    }

    /**
     * This method sets the request parameters for the investigations includes
     * @param invstId :: investigation id
     * @param include :: enum parameter to retrieve dat from DB
     * @param request :: request object
     */
    void CICatHelper::setReqParamforInvestigationIncludes(long long invstId,ns1__investigationInclude include,ns1__getInvestigationIncludes& request)
    {
      //get the sessionid which is cached in session class during login
      *request.sessionId=Session::Instance().getSessionId();;
      *request.investigationInclude=include;
      *request.investigationId=invstId;

    }
    /**
     * This method calls ICat API getInvestigationIncludes and returns investigation details for a given investigation Id
     * @param invstId :: investigation id
     * @param include :: enum parameter for selecting the response data from the db.
     * @param responsews_sptr :: table workspace to save the response data
     * @returns zero if success otherwise error code
     */
    int CICatHelper::getDataFiles(long long invstId,ns1__investigationInclude include,
        API::ITableWorkspace_sptr& responsews_sptr)
    {
      //ICAt proxy object
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__getInvestigationIncludes request;
      //get the sessionid which is cached in session class during login
      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId=sessionId_sptr.get();

      // enum include
      boost::shared_ptr<ns1__investigationInclude>invstInculde_sptr(new ns1__investigationInclude);
      request.investigationInclude=invstInculde_sptr.get();

      request.investigationId=new LONG64;
      setReqParamforInvestigationIncludes(invstId,include,request);

      ns1__getInvestigationIncludesResponse response;
      int ret_advsearch=icat.getInvestigationIncludes(&request,&response);
      if(ret_advsearch!=0)
      {
        CErrorHandling::throwErrorMessages(icat);
      }
      std::stringstream stream;
      stream<<invstId;
      if(!response.return_)
      {
        g_log.information()<<"No data files exists in the ICat database for the selected investigation"<<std::endl;
        return -1;
      }
      try
      {
        saveInvestigationIncludesResponse(response,responsews_sptr);
      }
      catch(std::runtime_error &)
      {
        throw std::runtime_error("Error when selecting the investigation data with inestigation id "+ stream.str());
      }
      return ret_advsearch;
    }

    /**
     * This method loops through the response return_vector and saves the datafile details to a table workspace
     * @param response :: const reference to response object
     * @param outputws :: shared pointer to table workspace which stores the data
     */
    void  CICatHelper::saveInvestigationIncludesResponse(const ns1__getInvestigationIncludesResponse& response,
        API::ITableWorkspace_sptr& outputws)
    {

      outputws->addColumn("str","Name");
      outputws->addColumn("str","Location");
      outputws->addColumn("str","Create Time");
      outputws->addColumn("long64","Id");

      try
      {		std::vector<ns1__dataset*> datasetVec;
      datasetVec.assign((response.return_)->datasetCollection.begin(),(response.return_)->datasetCollection.end());
      if(datasetVec.empty())
      {
        throw std::runtime_error("No data files exists in the ICAT database for the selected investigation");
      }
      std::vector<ns1__dataset*>::const_iterator dataset_citr;
      for(dataset_citr=datasetVec.begin();dataset_citr!=datasetVec.end();++dataset_citr)
      {
        std::vector<ns1__datafile * >datafileVec;
        datafileVec.assign((*dataset_citr)->datafileCollection.begin(),(*dataset_citr)->datafileCollection.end());
        if(datafileVec.empty())
        {
          throw std::runtime_error("No data files exists in the ICAT database for the selected  investigation ");
        }

        std::vector<ns1__datafile * >::const_iterator datafile_citr;
        for(datafile_citr=datafileVec.begin();datafile_citr!=datafileVec.end();++datafile_citr)
        {

          API::TableRow t = outputws->appendRow();

          // File Name
          savetoTableWorkspace((*datafile_citr)->name,t);
          // File Size
          savetoTableWorkspace((*datafile_citr)->location,t);

          //File creation Time.
          std::string *creationtime=NULL;
          if((*datafile_citr)->datafileCreateTime!=NULL)
          {
            time_t  crtime=*(*datafile_citr)->datafileCreateTime;
            char temp [25];
            strftime (temp,25,"%Y-%b-%d %H:%M:%S",localtime(&crtime));
            std::string ftime(temp);
            creationtime=new std::string ;
            creationtime->assign(ftime);
          }
          savetoTableWorkspace(creationtime,t);
          // 
          savetoTableWorkspace((*datafile_citr)->id,t);
        }

      }

      }
      catch(std::runtime_error& )
      {
        throw ;
      }

    }

    /**This checks the datafile boolean  selected
     * @param fileName :: pointer to file name
     * @return bool - returns true if it's a raw file or nexus file
     */

    bool CICatHelper::isDataFile(const std::string* fileName)
    {
      if(!fileName)
      {
        return false;
      }
      std::basic_string <char>::size_type dotIndex;
      //find the position of '.' in raw/nexus file name
      dotIndex = (*fileName).find_last_of (".");
      std::string fextn=(*fileName).substr(dotIndex+1,(*fileName).size()-dotIndex);
      std::transform(fextn.begin(),fextn.end(),fextn.begin(),tolower);

      return((!fextn.compare("raw")|| !fextn.compare("nxs")) ? true :  false);
    }

    /**This method calls ICat API getInvestigationIncludes and returns datasets details for a given investigation Id
     * @param invstId :: investigation id
     * @param include :: enum parameter for selecting the response data from iact db.
     * @param responsews_sptr :: table workspace to save the response data
     * @returns an integer zero if success if not an error number.
     */
    int CICatHelper::doDataSetsSearch(long long invstId,ns1__investigationInclude include,
        API::ITableWorkspace_sptr& responsews_sptr)
    {
      //ICAt proxy object
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      // request object
      ns1__getInvestigationIncludes request;
      //get the sessionid which is cached in session class during login
      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId=sessionId_sptr.get();

      // enum include
      boost::shared_ptr<ns1__investigationInclude>invstInculde_sptr(new ns1__investigationInclude);
      request.investigationInclude=invstInculde_sptr.get();

      request.investigationId=new LONG64;
      setReqParamforInvestigationIncludes(invstId,include,request);

      //response object
      ns1__getInvestigationIncludesResponse response;
      // Calling Icat api
      int ret_advsearch=icat.getInvestigationIncludes(&request,&response);
      if(ret_advsearch!=0)
      {
        CErrorHandling::throwErrorMessages(icat);
      }
      std::stringstream stream;
      stream<<invstId;
      if(!(response.return_)|| (response.return_)->datasetCollection.empty())
      {
        g_log.information()<<"No datasets  exists in the ICat database for the inevstigation id "+ stream.str()<<std::endl;
        return -1 ;
      }
      try
      {
        saveDataSets(response,responsews_sptr);
      }
      catch(std::runtime_error &)
      {

        throw std::runtime_error("Error when loading the datasets for the investigation id "+ stream.str());
      }

      return ret_advsearch;
    }

    /** This method loops through the response return_vector and saves the datasets details to a table workspace
     * @param response :: const reference to response object
     * @param outputws ::  shred pointer to workspace
     * @returns shared pointer to table workspace which stores the data
     */
    void  CICatHelper::saveDataSets(const ns1__getInvestigationIncludesResponse& response,API::ITableWorkspace_sptr& outputws)
    {
      //create table workspace
      //adding columns
      outputws->addColumn("str","Name");//File name
      outputws->addColumn("str","Status");
      outputws->addColumn("str","Type");
      outputws->addColumn("str","Description");
      outputws->addColumn("long64","Sample Id");

      try
      {

        std::vector<ns1__dataset*> datasetVec;
        datasetVec.assign((response.return_)->datasetCollection.begin(),(response.return_)->datasetCollection.end());

        std::vector<ns1__dataset*>::const_iterator dataset_citr;
        for(dataset_citr=datasetVec.begin();dataset_citr!=datasetVec.end();++dataset_citr)
        {
          API::TableRow t = outputws->appendRow();

          // DataSet Name
          savetoTableWorkspace((*dataset_citr)->name,t);
          // DataSet Status
          savetoTableWorkspace((*dataset_citr)->datasetStatus,t);
          //DataSet Type
          savetoTableWorkspace((*dataset_citr)->datasetType,t);
          //DataSet Type
          savetoTableWorkspace((*dataset_citr)->description,t);
          //DataSet Type
          savetoTableWorkspace((*dataset_citr)->sampleId,t);
        }
      }

      catch(std::runtime_error& )
      {
        throw;
      }

      //return outputws;
    }

    /**This method calls ICat api listruments and returns the list of instruments a table workspace
     *@param instruments ::  list of instruments
     */
    void CICatHelper::listInstruments(std::vector<std::string>& instruments)
    {
      //ICAt proxy object
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__listInstruments request;
      //get the sessionid which is cached in session class during login
      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId=sessionId_sptr.get();
      //setting the request parameters
      setReqparamforlistInstruments(request);

      // response object
      ns1__listInstrumentsResponse response;

      int ret=icat.listInstruments(&request,&response);
      if(ret!=0)
      {
        //
        if(isvalidSession())
        {
          CErrorHandling::throwErrorMessages(icat);
        }
        else
        {
          throw SessionException("Please login to the information catalog using the login dialog provided.");
        }

      }
      if(response.return_.empty())
      {
        g_log.error()<<"Instruments List is empty"<<std::endl;
        return ;
      }

      instruments.assign(response.return_.begin(),response.return_.end());

    }

    /**This method sets the request parameter for ICat api list isnturments
     * @param request :: reference to request object
     */
    void CICatHelper::setReqparamforlistInstruments(ns1__listInstruments& request)
    {
      *request.sessionId=Session::Instance().getSessionId();
    }


    /**This method calls ICat api listruments and returns the list of instruments a table workspace
     *@param investTypes ::  list of investigationtypes
     */
    void  CICatHelper::listInvestigationTypes(std::vector<std::string>& investTypes)
    {
      //ICAt proxy object
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__listInvestigationTypes request;
      //get the sessionid which is cached in session class during login
      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId=sessionId_sptr.get();
      *request.sessionId=Session::Instance().getSessionId();

      // response object
      ns1__listInvestigationTypesResponse response;

      int ret=icat.listInvestigationTypes(&request,&response);
      if(ret!=0)
      {
        //
        if(isvalidSession())
        {
          CErrorHandling::throwErrorMessages(icat);
        }
        else
        {
          throw SessionException("Please login to the information catalog using the login dialog provided.");
        }
      }
      if(response.return_.empty())
      {
        g_log.information()<<"Investigation types is empty"<<std::endl;
        return ;
      }
      investTypes.assign(response.return_.begin(),response.return_.end());
    }


    /** This method creates table workspace
     * @returns the table workspace created
     */
    API::ITableWorkspace_sptr CICatHelper::createTableWorkspace()
    {
      //create table workspace
      API::ITableWorkspace_sptr outputws ;
      try
      {
        outputws=WorkspaceFactory::Instance().createTable("TableWorkspace");
      }
      catch(Mantid::Kernel::Exception::NotFoundError& )
      {
        throw std::runtime_error("Error when saving  the ICat Search Results data to Workspace");
      }
      catch(std::runtime_error &)
      {
        throw std::runtime_error("Error when saving  the ICat Search Results data to Workspace");
      }
      return outputws;
    }


    /** 
     *This method calls ICat api logout and disconnects from ICat DB
     * @returns zero if successful otherwise error code
     */
    int CICatHelper::doLogout()
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__logout request;
      ns1__logoutResponse response;
      boost::shared_ptr<std::string > sessionId_sptr(new std::string);
      *sessionId_sptr=Session::Instance().getSessionId();
      request.sessionId=sessionId_sptr.get();
      int ret=icat.logout(&request,&response);
      if(ret!=0)
      {
        throw std::runtime_error("You are not currently logged into the cataloging system.");
      }

      return ret;
    }

    /**
     * This method calls ICat api getmyinvestigations and do returns the investigations of the logged in user
     * @param ws_sptr :: shared pointer to table workspace which stores the investigations search result
     */
    void CICatHelper::doMyDataSearch(API::ITableWorkspace_sptr& ws_sptr)
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__getMyInvestigationsIncludes request;
      ns1__getMyInvestigationsIncludesResponse response;
      boost::shared_ptr<std::string > sessionId_sptr(new std::string);
      *sessionId_sptr=Session::Instance().getSessionId();
      request.sessionId=sessionId_sptr.get();
      // investigation include
      boost::shared_ptr<ns1__investigationInclude>invstInculde_sptr(new ns1__investigationInclude);
      request.investigationInclude=invstInculde_sptr.get();
      *request.investigationInclude = ns1__investigationInclude__INVESTIGATORS_USCORESHIFTS_USCOREAND_USCORESAMPLES;

      int ret=icat.getMyInvestigationsIncludes(&request,&response);
      if(ret!=0)
      {
        if(isvalidSession())
        {
          CErrorHandling::throwErrorMessages(icat);
        }
        else
        {
          throw SessionException("Please login to the information catalog using the login dialog provided.");
        }
      }
      if(response.return_.empty())
      {
        g_log.information()<<"ICat Mydata search is complete.There are no results to display"<<std::endl;
        return ;
      }
      //save response to a table workspace
      saveMyInvestigations(response,ws_sptr);

    }

    /**This method calls ICat api getmyinvestigations and do returns the investigations of the logged in user
     * @param response :: reference to response  object
     * @param outputws :: shared pointer to table workspace which stores the investigations search result
     */
    void CICatHelper::saveMyInvestigations( const ns1__getMyInvestigationsIncludesResponse& response,API::ITableWorkspace_sptr& outputws)
    {
      outputws->addColumn("long64","InvestigationId");
      outputws->addColumn("str","Proposal");
      outputws->addColumn("str","Title");
      outputws->addColumn("str","Instrument");
      outputws->addColumn("str","Run Range");

      saveInvestigations(response.return_,outputws);

    }


    /* This method does advanced search and returns investigation data
     * @param inputs :: reference to class containing search inputs
     * @param outputws :: shared pointer to output workspace
     * @param offset   :: skip this many rows and start returning rows from this point.
     * @param limit    :: limit the number of rows returned by the query.
     */
    void CICatHelper::doAdvancedSearch(const CatalogSearchParam& inputs,API::ITableWorkspace_sptr &outputws,
        const int &offset, const int &limit)
    {
      // Show "my data" (without paging).
      if (inputs.getMyData())
      {
        doMyDataSearch(outputws);
        return;
      }

      ns1__searchByAdvancedPagination request;
      ns1__searchByAdvancedPaginationResponse response;

      // If offset or limit is default value then we want to return as
      // we just wanted to perform the getSearchQuery to build our COUNT query.
      if (offset == -1 || limit == -1) return;

      std::string ses = Session::Instance().getSessionId();
      request.sessionId = &ses;
      // Setup paging information to search with paging enabled.
      request.numberOfResults = limit;
      request.startIndex      = offset;
      request.advancedSearchDetails = buildSearchQuery(inputs);

      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      int result = icat.searchByAdvancedPagination(&request, &response);

      if(result != 0)
      {
        //replace with mantid error routine
        CErrorHandling::throwErrorMessages(icat);
      }
      if(response.return_.empty())
      {
        g_log.information()<<"ICat investigations search is complete.There are no results to display"<<std::endl;
        return ;
      }
      //save response to a table workspace
      saveSearchRessults(response,outputws);
    }

    /**
     * Creates a search query based on search inputs provided by the user.
     * @param inputs :: Reference to a class containing search inputs.
     * Return A populated searchDetails class used for performing a query against ICAT.
     */
    ICat3::ns1__advancedSearchDetails* CICatHelper::buildSearchQuery(const CatalogSearchParam& inputs)
    {
      // As this is a member variable we need to reset the search terms once
      // a new search is performed.
      ICat3::ns1__advancedSearchDetails* advancedSearchDetails = new ICat3::ns1__advancedSearchDetails;

      ns1__investigationInclude invesInclude = ns1__investigationInclude__INVESTIGATORS_USCOREAND_USCOREKEYWORDS;
      advancedSearchDetails->investigationInclude = &invesInclude;

      double runStart, runEnd;
      //run start
      if(inputs.getRunStart() > 0)
      {
        runStart = inputs.getRunStart();
        advancedSearchDetails->runStart = &runStart;
      }

      //run end
      if(inputs.getRunEnd()>0)
      {
        runEnd = inputs.getRunEnd();
        advancedSearchDetails->runEnd = &runEnd;
      }

      time_t startDate, endDate;
      //start date
      if(inputs.getStartDate()!=0)
      {
        startDate = inputs.getStartDate();
        advancedSearchDetails->dateRangeStart = &startDate;
      }

      //end date
      if(inputs.getEndDate()!=0)
      {
        endDate = inputs.getEndDate();
        advancedSearchDetails->dateRangeEnd = &endDate;
      }

      //instrument name
      if(!inputs.getInstrument().empty())
      {
        advancedSearchDetails->instruments.push_back(inputs.getInstrument());
      }

      // keywords
      if(!inputs.getKeywords().empty())
      {
        advancedSearchDetails->keywords.push_back(inputs.getKeywords());
      }

      std::string investigationName, investigationType, datafileName, sampleName;

      //Investigation name
      if(!inputs.getInvestigationName().empty())
      {
        investigationName = inputs.getInvestigationName();
        advancedSearchDetails->investigationName = &investigationName;
      }

      //Investigation type
      if(!inputs.getInvestigationType().empty())
      {
        investigationType = inputs.getInvestigationType();
        advancedSearchDetails->investigationType = &investigationType;
      }

      //datafile name
      if(!inputs.getDatafileName().empty())
      {
        datafileName = inputs.getDatafileName();
        advancedSearchDetails->datafileName = &datafileName;
      }

      //sample name
      if(!inputs.getSampleName().empty())
      {
        sampleName = inputs.getSampleName();
        advancedSearchDetails->sampleName = &sampleName;
      }

      //investigator's surname
      if(!inputs.getInvestigatorSurName().empty())
      {
        advancedSearchDetails->investigators.push_back(inputs.getInvestigatorSurName());
      }

      return advancedSearchDetails;
    }

    /**
     * Uses user input fields to perform a search & obtain the COUNT of results for paging.
     * @return The number of investigations returned by the search performed.
     */
    int64_t CICatHelper::getNumberOfSearchResults(const CatalogSearchParam& inputs)
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__searchByAdvanced request;
      ns1__searchByAdvancedResponse response;

      std::string session = Session::Instance().getSessionId();
      request.sessionId = &session;
      request.advancedSearchDetails = buildSearchQuery(inputs);

      int result = icat.searchByAdvanced(&request, &response);

      int64_t numOfResults = 0;

      if (result == 0)
      {
        numOfResults = response.return_.size();
      }
      else
      {
        CErrorHandling::throwErrorMessages(icat);
      }

      g_log.debug() << "CICatHelper::getNumberOfSearchResults -> Number of results returned is: { " << numOfResults << " }" << std::endl;

      return numOfResults;
    }

    /**This method checks the given session is valid
     *@return returns true if the session id is valid,otherwise false;
     */
    bool CICatHelper::isvalidSession()
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__isSessionValid request;
      ns1__isSessionValidResponse response;
      std::string sessionId=Session::Instance().getSessionId();
      request.sessionId = &sessionId;

      return (response.return_? true: false);


    }

    /**This method uses ICat API login to connect to catalog
     *@param name :: login name of the user
     *@param password :: of the user
     *@param url :: endpoint url of the catalog
     */
    void CICatHelper::doLogin(const std::string& name,const std::string& password,const std::string & url)
    {
      // Store the soap end-point in the session for use later.
      ICat::Session::Instance().setSoapEndPoint(url);
      
      // Obtain the ICAT proxy that has been securely set, including soap-endpoint.
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      // Output the soap end-point in use for debugging purposes.
      g_log.debug() << "The ICAT soap end-point is: " << icat.soap_endpoint << "\n";

      // CatalogLogin to icat
      ns1__login login;
      ns1__loginResponse loginResponse;

      std::string userName(name);
      std::string passWord(password);
      login.username = &userName;
      login.password = &passWord;

      std::string session_id;

      int query_id = icat.login(&login, &loginResponse);
      if( query_id == 0 )
      {
        session_id = *(loginResponse.return_);
        //save session id
        ICat::Session::Instance().setSessionId(session_id);
        //save user name
        ICat::Session::Instance().setUserName(userName);
      }
      else
      {
        throw std::runtime_error("Username or password supplied is invalid.");
      }

    }

    void CICatHelper::getdownloadURL(const long long& fileId,std::string& url)
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);

      ns1__downloadDatafile request;

      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId = sessionId_sptr.get();
      *request.sessionId = Session::Instance().getSessionId();

      boost::shared_ptr<LONG64>fileId_sptr(new LONG64 );
      request.datafileId=fileId_sptr.get();
      *request.datafileId= fileId;

      //set request parameters

      ns1__downloadDatafileResponse response;
      // get the URL using ICAT API
      int ret=icat.downloadDatafile(&request,&response);
      if(ret!=0)
      {
        CErrorHandling::throwErrorMessages(icat);
      }
      if(!response.URL)
      {
        throw std::runtime_error("Empty URL returned from ICat3 Catalog");
      }
      url=*response.URL;

    }

    void CICatHelper::getlocationString(const long long& fileid,std::string& filelocation)
    {
      ICATPortBindingProxy icat;
      setICATProxySettings(icat);
      
      ns1__getDatafile request;

      boost::shared_ptr<std::string >sessionId_sptr(new std::string);
      request.sessionId=sessionId_sptr.get();
      *request.sessionId = Session::Instance().getSessionId();

      boost::shared_ptr<LONG64>fileId_sptr(new LONG64 );
      request.datafileId=fileId_sptr.get();
      *request.datafileId=fileid;

      ns1__getDatafileResponse response;
      int ret=icat.getDatafile(&request,&response);
      if(ret==0)
      {
        if(response.return_->location)
        {
          filelocation=*response.return_->location;
        }
      }
    }

    /**
     * Sets the soap-endpoint & SSL context for the given ICAT proxy.
     */
    void CICatHelper::setICATProxySettings(ICat3::ICATPortBindingProxy& icat)
    {
      // Set the soap-endpoint of the catalog we want to use.
      icat.soap_endpoint = ICat::Session::Instance().getSoapEndPoint().c_str();
      // Sets SSL authentication scheme
      setSSLContext(icat);
    }

    /**
     * Defines the SSL authentication scheme.
     * @param icat :: ICATPortBindingProxy object.
     */
    void CICatHelper::setSSLContext(ICat3::ICATPortBindingProxy& icat)
    {
      if (soap_ssl_client_context(&icat,
        SOAP_SSL_CLIENT, /* use SOAP_SSL_DEFAULT in production code */
        NULL,       /* keyfile: required only when client must authenticate to
                    server (see SSL docs on how to obtain this file) */
        NULL,       /* password to read the keyfile */
        NULL,      /* optional cacert file to store trusted certificates */
        NULL,      /* optional capath to directory with trusted certificates */
        NULL      /* if randfile!=NULL: use a file with random data to seed randomness */
        ))
      {
        CErrorHandling::throwErrorMessages(icat);
      }
    }
  }
}

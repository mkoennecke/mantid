#include <boost/shared_ptr.hpp>
#include "MantidMDEvents/MDEventFactory.h"
#include "MantidAPI/IMDEventWorkspace.h"
#include "MantidMDEvents/MDEventWorkspace.h"

#include "MantidMDEvents/MDBoxBase.h"
#include "MantidMDEvents/MDBox.h"
#include "MantidMDEvents/MDEventWorkspace.h"
#include "MantidMDEvents/MDGridBox.h"
#include "MantidMDEvents/MDBin.h"
#include "MantidMDEvents/MDBoxIterator.h"
#include "MantidMDEvents/MDEvent.h"
#include "MantidMDEvents/MDLeanEvent.h"

// We need to include the .cpp files so that the declarations are picked up correctly. Weird, I know. 
// See http://www.parashift.com/c++-faq-lite/templates.html#faq-35.13 
#include "MDBoxBase.cpp"
#include "MDBox.cpp"
#include "MDEventWorkspace.cpp"
#include "MDGridBox.cpp"
#include "MDBin.cpp"
#include "MDBoxIterator.cpp"



namespace Mantid
{
    namespace MDEvents
    {
        //### BEGIN AUTO-GENERATED CODE #################################################################
        /* Code below Auto-generated by 'generate_mdevent_declarations.py' 
         *     on 2013-05-30 12:36:57.329000
         *
         * DO NOT EDIT!
         */ 
        
        // Instantiations for MDEvent
         template DLLExport class MDEvent<1>;
         template DLLExport class MDEvent<2>;
         template DLLExport class MDEvent<3>;
         template DLLExport class MDEvent<4>;
         template DLLExport class MDEvent<5>;
         template DLLExport class MDEvent<6>;
         template DLLExport class MDEvent<7>;
         template DLLExport class MDEvent<8>;
         template DLLExport class MDEvent<9>;
        // Instantiations for MDLeanEvent
         template DLLExport class MDLeanEvent<1>;
         template DLLExport class MDLeanEvent<2>;
         template DLLExport class MDLeanEvent<3>;
         template DLLExport class MDLeanEvent<4>;
         template DLLExport class MDLeanEvent<5>;
         template DLLExport class MDLeanEvent<6>;
         template DLLExport class MDLeanEvent<7>;
         template DLLExport class MDLeanEvent<8>;
         template DLLExport class MDLeanEvent<9>;
        // Instantiations for MDBoxBase
         template DLLExport class MDBoxBase<MDEvent<1>, 1>;
         template DLLExport class MDBoxBase<MDEvent<2>, 2>;
         template DLLExport class MDBoxBase<MDEvent<3>, 3>;
         template DLLExport class MDBoxBase<MDEvent<4>, 4>;
         template DLLExport class MDBoxBase<MDEvent<5>, 5>;
         template DLLExport class MDBoxBase<MDEvent<6>, 6>;
         template DLLExport class MDBoxBase<MDEvent<7>, 7>;
         template DLLExport class MDBoxBase<MDEvent<8>, 8>;
         template DLLExport class MDBoxBase<MDEvent<9>, 9>;
         template DLLExport class MDBoxBase<MDLeanEvent<1>, 1>;
         template DLLExport class MDBoxBase<MDLeanEvent<2>, 2>;
         template DLLExport class MDBoxBase<MDLeanEvent<3>, 3>;
         template DLLExport class MDBoxBase<MDLeanEvent<4>, 4>;
         template DLLExport class MDBoxBase<MDLeanEvent<5>, 5>;
         template DLLExport class MDBoxBase<MDLeanEvent<6>, 6>;
         template DLLExport class MDBoxBase<MDLeanEvent<7>, 7>;
         template DLLExport class MDBoxBase<MDLeanEvent<8>, 8>;
         template DLLExport class MDBoxBase<MDLeanEvent<9>, 9>;

 
        // Instantiations for MDBox
         template DLLExport class MDBox<MDEvent<1>, 1>;
         template DLLExport class MDBox<MDEvent<2>, 2>;
         template DLLExport class MDBox<MDEvent<3>, 3>;
         template DLLExport class MDBox<MDEvent<4>, 4>;
         template DLLExport class MDBox<MDEvent<5>, 5>;
         template DLLExport class MDBox<MDEvent<6>, 6>;
         template DLLExport class MDBox<MDEvent<7>, 7>;
         template DLLExport class MDBox<MDEvent<8>, 8>;
         template DLLExport class MDBox<MDEvent<9>, 9>;
         template DLLExport class MDBox<MDLeanEvent<1>, 1>;
         template DLLExport class MDBox<MDLeanEvent<2>, 2>;
         template DLLExport class MDBox<MDLeanEvent<3>, 3>;
         template DLLExport class MDBox<MDLeanEvent<4>, 4>;
         template DLLExport class MDBox<MDLeanEvent<5>, 5>;
         template DLLExport class MDBox<MDLeanEvent<6>, 6>;
         template DLLExport class MDBox<MDLeanEvent<7>, 7>;
         template DLLExport class MDBox<MDLeanEvent<8>, 8>;
         template DLLExport class MDBox<MDLeanEvent<9>, 9>;

 
        // Instantiations for MDEventWorkspace
         template DLLExport class MDEventWorkspace<MDEvent<1>, 1>;
         template DLLExport class MDEventWorkspace<MDEvent<2>, 2>;
         template DLLExport class MDEventWorkspace<MDEvent<3>, 3>;
         template DLLExport class MDEventWorkspace<MDEvent<4>, 4>;
         template DLLExport class MDEventWorkspace<MDEvent<5>, 5>;
         template DLLExport class MDEventWorkspace<MDEvent<6>, 6>;
         template DLLExport class MDEventWorkspace<MDEvent<7>, 7>;
         template DLLExport class MDEventWorkspace<MDEvent<8>, 8>;
         template DLLExport class MDEventWorkspace<MDEvent<9>, 9>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<1>, 1>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<2>, 2>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<3>, 3>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<4>, 4>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<5>, 5>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<6>, 6>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<7>, 7>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<8>, 8>;
         template DLLExport class MDEventWorkspace<MDLeanEvent<9>, 9>;

 
        // Instantiations for MDGridBox
         template DLLExport class MDGridBox<MDEvent<1>, 1>;
         template DLLExport class MDGridBox<MDEvent<2>, 2>;
         template DLLExport class MDGridBox<MDEvent<3>, 3>;
         template DLLExport class MDGridBox<MDEvent<4>, 4>;
         template DLLExport class MDGridBox<MDEvent<5>, 5>;
         template DLLExport class MDGridBox<MDEvent<6>, 6>;
         template DLLExport class MDGridBox<MDEvent<7>, 7>;
         template DLLExport class MDGridBox<MDEvent<8>, 8>;
         template DLLExport class MDGridBox<MDEvent<9>, 9>;
         template DLLExport class MDGridBox<MDLeanEvent<1>, 1>;
         template DLLExport class MDGridBox<MDLeanEvent<2>, 2>;
         template DLLExport class MDGridBox<MDLeanEvent<3>, 3>;
         template DLLExport class MDGridBox<MDLeanEvent<4>, 4>;
         template DLLExport class MDGridBox<MDLeanEvent<5>, 5>;
         template DLLExport class MDGridBox<MDLeanEvent<6>, 6>;
         template DLLExport class MDGridBox<MDLeanEvent<7>, 7>;
         template DLLExport class MDGridBox<MDLeanEvent<8>, 8>;
         template DLLExport class MDGridBox<MDLeanEvent<9>, 9>;

 
        // Instantiations for MDBin
         template DLLExport class MDBin<MDEvent<1>, 1>;
         template DLLExport class MDBin<MDEvent<2>, 2>;
         template DLLExport class MDBin<MDEvent<3>, 3>;
         template DLLExport class MDBin<MDEvent<4>, 4>;
         template DLLExport class MDBin<MDEvent<5>, 5>;
         template DLLExport class MDBin<MDEvent<6>, 6>;
         template DLLExport class MDBin<MDEvent<7>, 7>;
         template DLLExport class MDBin<MDEvent<8>, 8>;
         template DLLExport class MDBin<MDEvent<9>, 9>;
         template DLLExport class MDBin<MDLeanEvent<1>, 1>;
         template DLLExport class MDBin<MDLeanEvent<2>, 2>;
         template DLLExport class MDBin<MDLeanEvent<3>, 3>;
         template DLLExport class MDBin<MDLeanEvent<4>, 4>;
         template DLLExport class MDBin<MDLeanEvent<5>, 5>;
         template DLLExport class MDBin<MDLeanEvent<6>, 6>;
         template DLLExport class MDBin<MDLeanEvent<7>, 7>;
         template DLLExport class MDBin<MDLeanEvent<8>, 8>;
         template DLLExport class MDBin<MDLeanEvent<9>, 9>;

 
        // Instantiations for MDBoxIterator
         template DLLExport class MDBoxIterator<MDEvent<1>, 1>;
         template DLLExport class MDBoxIterator<MDEvent<2>, 2>;
         template DLLExport class MDBoxIterator<MDEvent<3>, 3>;
         template DLLExport class MDBoxIterator<MDEvent<4>, 4>;
         template DLLExport class MDBoxIterator<MDEvent<5>, 5>;
         template DLLExport class MDBoxIterator<MDEvent<6>, 6>;
         template DLLExport class MDBoxIterator<MDEvent<7>, 7>;
         template DLLExport class MDBoxIterator<MDEvent<8>, 8>;
         template DLLExport class MDBoxIterator<MDEvent<9>, 9>;
         template DLLExport class MDBoxIterator<MDLeanEvent<1>, 1>;
         template DLLExport class MDBoxIterator<MDLeanEvent<2>, 2>;
         template DLLExport class MDBoxIterator<MDLeanEvent<3>, 3>;
         template DLLExport class MDBoxIterator<MDLeanEvent<4>, 4>;
         template DLLExport class MDBoxIterator<MDLeanEvent<5>, 5>;
         template DLLExport class MDBoxIterator<MDLeanEvent<6>, 6>;
         template DLLExport class MDBoxIterator<MDLeanEvent<7>, 7>;
         template DLLExport class MDBoxIterator<MDLeanEvent<8>, 8>;
         template DLLExport class MDBoxIterator<MDLeanEvent<9>, 9>;

 
        
        /* CODE ABOWE WAS AUTO-GENERATED BY generate_mdevent_declarations.py - DO NOT EDIT! */ 
        
        //### END AUTO-GENERATED CODE ##################################################################


        //------------------------------- FACTORY METHODS ------------------------------------------------------------------------------------------------------------------

        /** Create a MDEventWorkspace of the given type
        @param nd :: number of dimensions
        @param eventType :: string describing the event type (MDEvent or MDLeanEvent)
        @return shared pointer to the MDEventWorkspace created (as a IMDEventWorkspace).
        */
        API::IMDEventWorkspace_sptr MDEventFactory::CreateMDWorkspace(size_t nd, const std::string & eventType)
        {

            if(nd > MAX_MD_DIMENSIONS_NUM)
                throw std::invalid_argument(" there are more dimensions requested then instantiated");

            API::IMDEventWorkspace *pWs = (*(wsCreatorFP[nd]))(eventType);

            return boost::shared_ptr<API::IMDEventWorkspace >(pWs);
        }
        /** Create a MDBox or MDGridBoxof the given type
        @param nDimensions  :: number of dimensions
        @param Type         :: enum descibing the box (MDBox or MDGridBox) and the event type (MDEvent or MDLeanEvent)
        @param splitter     :: shared pointer to the box controller responsible for splitting boxes. The BC is not incremented as boxes take usual pointer from this pointer
        @param extentsVector:: box extents in all n-dimensions (min-max)
        @param depth        :: the depth of the box within the box tree
        @param nBoxEvents   :: if defined, specify the memory the box should allocate to accept events -- not used for MDGridBox
        @param boxID        :: the unique identifier, referencing location of the box in 1D linked list of boxes.  -- not used for MDGridBox

        @return pointer to the IMDNode with proper box created.
        */

        API::IMDNode * MDEventFactory::createBox(size_t nDimensions,MDEventFactory::BoxType Type, API::BoxController_sptr & splitter, 
            const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & extentsVector,
            const uint32_t depth,const size_t nBoxEvents,const size_t boxID)
        {

            if(nDimensions > MAX_MD_DIMENSIONS_NUM)
                throw std::invalid_argument(" there are more dimensions requested then instantiated");

            size_t id = nDimensions*MDEventFactory::NumBoxTypes + Type;
            if(extentsVector.size()!=nDimensions) // set defaults
            {
                std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > defaultExtents(nDimensions);
                for(size_t i=0;i<nDimensions;i++)
                {
										//set to smaller than float max, so the entire range fits in a float.
                    defaultExtents[i].setExtents(-1e30f,1e30f);
                }
                return (*(boxCreatorFP[id]))(splitter.get(),defaultExtents,depth,nBoxEvents,boxID);
            }


            return (*(boxCreatorFP[id]))(splitter.get(),extentsVector,depth,nBoxEvents,boxID);
        }

        //------------------------------- FACTORY METHODS END --------------------------------------------------------------------------------------------------------------

        /// static vector, conaining the pointers to the functions creating MD boxes
        //std::vector<MDEventFactory::fpCreateBox>  MDEventFactory::boxCreatorFP(MDEventFactory::NumBoxTypes*(MDEventFactory::MAX_MD_DIMENSIONS_NUM+1),NULL);
        std::vector<MDEventFactory::fpCreateBox>  MDEventFactory::boxCreatorFP(20);
        // static vector, conaining the pointers to the functions creating MD Workspaces
        //std::vector<MDEventFactory::fpCreateMDWS> MDEventFactory::wsCreatorFP(MDEventFactory::MAX_MD_DIMENSIONS_NUM+1,NULL);
      std::vector<MDEventFactory::fpCreateMDWS> MDEventFactory::wsCreatorFP(20);

        //########### Teplate methaprogrammed CODE SOURCE start:  -------------------------------------

        //-------------------------------------------------------------- MD Workspace constructor wrapper
        /** Template to create md workspace with specific number of dimentisons
         * @param eventType -- type of event (lean or full) to generate workspace for
        */
        template<size_t nd>
        API::IMDEventWorkspace * MDEventFactory::createMDWorkspaceND(const std::string & eventType)
        {
            if (eventType == "MDEvent")
                return new MDEventWorkspace<MDEvent<nd>,nd>;
            else if(eventType == "MDLeanEvent")
                return new MDEventWorkspace<MDLeanEvent<nd>,nd>;
            else
                throw std::invalid_argument("Unknown event type "+eventType+" passed to CreateMDWorkspace.");

        }
        /** Template to create md workspace with 0 number of dimentisons. As this is wrong, just throws. Used as terminator and check for the wrong dimensions number
         * @param eventType -- type of event (lean or full) to generate workspace for - -does not actually used. 
        */
        template<>
        API::IMDEventWorkspace * MDEventFactory::createMDWorkspaceND<0>(const std::string & /*eventType*/)
        {
            throw std::invalid_argument("Workspace can not have 0 dimensions");
        }
        //-------------------------------------------------------------- MD BOX constructor wrapper
        /** Method to create any MDBox type with 0 number of dimensions. As it is wrong, just throws */ 
        API::IMDNode * MDEventFactory::createMDBoxWrong(API::BoxController *,const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & ,const uint32_t ,const size_t ,const size_t )
        {
            throw std::invalid_argument("MDBox/MDGridBox can not have 0 dimensions");
        }
        /**Method to create MDBox for lean events (Constructor wrapper) with given number of dimensions
           * @param splitter :: BoxController that controls how boxes split
           * @param extentsVector :: vector defining the extents of the box in all n-dimensions
           * @param depth :: splitting depth of the new box.
           * @param nBoxEvents :: number of events to reserve memory for (if needed).
           * @param boxID :: id for the given box
        */ 
        template<size_t nd>
        API::IMDNode * MDEventFactory::createMDBoxLean(API::BoxController *splitter,const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & extentsVector,const uint32_t depth,const size_t nBoxEvents,const size_t boxID)
        {
            return new MDBox<MDLeanEvent<nd>,nd>(splitter,depth,extentsVector,nBoxEvents,boxID);
        }
        /**Method to create MDBox for events (Constructor wrapper) with given number of dimensions
           * @param splitter :: BoxController that controls how boxes split
           * @param extentsVector :: vector defining the extents of the box in all n-dimensions
           * @param depth :: splitting depth of the new box.
           * @param nBoxEvents :: number of events to reserve memory for (if needed).
           * @param boxID :: id for the given box
        */ 
        template<size_t nd>
        API::IMDNode * MDEventFactory::createMDBoxFat(API::BoxController *splitter,const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & extentsVector,const uint32_t depth,const size_t nBoxEvents,const size_t boxID)
        {
            return new MDBox<MDEvent<nd>,nd>(splitter,depth,extentsVector,nBoxEvents,boxID);
        }
        /**Method to create MDGridBox for lean events (Constructor wrapper) with given number of dimensions
           * @param splitter :: BoxController that controls how boxes split
           * @param extentsVector :: vector defining the extents of the box in all n-dimensions
           * @param depth :: splitting depth of the new box.
           * @param nBoxEvents  -- not used 
           * @param boxID ::   --- not used
        */ 
        template<size_t nd>
        API::IMDNode * MDEventFactory::createMDGridBoxLean(API::BoxController *splitter,const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & extentsVector,const uint32_t depth,
            const size_t /*nBoxEvents*/,const size_t /*boxID*/)
        {
            return new MDGridBox<MDLeanEvent<nd>,nd>(splitter,depth,extentsVector);
        }
        /**Method to create MDGridBox for events (Constructor wrapper) with given number of dimensions
           * @param splitter :: BoxController that controls how boxes split
           * @param extentsVector :: vector defining the extents of the box in all n-dimensions
           * @param depth :: splitting depth of the new box.
           * @param nBoxEvents  -- not used 
           * @param boxID ::   --- not used
        */ 
        template<size_t nd>
        API::IMDNode * MDEventFactory::createMDGridBoxFat(API::BoxController *splitter,const std::vector<Mantid::Geometry::MDDimensionExtents<coord_t> > & extentsVector,const uint32_t depth,
            const size_t /*nBoxEvents*/,const size_t /*boxID*/)
        {
            return new MDGridBox<MDEvent<nd>,nd>(splitter,depth,extentsVector);
        }
        //-------------------------------------------------------------- MD BOX constructor wrapper -- END


        // the class instantiated by compiler at compilation time and generates the map,  
        // between the number of dimensions and the function, which process this number of dimensions
        template<size_t nd>
        class LOOP
        {
        public:
            LOOP()
            {
                EXEC();
            }
            static inline void EXEC()
            {
                LOOP< nd-1 >::EXEC();
                MDEventFactory::wsCreatorFP[nd]      = &MDEventFactory::createMDWorkspaceND<nd>;

                MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes*nd+MDEventFactory::MDBoxWithLean]     = &MDEventFactory::createMDBoxLean<nd>;
                MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes*nd+MDEventFactory::MDBoxWithFat]      = &MDEventFactory::createMDBoxFat<nd>;
                MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes*nd+MDEventFactory::MDGridBoxWithLean] = &MDEventFactory::createMDGridBoxLean<nd>;
                MDEventFactory::boxCreatorFP[MDEventFactory::NumBoxTypes*nd+MDEventFactory::MDGridBoxWithFat]  = &MDEventFactory::createMDGridBoxFat<nd>;
            }
        };
        // the class terminates the compitlation-time metaloop and sets up functions which process 0-dimension workspace operations (throw invalid argument)
        template<>
        class LOOP<0>
        {
        public:
            LOOP(){}
            static inline void EXEC()
            {           
                MDEventFactory::wsCreatorFP[0]    = &MDEventFactory::createMDWorkspaceND<0>;

                MDEventFactory::boxCreatorFP[MDEventFactory::MDBoxWithLean]    = &MDEventFactory::createMDBoxWrong;
                MDEventFactory::boxCreatorFP[MDEventFactory::MDBoxWithFat]     = &MDEventFactory::createMDBoxWrong;
                MDEventFactory::boxCreatorFP[MDEventFactory::MDGridBoxWithLean]= &MDEventFactory::createMDBoxWrong;
                MDEventFactory::boxCreatorFP[MDEventFactory::MDGridBoxWithFat] = &MDEventFactory::createMDBoxWrong;
            }
        };
        //########### Teplate methaprogrammed CODE SOURCE END:  -------------------------------------
        //statically instantiate the code, defined by the class above and assocoate it with events factory
        LOOP<MDEventFactory::MAX_MD_DIMENSIONS_NUM> MDEventFactory::CODE_GENERATOR;

    } // namespace Mantid
} // namespace MDEvents 

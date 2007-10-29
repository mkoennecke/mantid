
#ifndef MANTID_SUPPORTTEST_H_
#define MANTID_SUPPORTTEST_H_

#include <cxxtest/TestSuite.h>

#include "../inc/Support.h"
#include <string>

using namespace Mantid::StrFunc;

/*!
  \class testSupport 
  \brief test of Support components
  \version 1.0
  \date September 2005
  \author S.Ansell
  
  Checks the basic string operations in
  support.cxx
*/
class SupportTest : public CxxTest::TestSuite
{
public: 

	void testExtractWord()
	  /*!
		Applies a test to the extractWord
		The object is to find a suitable lenght
		of a string in a group of words
		\retval -1 :: failed find word in string
		when the pattern exists.
	  */
	{
	  std::string Ln="Name wav wavelength other stuff";
	  int retVal=extractWord(Ln,"wavelengt",4);
	  TS_ASSERT_EQUALS(Ln, "Name wav  other stuff"); 
	}

	void testConvert()
	  /*!
		Applies a test to convert
	  */
	{
	  int i;
	  //valid double convert
	  TS_ASSERT_EQUALS(convert("   568   ",i), 1);
	  TS_ASSERT_EQUALS(i,568);
	  double X;
	  //valid double convert
	  TS_ASSERT_EQUALS(convert("   3.4   ",X), 1);
	  TS_ASSERT_EQUALS(X,3.4);
	  X=9.0;
	  //invalid leading stuff
	  TS_ASSERT_DIFFERS(convert("   e3.4   ",X),1);
	  //invalid trailing stuff
	  TS_ASSERT_DIFFERS(convert("   3.4g   ",X),1);
	  std::string Y;
	  TS_ASSERT_EQUALS(convert("   3.4y   ",Y),1);
	  TS_ASSERT_EQUALS(Y,"3.4y");
	}

	void testSection()
	  /*!
		Applies a test to section
	  */
	{
	  std::string Mline="V 1 tth ";
	  std::string Y;
	  TS_ASSERT_EQUALS(section(Mline,Y),1);
	  //I'm not usre what this is supposed to do!
	  //  std::cout<<"Mline =="<<Mline<<"=="<<std::endl;
	  //  std::cout<<"Y =="<<Y<<"=="<<std::endl;
	}

	void testSectPartNum()
	  /*!
		Applies a test to sectPartNum
	  */
	{
	  double X;
	  std::string NTest="   3.4   ";
	  TS_ASSERT_EQUALS(sectPartNum(NTest,X),1);
	  TS_ASSERT_EQUALS(X,3.4);
	  X=9.0;
	  NTest="   3.4g   ";
	  TS_ASSERT_EQUALS(sectPartNum(NTest,X),1);
	  TS_ASSERT_EQUALS(X,3.4);
	  X=9.0;
	  NTest="   e3.4   ";
	  TS_ASSERT_DIFFERS(sectPartNum(NTest,X),1);
	  TS_ASSERT_EQUALS(X,9.0);
	}

	void testStrParts()
	  /*!
		Applies a test to the StrParts function
	  */
	{ 
	  std::string Y=" $var s566>s4332 dxx";
	  std::vector<std::string> out=StrParts(Y);
	  TS_ASSERT_EQUALS(out.size(),3);
	  TS_ASSERT_EQUALS(out[0],"$var");
	  TS_ASSERT_EQUALS(out[1],"s566>s4332");
	  TS_ASSERT_EQUALS(out[2],"dxx");
	}

};

#endif //MANTID_SUPPORTTEST_H_

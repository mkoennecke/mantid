#include <iostream>
#include <sstream>
#include <cmath>
#include <vector>

#include "../inc/Support.h"

namespace Mantid
{
namespace  StrFunc
{

void 
printHex(std::ostream& OFS,const int n)
  /*!
    Function to convert and number into hex
    output (and leave the stream un-changed)
    \param OFS :: Output stream
    \param n :: integer to convert
    \todo Change this to a stream operator
  */
{
  std::ios_base::fmtflags PrevFlags=OFS.flags();
  OFS<<"Ox";
  OFS.width(8);
  OFS.fill('0');
  hex(OFS);
  OFS << n;
  OFS.flags(PrevFlags);
  return;
} 

int
extractWord(std::string& Line,const std::string& Word,const int cnt)
  /*!
    Checks that as least cnt letters of 
    works is part of the string. It is currently 
    case sensative. It removes the Word if found
    \param Line :: Line to process
    \param Word :: Word to use
    \param cnt :: Length of Word for significants [default =4]
    \retval 1 on success (and changed Line) 
    \retval 0 on failure 
  */
{
  if (Word.empty())
    return 0;

  unsigned int minSize(cnt>static_cast<int>(Word.size()) ?  Word.size() : cnt);
  std::string::size_type pos=Line.find(Word.substr(0,minSize));
  if (pos==std::string::npos)
    return 0;
  // Pos == Start of find
  unsigned int LinePt=minSize+pos;
  for(;minSize<Word.size() && LinePt<Line.size()
	&& Word[minSize]==Line[LinePt];LinePt++,minSize++);

  Line.erase(pos,LinePt-(pos-1));
  return 1;
}

int
confirmStr(const std::string& S,const std::string& fullPhrase)
  /*!
    Check to see if S is the same as the
    first part of a phrase. (case insensitive)
    \param S :: string to check
    \param fullPhrase :: complete phrase
    \returns 1 on success 
  */
{
  const int nS(S.length());
  const int nC(fullPhrase.length());
  if (nS>nC || nS<=0)    
    return 0;           
  for(int i=0;i<nS;i++)
    if (S[i]!=fullPhrase[i])
      return 0;
  return 1;
}

int
getPartLine(std::istream& fh,std::string& Out,std::string& Excess,const int spc)
  /*!
    Gets a line and determine if there is addition component to add
    in the case of a very long line.
    \param fh :: input stream to get line 
    \param Out :: string up to last 'tab' or ' '
    \param Excess :: string after 'tab or ' ' 
    \param spc :: number of char to try to read 
    \retval 1 :: more line to be found
    \retval -1 :: Error with file
    \retval 0  :: line finished.
  */
{
  std::string Line;
  if (fh.good())
    {
      char* ss=new char[spc+1];
      const int clen=spc-Out.length();
      fh.getline(ss,clen,'\n');
      ss[clen+1]=0;           // incase line failed to read completely
      Out+=static_cast<std::string>(ss);
      delete [] ss;                   
      // remove trailing comments
      std::string::size_type pos = Out.find_first_of("#!");        
      if (pos!=std::string::npos)
        {
	  Out.erase(pos); 
	  return 0;
	}
      if (fh.gcount()==clen-1)         // cont line
        {
	  pos=Out.find_last_of("\t ");
	  if (pos!=std::string::npos)
	    {
	      Excess=Out.substr(pos,std::string::npos);
	      Out.erase(pos);
	    }
	  else
	    Excess.erase(0,std::string::npos);
	  fh.clear();
	  return 1;
	}
      return 0;
    }
  return -1;
}

std::string
removeSpace(const std::string& CLine)
  /*!
    Removes all spaces from a string 
    except those with in the form '\ '
    \param CLine :: Line to strip
    \return String without space
  */
{
  std::string Out;
  char prev='x';
  for(unsigned int i=0;i<CLine.length();i++)
    {
      if (!isspace(CLine[i]) || prev=='\\')
        {
	  Out+=CLine[i];
	  prev=CLine[i];
	}
    }
  return Out;
}
	

std::string 
getLine(std::istream& fh,const int spc)
  /*!
    Reads a line from the stream of max length spc.
    Trailing comments are removed. (with # or ! character)
    \param fh :: already open file handle
    \param spc :: max number of characters to read 
    \return String read.
  */

{
  char* ss=new char[spc+1];
  std::string Line;
  if (fh.good())
    {
      fh.getline(ss,spc,'\n');
      ss[spc]=0;           // incase line failed to read completely
      Line=ss;
      // remove trailing comments
      std::string::size_type pos = Line.find_first_of("#!");
      if (pos!=std::string::npos)
	Line.erase(pos); 
    }
  delete [] ss;
  return Line;
}

int
isEmpty(const std::string& A)
  /*!
    Determines if a string is only spaces
    \param A :: string to check
    \returns 1 on an empty string , 0 on failure
  */
{
  std::string::size_type pos=
    A.find_first_not_of(" \t");
  return (pos!=std::string::npos) ? 0 : 1;
}

void
stripComment(std::string& A)
  /*!
    removes the string after the comment type of 
    '$ ' or '!' or '#  '
    \param A :: String to process
  */
{
  std::string::size_type posA=A.find("$ ");
  std::string::size_type posB=A.find("# ");
  std::string::size_type posC=A.find("!");
  if (posA>posB)
    posA=posB;
  if (posA>posC)
    posA=posC;
  if (posA!=std::string::npos)
    A.erase(posA,std::string::npos);
  return;
}

std::string
fullBlock(const std::string& A)
  /*!
    Returns the string from the first non-space to the 
    last non-space 
    \param A :: string to process
    \returns shortened string
  */
{
  std::string::size_type posA=A.find_first_not_of(" ");
  std::string::size_type posB=A.find_last_not_of(" ");
  if (posA==std::string::npos)
    return "";
  return A.substr(posA,1+posB-posA);
}

template<typename T>
int
sectPartNum(std::string& A,T& out)
  /*!
    Takes a character string and evaluates 
    the first [typename T] object. The string is then 
    erase upt to the end of number.
    The diffierence between this and section is that
    it allows trailing characters after the number. 
    \param out :: place for output
    \param A :: string to process
    \returns 1 on success 0 on failure
   */ 
{
  if (A.empty())
    return 0;

  std::istringstream cx;
  T retval;
  cx.str(A);
  cx.clear();
  cx>>retval;
  const int xpt=cx.tellg();
  if (xpt<0)
    return 0;
  A.erase(0,xpt);
  out=retval;
  return 1; 
}

template<typename T>
int 
section(char* cA,T& out)
  /*!
    Takes a character string and evaluates 
    the first [typename T] object. The string is then filled with
    spaces upto the end of the [typename T] object
    \param out :: place for output
    \param cA :: char array for input and output. 
    \returns 1 on success 0 on failure
   */ 
{
  if (!cA) return 0;
  std::string sA(cA);
  const int item(section(sA,out));
  if (item)
    {
      strcpy(cA,sA.c_str());
      return 1;
    }
  return 0;
}

template<typename T>
int
section(std::string& A,T& out)
  /* 
    takes a character string and evaluates 
    the first <T> object. The string is then filled with
    spaces upto the end of the <T> object
    \param out :: place for output
    \param A :: string for input and output. 
    \return 1 on success 0 on failure
  */
{
  if (A.empty()) return 0;
  std::istringstream cx;
  T retval;
  cx.str(A);
  cx.clear();
  cx>>retval;
  if (cx.fail())
    return 0;
  const int xpt=cx.tellg();
  const char xc=cx.get();
  if (!cx.fail() && !isspace(xc))
    return 0;
  A.erase(0,xpt);
  out=retval;
  return 1;
}

template<typename T>
int
sectionMCNPX(std::string& A,T& out)
  /* 
    Takes a character string and evaluates 
    the first [T] object. The string is then filled with
    spaces upto the end of the [T] object.
    This version deals with MCNPX numbers. Those
    are numbers that are crushed together like
    - 5.4938e+04-3.32923e-6
    \param out :: place for output
    \param A :: string for input and output. 
    \return 1 on success 0 on failure
  */
{
  if (A.empty()) return 0;
  std::istringstream cx;
  T retval;
  cx.str(A);
  cx.clear();
  cx>>retval;
  if (!cx.fail())
    {
      int xpt=cx.tellg();
      const char xc=cx.get();
      if (!cx.fail() && !isspace(xc) 
	  && (xc!='-' || xpt<5))
	return 0;
      A.erase(0,xpt);
      out=retval;
      return 1;
    }
  return 0;
}

std::vector<std::string>
StrParts(std::string Ln)
  /*!
    Splits the sting into parts that are space delminated.
    \param Ln :: line component to strip
    \returns vector of components
  */
{
  std::vector<std::string> Out;
  std::string Part;
  while(section(Ln,Part))
    Out.push_back(Part);
  return Out;
}

template<typename T>
int
convPartNum(const std::string& A,T& out)
  /*!
    Takes a character string and evaluates 
    the first [typename T] object. The string is then 
    erase upt to the end of number.
    The diffierence between this and section is that
    it allows trailing characters after the number. 
    \param out :: place for output
    \param A :: string to process
    \retval number of char read on success
    \retval 0 on failure
   */ 
{
  if (A.empty()) return 0;
  std::istringstream cx;
  T retval;
  cx.str(A);
  cx.clear();
  cx>>retval;
  const int xpt=cx.tellg();
  if (xpt<0)
    return 0;
  out=retval;
  return xpt; 
}

template<typename T>
int
convert(const std::string& A,T& out)
  /*!
    Convert a string into a value 
    \param A :: string to pass
    \param out :: value if found
    \returns 0 on failure 1 on success
  */
{
  if (A.empty()) return 0;
  std::istringstream cx;
  T retval;
  cx.str(A);
  cx.clear();
  cx>>retval;
  if (cx.fail())  
    return 0;
  const char clast=cx.get();
  if (!cx.fail() && !isspace(clast))
    return 0;
  out=retval;
  return 1;
}

template<typename T>
int
convert(const char* A,T& out)
  /*!
    Convert a string into a value 
    \param A :: string to pass
    \param out :: value if found
    \returns 0 on failure 1 on success
  */
{
  // No string, no conversion
  if (!A) return 0;
  std::string Cx=A;
  return convert(Cx,out);
}

float
getVAXnum(const float A) 
  /*!
    Converts a vax number into a standard unix number
    \param A :: float number as read from a VAX file
    \returns float A in IEEE little eindian format
  */
{
  union 
   {
     char a[4];
     float f;
     int ival;
   } Bd;

  int sign,expt,fmask;
  float frac;
  double onum;

  Bd.f=A;
  sign  = (Bd.ival & 0x8000) ? -1 : 1;
  expt = ((Bd.ival & 0x7f80) >> 7);   //reveresed ? 
  if (!expt) 
    return 0.0;

  fmask = ((Bd.ival & 0x7f) << 16) | ((Bd.ival & 0xffff0000) >> 16);
  expt-=128;
  fmask |=  0x800000;
  frac = (float) fmask  / 0x1000000;
  onum= frac * sign * 
         pow(2.0,expt);
  return (float) onum;
}

}
};

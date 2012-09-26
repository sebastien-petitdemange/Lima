//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
%MappedType HeaderMap
{
%TypeHeaderCode
#include <map>
#include <string>
typedef std::map<std::string,std::string> HeaderMap;
typedef std::pair<std::string, std::string> HeaderValue;
%End

%ConvertToTypeCode
if (sipIsErr == NULL)
   return PyDict_Check(sipPy);

    *sipCppPtr = new HeaderMap();
    PyObject *key, *value;
    SIP_SSIZE_T pos = 0;

    while (PyDict_Next(sipPy, &pos, &key, &value)) 
    {
	if(PyString_Check(key) && PyString_Check(value))
	   (*sipCppPtr)->insert(HeaderValue(PyString_AS_STRING(key),PyString_AS_STRING(value)));
    }

    return sipGetState(sipTransferObj);
%End

%ConvertFromTypeCode
    PyObject* aReturnDict = PyDict_New();
    for(HeaderMap::iterator i = sipCpp->begin();i != sipCpp->end();++i)
	{
	  PyObject *value = PyString_FromString(i->second.c_str());
	  PyDict_SetItemString(aReturnDict,i->first.c_str(),value);
	  Py_DECREF(value);
	}
return aReturnDict;
%End

};

%MappedType HeaderValue
{
%TypeHeaderCode
#include <string>
#include <map>
typedef std::pair<std::string, std::string> HeaderValue;
%End

%ConvertToTypeCode
if (sipIsErr == NULL)
   {
     bool aCheckFlag = PySequence_Check(sipPy) && PySequence_Length(sipPy) == 2;
     if(aCheckFlag)
	{
	 PyObject *first = PySequence_GetItem(sipPy,0);
	 PyObject *second = PySequence_GetItem(sipPy,1);
	 aCheckFlag = (PyString_Check(first) && PyString_Check(second));
	 Py_DECREF(first);Py_DECREF(second);
	}
     return aCheckFlag;
   }
    PyObject *first = PySequence_GetItem(sipPy,0);
    PyObject *second = PySequence_GetItem(sipPy,1);
    *sipCppPtr = new HeaderValue(PyString_AS_STRING(first),PyString_AS_STRING(second));
    Py_DECREF(first);Py_DECREF(second);

    return sipGetState(sipTransferObj);
%End

%ConvertFromTypeCode
PyObject *key = PyString_FromString(sipCpp->first.c_str());
PyObject *value = PyString_FromString(sipCpp->second.c_str());
PyObject* aReturnTuple = PyTuple_Pack(2,key,value);
Py_DECREF(key);Py_DECREF(value);

return aReturnTuple;
%End

};

%MappedType CtSaving::Parameters::PrefixListType
{
%TypeHeaderCode
#include <CtSaving.h>
%End

%ConvertToTypeCode
  if(sipIsErr == NULL)
    {
      bool aReturnFlag = PyList_Check(sipPy);
      for(int i = 0;aReturnFlag && i < PyList_Size(sipPy);++i)
	aReturnFlag = sipCanConvertToType(PyList_GET_ITEM(sipPy,i),
					  sipType_CtSaving_Parameters_PrefixCnt,SIP_NOT_NONE);
      return aReturnFlag;
    }
  
  CtSaving::Parameters::PrefixListType *aPrefixsPt = new CtSaving::Parameters::PrefixListType();
  for(int i = 0;i < PyList_Size(sipPy);++i)
    {
      int state;
      CtSaving::Parameters::PrefixCnt *aCnt = 
	reinterpret_cast<CtSaving::Parameters::PrefixCnt*>(sipConvertToType(PySequence_Fast_GET_ITEM(sipPy,i),
									    sipType_CtSaving_Parameters_PrefixCnt, 
									    0,
									    SIP_NOT_NONE,
									    &state,sipIsErr));
      if(*sipIsErr)
	{
	  sipReleaseType(aCnt,sipType_CtSaving_Parameters_PrefixCnt,state);
	  delete aPrefixsPt;
	  return 0;
	}
      aPrefixsPt->push_back(*aCnt);
      sipReleaseType(aCnt,sipType_CtSaving_Parameters_PrefixCnt,state);
    }
  *sipCppPtr = aPrefixsPt;
  return sipGetState(sipTransferObj);
%End
     
%ConvertFromTypeCode
  PyObject *l;
  if(!(l = PyList_New(sipCpp->size())))
    return NULL;

  sipTransferObj = NULL;
  int aListIndex = 0;
  for(CtSaving::Parameters::PrefixListType::const_iterator i = sipCpp->begin();
      i != sipCpp->end();++i,++aListIndex)
    {
      CtSaving::Parameters::PrefixCnt *aCnt = new CtSaving::Parameters::PrefixCnt(*i);
      PyObject *wobj = sipConvertFromNewType(aCnt,sipType_CtSaving_Parameters_PrefixCnt,sipTransferObj);
      if(!wobj)
	{
	  delete aCnt;
	  Py_DECREF(l);
	  return NULL;
	}
      PyList_SET_ITEM(l,aListIndex,wobj);
    }
  return l;
%End
};

class CtSaving 
{
%TypeHeaderCode
#include <CtSaving.h>
#include <sstream>
using namespace lima;
%End
  public:

    CtSaving(CtControl&);
    ~CtSaving();

    enum ManagedMode
      {
	Software,
	Hardware
      };
  
    enum FileFormat {
      RAW,
      EDF,
      CBFFormat,
      NXS,
      FITS,
      HARDWARE_SPECIFIC,
    };

    enum SavingMode {
      Manual,
      AutoFrame,
      AutoHeader,
    };
	
    enum OverwritePolicy {
      Abort,
      Overwrite,
      Append,
    };	

    struct Parameters {
      struct PrefixCnt
      {
	PrefixCnt(const std::string& arg_prefix = "",
		  int arg_nextNumber = 0,
		  int arg_timesUsed = -1,
		  int arg_framesPerFile = 1);

	std::string 	prefix;
	int	    	timesUsed;
	long	    	nextNumber;
	long		framesPerFile;

	const char* __repr__();
%MethodCode
	std::ostringstream str;
	str << *sipCpp;	
	const std::string& tmpString = str.str();
	sipRes = tmpString.c_str();
%End
      };
      typedef std::list<PrefixCnt> PrefixListType;

      std::string directory;
      PrefixListType prefixs;
      std::string suffix;
      ImageType   imageType;
      CtSaving::FileFormat fileFormat;
      CtSaving::SavingMode savingMode;
      CtSaving::OverwritePolicy overwritePolicy;
      std::string indexFormat;
      long nbframes;

      Parameters();
      void checkValid() const;

      const char* __repr__();
%MethodCode
	std::ostringstream str;
	str << *sipCpp;	
	const std::string& tmpString = str.str();
	sipRes = tmpString.c_str();
%End
    };


    // --- file parameters

    void setParameters(const Parameters &pars, int stream_idx=0);
    void getParameters(Parameters& pars /Out/, int stream_idx=0) const;

    void setDirectory(const std::string &directory, int stream_idx=0);
    void getDirectory(std::string &directory /Out/, int stream_idx=0) const;

    void addPrefix(const Parameters::PrefixCnt &prefix,int stream_idx=0);
    void setPrefix(const std::string &prefix, int stream_idx=0);
    void setPrefix(const Parameters::PrefixCnt &prefix,int stream_idx = 0);
    void setPrefix(const Parameters::PrefixListType &prefix, int stream_idx=0);
    void getPrefix(Parameters::PrefixListType& prefix /Out/, int stream_idx=0) const;
    void clearPrefix();

    void setSuffix(const std::string &suffix, int stream_idx=0);
    void getSuffix(std::string &suffix /Out/, int stream_idx=0) const;

    void setFormat(FileFormat format, int stream_idx=0);
    void getFormat(FileFormat &format /Out/, int stream_idx=0) const;

    // --- saving modes

    void setSavingMode(SavingMode mode);
    void getSavingMode(SavingMode& mode /Out/) const;

    void setOverwritePolicy(OverwritePolicy policy, int stream_idx=0);
    void getOverwritePolicy(OverwritePolicy& policy /Out/, 
                            int stream_idx=0) const;

    // --- common headers

    void resetCommonHeader();
    void setCommonHeader(const HeaderMap &header);
    void updateCommonHeader(const HeaderMap &header);
    void getCommonHeader(HeaderMap &header /Out/) const;

    void addToCommonHeader(const HeaderValue &value);

    // --- frame headers

    void updateFrameHeader(long frame_nr, const HeaderMap &header);
    void addToFrameHeader(long frame_nr,const HeaderValue &value);
    void validateFrameHeader(long frame_nr);
    void getFrameHeader(long frame_nr,HeaderMap &header /Out/) const;
    void takeFrameHeader(long frame_nr,HeaderMap &header /Out/);

    void removeFrameHeader(long frame_nr);
    void removeAllFrameHeaders();

    void frameReady(Data &);
    void resetLastFrameNb();

    void setEndCallback(TaskEventCallback *);
    // --- statistic

    void getWriteTimeStatistic(std::list<double>& /Out/, 
                               int stream_idx=0) const;
    void setStatisticHistorySize(int aSize, int stream_idx=0);

    // --- misc

    void clear();
    void writeFrame(int frame_nr = -1, int nb_frames = 1,
		    bool synchronous = true);

    void setStreamActive(int stream_idx, bool  active);
    void getStreamActive(int stream_idx, bool& active /Out/) const;
 };

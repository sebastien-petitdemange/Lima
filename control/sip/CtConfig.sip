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
class CtConfig
{
%TypeHeaderCode
#include <CtConfig.h>
using namespace lima;
%End
public:
  enum Module {All = -1,Acquisition,Saving,Image,Accumulation,Video,Shutter};

  CtConfig(CtControl &);
  ~CtConfig();
    
  void setFilename(const std::string&);
  void getFilename(std::string& /Out/) const;

  // --- set current config into a context alias
  void store(const std::string& alias,
	     CtConfig::Module);
  void store(const std::string& alias,
	     const std::list<CtConfig::Module>&);
  // --- add current config to a context alias
  void storeAdd(const std::string& alias,
		CtConfig::Module);
  void storeAdd(const std::string& alias,
		const std::list<CtConfig::Module>&);
    
  void getAlias(std::list<std::string>& /Out/) const;
  void apply(const std::string&);
  void pop(const std::string&);
  void remove(const std::string&,CtConfig::Module = CtConfig::All);
  void remove(const std::string&,
	      const std::list<CtConfig::Module>&);
  void save();
  private:
  CtConfig(const CtConfig &);
};

%MappedType std::list<CtConfig::Module>
{
%TypeHeaderCode
#include <list>
#include <CtConfig.h>
%End

%ConvertToTypeCode
if(sipIsErr == NULL)
 return PySequence_Check(sipPy);

*sipCppPtr = new std::list<CtConfig::Module>(PyList_Size(sipPy));
for(int i = 0;i < PyList_Size(sipPy);++i)
{
  PyObject *itemPt = PyList_GetItem(sipPy,i);
  int state;
#if (SIP_VERSION > 0x040800)
  CtConfig::Module *modulePt = (CtConfig::Module*)sipConvertToType(itemPt,sipType_CtConfig_Module, sipTransferObj,SIP_NOT_NONE, &state,sipIsErr);
#else
  CtConfig::Module *modulePt = (CtConfig::Module*)sipConvertToInstance(itemPt,sipClass_CtConfig_Module, sipTransferObj,SIP_NOT_NONE, &state,sipIsErr);
#endif
  if(!*sipIsErr)
  {
    if(modulePt)
      (*sipCppPtr)->push_back(CtConfig::Module(*modulePt));
  }
#if (SIP_VERSION >= 0x040800)
  sipReleaseType(modulePt,sipType_CtConfig_Module,state);
#else
  sipReleaseInstance(modulePt,sipClass_CtConfig_Module,state);
#endif
}
return sipGetState(sipTransferObj);
%End

%ConvertFromTypeCode
PyObject *returnList =  PyList_New(sipCpp->size());
int aPos = 0;
for(std::list<CtConfig::Module>::iterator i = sipCpp->begin();
    i != sipCpp->end();++i,++aPos)
{
#if (SIP_VERSION >= 0x040800)
  PyObject *aModulePt = sipConvertFromEnum(*i,sipType_CtConfig_Module); 
#else
  PyObject *aModulePt = sipConvertFromInstance(&(*i),sipClass_CtConfig_Module,NULL); 
#endif
  PyList_SET_ITEM(returnList,aPos,aModulePt);
  Py_DECREF(aModulePt);
}
return returnList;
%End
};

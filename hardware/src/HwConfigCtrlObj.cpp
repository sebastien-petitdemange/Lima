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

#include <libconfig.h++>

#include <HwConfigCtrlObj.h>

using namespace lima;

HwConfigCtrlObj::HwConfigCtrlObj()
{
}

HwConfigCtrlObj::~HwConfigCtrlObj()
{
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,bool& value) const
{
  return m_setting->lookupValue(alias,value);
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,int& value) const
{
  return m_setting->lookupValue(alias,value);
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,long long& value) const
{
  return m_setting->lookupValue(alias,value);
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,double& value) const
{
  return m_setting->lookupValue(alias,value);
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,const char*& value) const
{
  return m_setting->lookupValue(alias,value);
}

bool HwConfigCtrlObj::Setting::lookupValue(const char* alias,std::string& value) const
{
  return m_setting->lookupValue(alias,value);
}

// --- modifiers
#define SET_VALUE(Type)				\
  if(m_setting->exists(alias))			\
    m_setting->operator[](alias) = value;	\
  else						\
    {									\
      libconfig::Setting& nSetting = m_setting->add(alias,Type);	\
      nSetting = value;							\
    }

void HwConfigCtrlObj::Setting::setValue(const char* alias,bool value)
{
  SET_VALUE(libconfig::Setting::TypeBoolean);
}

void HwConfigCtrlObj::Setting::setValue(const char* alias,int value)
{
  SET_VALUE(libconfig::Setting::TypeInt);
}

void HwConfigCtrlObj::Setting::setValue(const char* alias,long long value)
{
  SET_VALUE(libconfig::Setting::TypeInt64);
}

void HwConfigCtrlObj::Setting::setValue(const char* alias,double value)
{
  SET_VALUE(libconfig::Setting::TypeFloat);
}

void HwConfigCtrlObj::Setting::setValue(const char* alias,const char* value)
{
  SET_VALUE(libconfig::Setting::TypeString);
}

void HwConfigCtrlObj::Setting::setValue(const char* alias,
					const std::string &value)
{
  SET_VALUE(libconfig::Setting::TypeString);
}

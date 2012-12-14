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

#ifndef HWCONFIGCTRLOBJ_H
#define HWCONFIGCTRLOBJ_H
#include "Debug.h"
#include "LimaCompatibility.h"

namespace libconfig
{
  class Setting;
}

namespace lima
{
  class CtConfig;
  class LIMACORE_API HwConfigCtrlObj
  {
    DEB_CLASS(DebModHardware,"HwConfigCtrlObj");
  public:
    class LIMACORE_API Setting
    {
      friend class CtConfig;
    public:
      Setting(libconfig::Setting* setting = NULL) : m_setting(setting) {}
      // --- lookup
      bool lookupValue(const char* alias,bool& value) const;
      bool lookupValue(const char* alias,int& value) const;
      bool lookupValue(const char* alias,long long& value) const;
      bool lookupValue(const char* alias,double& value) const;
      bool lookupValue(const char* alias,const char*& value) const;
      bool lookupValue(const char* alias,std::string &value) const;

      // --- modifiers
      void setValue(const char* alias,bool value);
      void setValue(const char* alias,int value);
      void setValue(const char* alias,long long value);
      void setValue(const char* alias,double value);
      void setValue(const char* alias,const char* value);
      void setValue(const char* alias,const std::string &value);
    private:
      libconfig::Setting* m_setting;
    };

    HwConfigCtrlObj();
    virtual ~HwConfigCtrlObj();
    
    virtual void store(Setting&) = 0;
    virtual void restore(const Setting &) = 0;
  };
}
#endif

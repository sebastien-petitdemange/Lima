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
#ifndef CTCONFIGCONTEXT_H
#define CTCONFIGCONTEXT_H
#include <string>

#include "CtControl.h"

namespace libconfig
{
  class Config;
}

namespace lima
{
  class LIMACORE_API CtConfig
  {
    DEB_CLASS_NAMESPC(DebModControl,"Config","Control");
  public:
    enum Module {All = -1,Acquisition,Saving,
		 Image,Accumulation,Video,Shutter,Camera};

    CtConfig(CtControl &);
    ~CtConfig();
    
    void setFilename(const std::string&);
    void getFilename(std::string&) const;

    // --- set current config into a context alias
    void store(const std::string& alias,
	       Module);
    void store(const std::string& alias,
	       const std::list<Module>&);
    // --- add current config to a context alias
    void storeAdd(const std::string& alias,
		  Module);
    void storeAdd(const std::string& alias,
		  const std::list<Module>&);
    
    void getAlias(std::list<std::string>&) const;
    void apply(const std::string&);
    void pop(const std::string&);
    void remove(const std::string&,Module = All);
    void remove(const std::string&,
		const std::list<Module>&);
    void save();

  private:
    CtConfig(const CtConfig &other): m_ctrl(other.m_ctrl) {}

    CtControl&		m_ctrl;
    libconfig::Config*	m_config;
    std::string		m_file_name;
    bool		m_has_hwconfig;
    HwConfigCtrlObj*	m_hwconfig;
  };
}
#endif


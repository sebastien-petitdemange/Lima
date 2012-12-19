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
#include <unistd.h>

#include "CtAccumulation.h"
#include "CtAcquisition.h"
#include "CtConfig.h"
#include "CtImage.h"
#include "CtSaving.h"
#include "CtShutter.h"
#include "CtVideo.h"

#include "HwConfigCtrlObj.h"
#include "HwInterface.h"

using namespace lima;

//Static function
static void _remove_if_exists(libconfig::Setting &setting,
			      const std::string& alias)
{
  if(setting.exists(alias))
    setting.remove(alias);
}

static void _fill_context_with_acquisition_parameters(CtControl& ctrl,
						      libconfig::Setting& setting)
{
  CtAcquisition* acq = ctrl.acquisition();
  CtAcquisition::Parameters pars;
  acq->getPars(pars);

  libconfig::Setting &acq_setting = setting.add("Acquisition",
						libconfig::Setting::TypeGroup);

  libconfig::Setting& acqMode_setting =
    acq_setting.add("acqMode",libconfig::Setting::TypeString);
  acqMode_setting = convert_2_string(pars.acqMode);

  libconfig::Setting& accTimeMode_setting =
    acq_setting.add("accTimeMode",libconfig::Setting::TypeString);
  accTimeMode_setting = convert_2_string(pars.accTimeMode);

  libconfig::Setting& acqNbFrames_setting =
    acq_setting.add("acqNbFrames",libconfig::Setting::TypeInt);
  acqNbFrames_setting = pars.acqNbFrames;

  libconfig::Setting& acqExpoTime_setting =
    acq_setting.add("acqExpoTime",libconfig::Setting::TypeFloat);
  acqExpoTime_setting = pars.acqExpoTime;

  libconfig::Setting& accMaxExpoTime_setting =
    acq_setting.add("accMaxExpoTime",libconfig::Setting::TypeFloat);
  accMaxExpoTime_setting = pars.accMaxExpoTime;

  libconfig::Setting& concatNbFrames_setting =
    acq_setting.add("concatNbFrames",libconfig::Setting::TypeInt);
  concatNbFrames_setting = pars.concatNbFrames;

  libconfig::Setting& latencyTime_setting =
    acq_setting.add("latencyTime",libconfig::Setting::TypeFloat);
  latencyTime_setting = pars.latencyTime;

  libconfig::Setting& triggerMode_setting =
    acq_setting.add("triggerMode",libconfig::Setting::TypeString);
  triggerMode_setting = convert_2_string(pars.triggerMode);
}

static void _fill_context_with_saving_parameters(CtControl& ctrl,
						 libconfig::Setting& setting)
{
  CtSaving* saving = ctrl.saving();
  CtSaving::Parameters pars;
  saving->getParameters(pars);

  libconfig::Setting &saving_setting = setting.add("Saving",
						libconfig::Setting::TypeGroup);

  libconfig::Setting& directory_setting =
    saving_setting.add("directory",libconfig::Setting::TypeString);
  directory_setting = pars.directory;

  libconfig::Setting& prefix_setting =
    saving_setting.add("prefix",libconfig::Setting::TypeString);
  prefix_setting = pars.prefix;

  libconfig::Setting& suffix_setting =
    saving_setting.add("suffix",libconfig::Setting::TypeString);
  suffix_setting = pars.suffix;

  libconfig::Setting& imageType_setting =
    saving_setting.add("imageType",libconfig::Setting::TypeString);
  imageType_setting = convert_2_string(pars.imageType);

  libconfig::Setting& nextNumber_setting =
    saving_setting.add("nextNumber",libconfig::Setting::TypeInt);
  nextNumber_setting = pars.nextNumber;

  libconfig::Setting& fileFormat_setting =
    saving_setting.add("fileFormat",libconfig::Setting::TypeString);
  fileFormat_setting = convert_2_string(pars.fileFormat);

  libconfig::Setting& savingMode_setting =
    saving_setting.add("savingMode",libconfig::Setting::TypeString);
  savingMode_setting = convert_2_string(pars.savingMode);

  libconfig::Setting& overwritePolicy_setting =
    saving_setting.add("overwritePolicy",libconfig::Setting::TypeString);
  overwritePolicy_setting = convert_2_string(pars.overwritePolicy);

  libconfig::Setting& indexFormat_setting =
    saving_setting.add("indexFormat",libconfig::Setting::TypeString);
  indexFormat_setting = pars.indexFormat;

  libconfig::Setting& framesPerFile_setting =
    saving_setting.add("framesPerFile",libconfig::Setting::TypeInt);
  framesPerFile_setting = pars.framesPerFile;

  libconfig::Setting& nbframes_setting =
    saving_setting.add("nbframes",libconfig::Setting::TypeInt);
  nbframes_setting = pars.nbframes;

  CtSaving::ManagedMode managedmode;
  saving->getManagedMode(managedmode);
  libconfig::Setting& managedmode_setting =
    saving_setting.add("managedmode",libconfig::Setting::TypeString);
  managedmode_setting = convert_2_string(managedmode);
}

static void _fill_context_with_image_parameters(CtControl& ctrl,
						libconfig::Setting& setting)
{
  CtImage* image = ctrl.image();

  libconfig::Setting &image_setting = setting.add("Image",
						  libconfig::Setting::TypeGroup);
  CtImage::ImageOpMode imageOpMode;
  image->getMode(imageOpMode);
  libconfig::Setting& imageOpMode_setting =
    image_setting.add("imageOpMode",libconfig::Setting::TypeString);
  imageOpMode_setting = convert_2_string(imageOpMode);

  // --- Roi
  Roi roi;
  image->getRoi(roi);
  libconfig::Setting& roi_setting =
    image_setting.add("roi",libconfig::Setting::TypeGroup);

  const Point& topleft = roi.getTopLeft();
  libconfig::Setting& roi_x_setting =
    roi_setting.add("x",libconfig::Setting::TypeInt);
  roi_x_setting = topleft.x;
  libconfig::Setting& roi_y_setting =
    roi_setting.add("y",libconfig::Setting::TypeInt);
  roi_y_setting = topleft.y;

  const Size& roiSize = roi.getSize();
  libconfig::Setting& roi_width_setting =
    roi_setting.add("width",libconfig::Setting::TypeInt);
  roi_width_setting = roiSize.getWidth();
  libconfig::Setting& roi_height_setting =
    roi_setting.add("height",libconfig::Setting::TypeInt);
  roi_height_setting = roiSize.getHeight();
  
  // --- Bin
  Bin bin;
  image->getBin(bin);
  libconfig::Setting& bin_setting =
    image_setting.add("bin",libconfig::Setting::TypeGroup);

  libconfig::Setting& bin_x_setting =
    bin_setting.add("x",libconfig::Setting::TypeInt);
  bin_x_setting = bin.getX();
  libconfig::Setting& bin_y_setting =
    bin_setting.add("y",libconfig::Setting::TypeInt);
  bin_y_setting = bin.getY();

  // --- Flip
  Flip flip;
  image->getFlip(flip);
  libconfig::Setting& flip_setting =
    image_setting.add("flip",libconfig::Setting::TypeGroup);

  libconfig::Setting& flip_x_setting =
    flip_setting.add("x",libconfig::Setting::TypeBoolean);
  flip_x_setting = flip.x;
  libconfig::Setting& flip_y_setting =
    flip_setting.add("y",libconfig::Setting::TypeBoolean);
  flip_y_setting = flip.y;

  // --- Rotation
  RotationMode rMode;
  image->getRotation(rMode);
  
  libconfig::Setting& rotation_setting =
    image_setting.add("rotation",libconfig::Setting::TypeString);
  rotation_setting = convert_2_string(rMode);
}

static void _fill_context_with_accumulation_parameters(CtControl& ctrl,
						       libconfig::Setting& setting)
{

  CtAccumulation* accumulation = ctrl.accumulation();
  CtAccumulation::Parameters pars;
  accumulation->getParameters(pars);
  
  libconfig::Setting &accumulation_setting = setting.add("Accumulation",
							 libconfig::Setting::TypeGroup);

  libconfig::Setting& active_setting =
    accumulation_setting.add("active",libconfig::Setting::TypeBoolean);
  active_setting = pars.active;

  libconfig::Setting& pixelThresholdValue_setting =
    accumulation_setting.add("pixelThresholdValue",libconfig::Setting::TypeInt64);
  pixelThresholdValue_setting = pars.pixelThresholdValue;

  libconfig::Setting& savingFlag_setting =
    accumulation_setting.add("savingFlag",libconfig::Setting::TypeBoolean);
  savingFlag_setting = pars.savingFlag;

  libconfig::Setting& savePrefix_setting =
    accumulation_setting.add("savePrefix",libconfig::Setting::TypeString);
  savePrefix_setting = pars.savePrefix;
}
static void _fill_context_with_video_parameters(CtControl& ctrl,
						libconfig::Setting& setting)
{
  CtVideo* video = ctrl.video();
  CtVideo::Parameters pars;
  video->getParameters(pars);

  libconfig::Setting &video_setting = setting.add("Video",
						  libconfig::Setting::TypeGroup);

 
  libconfig::Setting& live_setting =
    video_setting.add("live",libconfig::Setting::TypeBoolean);
  live_setting = pars.live;

  libconfig::Setting& exposure_setting =
    video_setting.add("exposure",libconfig::Setting::TypeFloat);
  exposure_setting = pars.exposure;

  libconfig::Setting& gain_setting =
    video_setting.add("gain",libconfig::Setting::TypeFloat);
  gain_setting = pars.gain;

  libconfig::Setting& mode_setting =
    video_setting.add("mode",libconfig::Setting::TypeString);
  mode_setting = convert_2_string(pars.mode);

  // --- Roi
  libconfig::Setting& roi_setting =
    video_setting.add("roi",libconfig::Setting::TypeGroup);

  const Point& topleft = pars.roi.getTopLeft();
  libconfig::Setting& roi_x_setting =
    roi_setting.add("x",libconfig::Setting::TypeInt);
  roi_x_setting = topleft.x;
  libconfig::Setting& roi_y_setting =
    roi_setting.add("y",libconfig::Setting::TypeInt);
  roi_y_setting = topleft.y;

  const Size& roiSize = pars.roi.getSize();
  libconfig::Setting& roi_width_setting =
    roi_setting.add("width",libconfig::Setting::TypeInt);
  roi_width_setting = roiSize.getWidth();
  libconfig::Setting& roi_height_setting =
    roi_setting.add("height",libconfig::Setting::TypeInt);
  roi_height_setting = roiSize.getHeight();

  // --- Bin
  libconfig::Setting& bin_setting =
    video_setting.add("bin",libconfig::Setting::TypeGroup);

  libconfig::Setting& bin_x_setting =
    bin_setting.add("x",libconfig::Setting::TypeInt);
  bin_x_setting = pars.bin.getX();
  libconfig::Setting& bin_y_setting =
    bin_setting.add("y",libconfig::Setting::TypeInt);
  bin_y_setting = pars.bin.getY();

}
static void _fill_context_with_shutter_parameters(CtControl& ctrl,
						  libconfig::Setting& setting)
{
  CtShutter* shutter = ctrl.shutter();
  CtShutter::Parameters pars;
  try
    {
      shutter->getParameters(pars);
    }
  catch(...)			// no shutter capability
    {
      return;
    }

  libconfig::Setting &shutter_setting = setting.add("Shutter",
						    libconfig::Setting::TypeGroup);

  libconfig::Setting& mode_setting =
    shutter_setting.add("mode",libconfig::Setting::TypeString);
  mode_setting = convert_2_string(pars.mode);

  libconfig::Setting& close_time_setting =
    shutter_setting.add("close_time",libconfig::Setting::TypeFloat);
  close_time_setting = pars.close_time;

  libconfig::Setting& open_time_setting =
    shutter_setting.add("open_time",libconfig::Setting::TypeFloat);
  open_time_setting = pars.open_time;
}

static void _fill_context_with_camera_parameters(CtControl& ctrl,
						 HwConfigCtrlObj* hwconfig,
						 libconfig::Setting& setting)
{
  libconfig::Setting &camera_setting = setting.add("Camera",
						   libconfig::Setting::TypeGroup);
  HwConfigCtrlObj::Setting aSetting(&camera_setting);
  hwconfig->store(aSetting);
}

static void _fill_context_with_current_config(CtControl& ctrl,
					      HwConfigCtrlObj* hwconfig,
					      CtConfig::Module module,
					      libconfig::Setting& setting)
{
  //Acquisition
  if(module == CtConfig::All ||
     module == CtConfig::Acquisition)
    _fill_context_with_acquisition_parameters(ctrl,setting);
  //Saving
  if(module == CtConfig::All ||
     module == CtConfig::Saving)
    _fill_context_with_saving_parameters(ctrl,setting);
  //Image
  if(module == CtConfig::All ||
     module == CtConfig::Image)
    _fill_context_with_image_parameters(ctrl,setting);
  //Accumulation
  if(module == CtConfig::All ||
     module == CtConfig::Accumulation)
    _fill_context_with_accumulation_parameters(ctrl,setting);
  //Video
  if(module == CtConfig::All ||
     module == CtConfig::Video)
    _fill_context_with_video_parameters(ctrl,setting);
  //Shutter
  if(module == CtConfig::All ||
     module == CtConfig::Shutter)
    _fill_context_with_shutter_parameters(ctrl,setting);
  //Camera
  if(hwconfig &&
     (module == CtConfig::All ||
      module == CtConfig::Shutter))
    _fill_context_with_camera_parameters(ctrl,hwconfig,setting);
}

static void _apply_acquisition_parameters(CtControl &ctrl,
					  libconfig::Setting& setting)
{
  if(!setting.exists("Acquisition"))
    return;

  CtAcquisition* acq = ctrl.acquisition();
  CtAcquisition::Parameters pars;
  acq->getPars(pars);


  libconfig::Setting &acq_setting = setting["Acquisition"];

  std::string strAcqMode;
  if(acq_setting.lookupValue("acqMode",strAcqMode))
    convert_from_string(strAcqMode,pars.acqMode);

  std::string straccTimeMode;
  if(acq_setting.lookupValue("accTimeMode",straccTimeMode))
    convert_from_string(straccTimeMode,pars.accTimeMode);

  acq_setting.lookupValue("acqNbFrames",pars.acqNbFrames);
  acq_setting.lookupValue("acqExpoTime",pars.acqExpoTime);
  acq_setting.lookupValue("accMaxExpoTime",pars.accMaxExpoTime);
  acq_setting.lookupValue("concatNbFrames",pars.concatNbFrames);
  acq_setting.lookupValue("latencyTime",pars.latencyTime);

  std::string strtriggerMode;
  if(acq_setting.lookupValue("triggerMode",strtriggerMode))
    convert_from_string(strtriggerMode,pars.triggerMode);

  acq->setPars(pars);
}

static void _apply_saving_parameters(CtControl &ctrl,
				     libconfig::Setting& setting)
{
  if(!setting.exists("Saving"))
    return;

  CtSaving* saving = ctrl.saving();
  CtSaving::Parameters pars;
  saving->getParameters(pars);

 
  libconfig::Setting &saving_setting = setting["Saving"];

  saving_setting.lookupValue("directory",pars.directory);
  saving_setting.lookupValue("prefix",pars.prefix);
  saving_setting.lookupValue("suffix",pars.suffix);

  std::string strimageType;
  if(saving_setting.lookupValue("imageType",strimageType))
    convert_from_string(strimageType,pars.imageType);

  int nextNumber;
  if(saving_setting.lookupValue("nextNumber",nextNumber))
    pars.nextNumber = nextNumber;

  std::string strfileFormat;
  if(saving_setting.lookupValue("fileFormat",strfileFormat))
    convert_from_string(strfileFormat,pars.fileFormat);

  std::string strsavingMode;
  if(saving_setting.lookupValue("savingMode",strsavingMode))
    convert_from_string(strsavingMode,pars.savingMode);

  std::string stroverwritePolicy;
  if(saving_setting.lookupValue("overwritePolicy",stroverwritePolicy))
    convert_from_string(stroverwritePolicy,pars.overwritePolicy);

  saving_setting.lookupValue("indexFormat",pars.indexFormat);

  int framesPerFile;
  if(saving_setting.lookupValue("framesPerFile",framesPerFile))
    pars.framesPerFile = framesPerFile;

  int nbframes;
  if(saving_setting.lookupValue("nbframes",nbframes))
    pars.nbframes = nbframes;

  std::string strmanagedmode;
  if(saving_setting.lookupValue("managedmode",strmanagedmode))
    {
      CtSaving::ManagedMode managedmode;
      convert_from_string(strmanagedmode,managedmode);
      saving->getManagedMode(managedmode);
    }

  saving->setParameters(pars);
}

static void _apply_image_parameters(CtControl &ctrl,
				    libconfig::Setting& setting) 
{
  if(!setting.exists("Image"))
    return;

  CtImage* image = ctrl.image();
  

  libconfig::Setting &image_setting = setting["Image"];

  std::string strimageOpMode;
  if(setting.lookupValue("imageOpMode",strimageOpMode))
    {
      CtImage::ImageOpMode imageOpMode;
      convert_from_string(strimageOpMode,imageOpMode);
      image->setMode(imageOpMode);
    }
  // --- Flip
  if(image_setting.exists("flip"))
    {
      libconfig::Setting& flip_setting = image_setting["flip"];
      Flip flip;
      if(flip_setting.lookupValue("x",flip.x) &&
	 flip_setting.lookupValue("y",flip.y))
	image->setFlip(flip);
    }
  // --- Bin
  if(image_setting.exists("bin"))
    {
      int x,y;
  
      libconfig::Setting& bin_setting = image_setting["bin"];
      if(bin_setting.lookupValue("x",x) &&
	 bin_setting.lookupValue("y",y))
	{
	  Bin aBin(x,y);
	  image->setBin(aBin);
	}
    }
  // --- Roi
  if(image_setting.exists("roi"))
    {
      libconfig::Setting& roi_setting = image_setting["roi"];
      Point topleft;
      int width,height;
      if(roi_setting.lookupValue("x",topleft.x) &&
	 roi_setting.lookupValue("y",topleft.y) &&
	 roi_setting.lookupValue("width",width) &&
	 roi_setting.lookupValue("height",height))
	{
	  Roi aRoi(topleft.x,topleft.y,width,height);
	  image->setRoi(aRoi);
	}
    }
  // --- Rotation
  std::string strrMode;
  if(image_setting.lookupValue("rotation",strrMode))
    {
      RotationMode rMode;
      convert_from_string(strrMode,rMode);
      image->setRotation(rMode);
    }
}

static void _apply_accumulation_parameters(CtControl &ctrl,
					   libconfig::Setting& setting) 
{
  if(!setting.exists("Accumulation"))
    return;

  CtAccumulation* accumulation = ctrl.accumulation();
  CtAccumulation::Parameters pars;
  accumulation->getParameters(pars);
  
  libconfig::Setting &accumulation_setting = setting["Accumulation"];

  accumulation_setting.lookupValue("active",pars.active);
  accumulation_setting.lookupValue("pixelThresholdValue",
				   pars.pixelThresholdValue);
  accumulation_setting.lookupValue("savingFlag",pars.savingFlag);
  accumulation_setting.lookupValue("savePrefix",pars.savePrefix);

  accumulation->setParameters(pars);
}

static void _apply_video_parameters(CtControl &ctrl,
				    libconfig::Setting& setting) 
{
  if(!setting.exists("Video"))
    return;

  CtVideo* video = ctrl.video();
  CtVideo::Parameters pars;
  video->getParameters(pars);

  libconfig::Setting &video_setting = setting["Video"];

  video_setting.lookupValue("live",pars.live);
  video_setting.lookupValue("exposure",pars.exposure);
  video_setting.lookupValue("gain",pars.gain);

  std::string strmode;
  if(video_setting.lookupValue("mode",strmode))
    convert_from_string(strmode,pars.mode);

  // --- Bin
  if(video_setting.exists("bin"))
    {
      libconfig::Setting& bin_setting = video_setting["bin"];

      int x,y;
      if(bin_setting.lookupValue("x",x) &&
	 bin_setting.lookupValue("y",y))
	pars.bin = Bin(x,y);
    }
  // --- Roi
  if(video_setting.exists("roi"))
    {
      libconfig::Setting& roi_setting = video_setting["roi"];

      Point topleft;
      int width,height;
      if(roi_setting.lookupValue("x",topleft.x) &&
	 roi_setting.lookupValue("y",topleft.y) &&
	 roi_setting.lookupValue("width",width) &&
	 roi_setting.lookupValue("height",height))
	pars.roi = Roi(topleft.x,topleft.y,
		       width,height);
    }
  video->setParameters(pars);
}

static void _apply_shutter_parameters(CtControl &ctrl,
				      libconfig::Setting& setting) 
{
  if(!setting.exists("Shutter"))
    return;

  CtShutter* shutter = ctrl.shutter();
  CtShutter::Parameters pars;
  shutter->getParameters(pars);

  libconfig::Setting &shutter_setting = setting["Shutter"];

  std::string strmode;
  if(shutter_setting.lookupValue("mode",strmode))
    convert_from_string(strmode,pars.mode);

  shutter_setting.lookupValue("close_time",pars.close_time);
  shutter_setting.lookupValue("open_time",pars.open_time);
  
  shutter->setParameters(pars);
}

static void _apply_camera_parameters(CtControl &ctrl,
				     HwConfigCtrlObj *hwconfig,
				     libconfig::Setting& setting)
{
  if(!setting.exists("Camera") || !hwconfig)
    return;

  libconfig::Setting &camera_setting = setting["Camera"];
  HwConfigCtrlObj::Setting aSetting(&camera_setting);
  hwconfig->restore(aSetting);
}
static void _apply_context(CtControl &ctrl,
			   HwConfigCtrlObj* hwconfig,
			   libconfig::Setting& setting)
{
  //Camera
  _apply_camera_parameters(ctrl,hwconfig,setting);
  //Acquisition
  _apply_acquisition_parameters(ctrl,setting);
  //Saving
  _apply_saving_parameters(ctrl,setting);
  //Image
  _apply_image_parameters(ctrl,setting);
  //Accumulation
  _apply_accumulation_parameters(ctrl,setting);
  //Video
  _apply_video_parameters(ctrl,setting);
  //Shutter
  _apply_shutter_parameters(ctrl,setting);
}

static void _remove_context_if_exists(CtConfig::Module module,
				      bool has_hwconfig,
				      libconfig::Setting& setting)
{
    //Acquisition
  if(module == CtConfig::All ||
     module == CtConfig::Acquisition)
    _remove_if_exists(setting,"Acquisition");
  //Saving
  if(module == CtConfig::All ||
     module == CtConfig::Saving)
    _remove_if_exists(setting,"Saving");
  //Image
  if(module == CtConfig::All ||
     module == CtConfig::Image)
    _remove_if_exists(setting,"Image");
  //Accumulation
  if(module == CtConfig::All ||
     module == CtConfig::Accumulation)
    _remove_if_exists(setting,"Accumulation");
  //Video
  if(module == CtConfig::All ||
     module == CtConfig::Video)
    _remove_if_exists(setting,"Video");
  //Shutter
  if(module == CtConfig::All ||
     module == CtConfig::Shutter)
    _remove_if_exists(setting,"Shutter");
  //Camera
  if(has_hwconfig &&
     (module == CtConfig::All ||
      module == CtConfig::Camera))
    _remove_if_exists(setting,"Camera");
}

CtConfig::CtConfig(CtControl &control) :
  m_ctrl(control),
  m_config(new libconfig::Config())
{
  HwInterface *hw = control.hwInterface();
  m_has_hwconfig = hw->getHwCtrlObj(m_hwconfig);
}

CtConfig::~CtConfig()
{
  try
    {
      save();
    }
  catch(...)
    {
    }
  delete m_config;
}
/** @brief set the full path for the filename used for saving configuration
 */
void CtConfig::setFilename(const std::string &full_path)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(full_path);

  m_file_name = full_path;
}

void CtConfig::getFilename(std::string &full_path) const
{
  full_path = m_file_name;
}

void CtConfig::store(const std::string& alias,
			  Module module_to_save)
{
  std::list<Module> modules_to_save;
  modules_to_save.push_back(module_to_save);
  store(alias,modules_to_save);
}

void CtConfig::store(const std::string& alias,
		     const std::list<Module>& modules_to_save)
{
  libconfig::Setting& root = m_config->getRoot();
  _remove_if_exists(root,alias);
  libconfig::Setting& alias_setting =
    root.add(alias,libconfig::Setting::TypeGroup);

  for(std::list<Module>::const_iterator i = modules_to_save.begin();
      i != modules_to_save.end();++i)
    _fill_context_with_current_config(m_ctrl,this->m_hwconfig,*i,alias_setting);
}

void CtConfig::update(const std::string& alias,
			Module module_to_save)
{
  std::list<Module> modules_to_save;
  modules_to_save.push_back(module_to_save);
  update(alias,modules_to_save);
}

void CtConfig::update(const std::string& alias,
			const std::list<Module>& modules_to_save)

{
  libconfig::Setting& root = m_config->getRoot();
  if(root.exists(alias))
    {
      libconfig::Setting& alias_setting = root[alias];
      for(std::list<Module>::const_iterator i = modules_to_save.begin();
	  i != modules_to_save.end();++i)
	{
	  _remove_context_if_exists(*i,m_has_hwconfig,alias_setting);
	  _fill_context_with_current_config(m_ctrl,m_hwconfig,*i,alias_setting);
	}
    }
  else
    store(alias,modules_to_save);
}

void CtConfig::getAlias(std::list<std::string>& aliases) const
{
  libconfig::Setting& root = m_config->getRoot();
  int nbAlias = root.getLength();
  for(int i = 0;i < nbAlias;++i)
    {
      const libconfig::Setting &a = root[i];
      if(a.isGroup() && a.getName())
	aliases.push_back(a.getName());
    }
}

void CtConfig::apply(const std::string& alias)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(alias);

  libconfig::Setting& root = m_config->getRoot();
  try
    {  
      libconfig::Setting& alias_setting = root[alias];
      _apply_context(m_ctrl,m_hwconfig,alias_setting);
    }
  catch(libconfig::SettingTypeException &exp)
    {
      THROW_CTL_ERROR(Error) << exp.what();
    }
  catch(libconfig::SettingNotFoundException& exp)
    {
      THROW_CTL_ERROR(Error) << exp.what();
    }
}

void CtConfig::pop(const std::string& alias)
{
  apply(alias);
  remove(alias);
}

void CtConfig::remove(const std::string& alias,Module modules_to_remove)
{
  libconfig::Setting& root = m_config->getRoot();
  if(modules_to_remove == All)
    _remove_if_exists(root,alias);
  else if(root.exists(alias))
    {
      libconfig::Setting& alias_setting = root[alias];
      _remove_context_if_exists(modules_to_remove,m_has_hwconfig,alias_setting);
    }
}

void CtConfig::remove(const std::string& alias,
		      const std::list<Module>& modules_to_remove)
{
  libconfig::Setting& root = m_config->getRoot();
  if(!root.exists(alias)) return;

  libconfig::Setting& alias_setting = root[alias];
  for(std::list<Module>::const_iterator i = modules_to_remove.begin();
      i != modules_to_remove.end();++i)
    {
      if(*i == All)
	{
	  root.remove(alias);
	  break;
	}
      else
	_remove_context_if_exists(*i,m_has_hwconfig,alias_setting);
    }
}

void CtConfig::save()
{
  DEB_MEMBER_FUNCT();

  try
    {
      m_config->writeFile(m_file_name.c_str());
    }
  catch(libconfig::FileIOException &exp)
    {
      THROW_CTL_ERROR(Error) << exp.what();
    }
}

void CtConfig::load()
{
  DEB_MEMBER_FUNCT();

  try
    {
      m_config->readFile(m_file_name.c_str());
    }
  catch(libconfig::ParseException &exp)
    {
      THROW_CTL_ERROR(Error) << exp.getError();
    }
  catch(libconfig::FileIOException &exp)
    {
      THROW_CTL_ERROR(Error) << exp.what();
    }
}

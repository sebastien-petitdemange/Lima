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
#ifndef CTSAVING_H
#define CTSAVING_H

#include <map>
#include <list>
#include <string>
#include <fstream>
#include <ios>

#include "LimaCompatibility.h"
#include "ThreadUtils.h"
#include "CtControl.h"
#include "HwSavingCtrlObj.h"

struct Data;
class TaskEventCallback;
class SinkTaskBase;

namespace lima {
  /** @brief Saving management
   *
   * With this class you manage the image saving in different format
   */
  class LIMACORE_API CtSaving 
  {
    DEB_CLASS_NAMESPC(DebModControl,"Saving","Control");

    friend class CtControl;
  public:
    CtSaving(CtControl&);
    ~CtSaving();

    enum ManagedMode
      {
	Software,		///< Saving will be managed by Lima Core (Control)
	Hardware		///< Saving will be managed by Hardware or Camera SDK
      };

    enum FileFormat 
      {
	HARDWARE_SPECIFIC = -1,	///< extended hardware format (ADSC,MarCCD...) @see setHardwareFormat
	RAW,			///< Raw format (no header)
	EDF,			///< EDF format (Esrf Data Format)
	CBFFormat,		///< CBF format
	NXS,			///< Soleil Nexus format
	FITS,			///< Flexible Image Transport Layer (NOST)
      };

    enum SavingMode 
      {
	Manual,			///< No automatic saving, you should call CtSaving::writeFrame
	AutoFrame,		///< Save a frame just after it acquisition
	AutoHeader,		///< Save the frame if header and the data of the frame is available 
      };
	
    enum OverwritePolicy 
      {
	Abort,			///< Abort acquisition if file already exist
	Overwrite,		///< Overwrite old files
	Append,			///< Append new data at the end of already existing files
      };	

    class Stream;
    class SaveContainer;

    struct LIMACORE_API Parameters 
    {
      struct PrefixCnt
      {
	PrefixCnt(const std::string& arg_prefix = "",
		  int arg_nextNumber = 0,
		  int arg_timesUsed = -1,
		  int arg_framesPerFile = 1) :
	  prefix(arg_prefix),
	  timesUsed(arg_timesUsed),
	  nextNumber(arg_nextNumber),
	  framesPerFile(arg_framesPerFile){}

	std::string 	prefix;		///< prefix of the filename
	int	    	timesUsed;	///< nb times used
	long	    	nextNumber;	///< next file number
	long		framesPerFile;	///< the number of images save in one files
      };
      typedef std::list<PrefixCnt> PrefixListType;

      std::string directory;	///< base path where the files will be saved
      PrefixListType prefixs;
      std::string suffix;	///< suffix of the filename
      ImageType   imageType;
      FileFormat fileFormat;	///< the saving format (EDF,CBF...)
      SavingMode savingMode;	///< saving mode (automatic,manual...)
      OverwritePolicy overwritePolicy; ///< how you the saving react it find existing filename
      ManagedMode managedMode;	///< two option either harware (manage by SDK,hardware) or software (Lima core)
      std::string indexFormat;	///< ie: %.4d if you want 4 digits
      long nbframes;
      
      Parameters();
      void checkValid() const;
    private:
      friend class Stream;
      friend class SaveContainer;
      void _initCurrentPrefix();
      void _nextPrefix();
      PrefixCnt& _currentPrefix() const;

      PrefixListType::iterator	m_current_prefix;
      int			m_prefix_times_used;
    };
    
    typedef std::pair<std::string, std::string> HeaderValue;
    typedef std::map<std::string,std::string> HeaderMap;
    typedef std::map<long,Data> FrameMap;

    // --- file parameters

    void setParameters(const Parameters &pars, int stream_idx=0);
    void getParameters(Parameters& pars, int stream_idx=0) const;

    void setDirectory(const std::string &directory, int stream_idx=0);
    void getDirectory(std::string& directory, int stream_idx=0) const;

    void addPrefix(const Parameters::PrefixCnt &prefix,int stream_idx=0);
    void setPrefix(const std::string &prefix, int stream_idx=0);
    void setPrefix(const Parameters::PrefixCnt &prefix,int stream_idx = 0);
    void setPrefix(const Parameters::PrefixListType &prefix, int stream_idx=0);
    void getPrefix(Parameters::PrefixListType& prefix, int stream_idx=0) const;
    void clearPrefix();

    void setSuffix(const std::string &suffix, int stream_idx=0);
    void getSuffix(std::string& suffix, int stream_idx=0) const;
    
    void setFormat(FileFormat format, int stream_idx=0);
    void getFormat(FileFormat& format, int stream_idx=0) const;

    void getHardwareFormatList(std::list<std::string> &format_list) const;
    void setHardwareFormat(const std::string &format);
    void getHardwareFormat(std::string &format) const;
    // --- saving modes

    void setSavingMode(SavingMode mode);
    void getSavingMode(SavingMode& mode) const;

    bool hasAutoSaveMode()
    { return m_stream[0]->hasAutoSaveMode(); }

    void setOverwritePolicy(OverwritePolicy policy, int stream_idx=0);
    void getOverwritePolicy(OverwritePolicy& policy, int stream_idx=0) const;
    
    void setManagedMode(ManagedMode mode);
    void getManagedMode(ManagedMode &mode) const;
    // --- common headers

    void resetCommonHeader();
    void setCommonHeader(const HeaderMap &header);
    void updateCommonHeader(const HeaderMap &header);
    void getCommonHeader(HeaderMap& header) const;
    void addToCommonHeader(const HeaderValue &value);

    // --- frame headers

    void updateFrameHeader(long frame_nr, const HeaderMap &header);
    void addToFrameHeader(long frame_nr,const HeaderValue &value);
    void validateFrameHeader(long frame_nr);
    void getFrameHeader(long frame_nr, HeaderMap& header) const;
    void takeFrameHeader(long frame_nr, HeaderMap& header);

    void removeFrameHeader(long frame_nr);
    void removeAllFrameHeaders();

    void frameReady(Data&);
    void resetLastFrameNb();

    void setEndCallback(TaskEventCallback *);

    // --- statistic

    void getWriteTimeStatistic(std::list<double>&, int stream_idx=0) const;
    void setStatisticHistorySize(int aSize, int stream_idx=0);

    // --- misc

    void clear();
    //                  frame_nr == -1 => last frame
    void writeFrame(int frame_nr = -1, int nb_frames = 1,bool synchronous = true); 


    void setStreamActive(int stream_idx, bool  active);
    void getStreamActive(int stream_idx, bool& active) const;

    class LIMACORE_API SaveContainer
      {
      DEB_CLASS_NAMESPC(DebModControl,"Saving Container","Control");
    public:
      SaveContainer(Stream& stream);
      virtual ~SaveContainer();
      
      void open(const CtSaving::Parameters&);
      void close();
      void writeFile(Data&,CtSaving::HeaderMap &);
      void setStatisticSize(int aSize);
      void getStatistic(std::list<double>&) const;
      void getParameters(CtSaving::Parameters&) const;
      void clear();
      
      /** @brief should return true if container has compression or
       *  havy task to do before saving
       *  if return is true, getCompressionTask should return a Task
       * @see getCompressionTask
       */
      virtual bool needParallelCompression() const {return false;}
      /** @brief get a new compression task at each call.
       * this methode is not call if needParallelCompression return false
       *  @see needParallelCompression
       */
      virtual SinkTaskBase* getCompressionTask(const CtSaving::HeaderMap&) {return NULL;}

    protected:
      virtual bool _open(const std::string &filename,
		  std::ios_base::openmode flags) = 0;
      virtual void _close() = 0;
      virtual void _writeFile(Data &data,
			      CtSaving::HeaderMap &aHeader,
			      FileFormat) = 0;
      virtual void _clear() {};

      int			m_written_frames;
    private:
      Stream			&m_stream;
      std::list<double>		m_statistic_list;
      int			m_statistic_size;
      mutable Cond		m_cond;
      bool			m_file_opened;
    };
    friend class SaveContainer;

    enum ParameterType {
      Acq, Cache, Auto, 
    };

    enum TaskType {
      Save, Compression,
    };

    class LIMACORE_API Stream 
    {
      DEB_CLASS(DebModControl, "CtSaving::Stream");

    public:
      Stream(CtSaving& aCtSaving, int idx);
      ~Stream();

      int getIndex() const
      { return m_idx; }

      const Parameters& getParameters(ParameterType type) const;
      Parameters& getParameters(ParameterType type);
      void setParameters(const Parameters& pars);
      void updateParameters();

      void prepare();
      void createSaveContainer();
      void checkWriteAccess();
      
      bool needCompression()
      { return m_save_cnt->needParallelCompression(); }

      void setSavingError(CtControl::ErrorCode error)
      { m_saving._setSavingError(error); }

      SinkTaskBase *getTask(TaskType type, const HeaderMap& header);

      void compressionFinished(Data& data);
      void saveFinished(Data& data);
      int getNextNumber() const;

      bool isActive() const
      { return m_active; }
      void setActive(bool active);

      void writeFile(Data& data, HeaderMap& header);

      bool hasAutoSaveMode()
      { const Parameters& pars = getParameters(Cache);
	return pars.savingMode != Manual; 
      }

      void getStatistic(std::list<double>& stat_list) const
      { m_save_cnt->getStatistic(stat_list); }
      void setStatisticSize(int size) 
      { m_save_cnt->setStatisticSize(size); }

      void clear()
      { m_save_cnt->clear(); }

    private:
      class _SaveCBK;
      class _SaveTask;
      class _CompressionCBK;

      CtSaving&			m_saving;
      int			m_idx;

      SaveContainer 	       *m_save_cnt;
      _SaveCBK	 	       *m_saving_cbk;
      Parameters		m_pars;
      Parameters		m_reference_pars;
      Parameters		m_acquisition_pars;
      bool			m_pars_dirty_flag;
      bool			m_active;
      _CompressionCBK 	       *m_compression_cbk;
    };
    friend class Stream;

  private:
    class	_ManualBackgroundSaveTask;
    friend class _ManualBackgroundSaveTask;
    class	_NewFrameSaveCBK;
    friend class _NewFrameSaveCBK;
    typedef std::vector<SinkTaskBase *> TaskList;
    typedef std::map<long, long>	FrameCbkCountMap;
    typedef std::map<long, HeaderMap>	FrameHeaderMap;

    CtControl& 			m_ctrl;

    int				m_nb_stream;
    Stream		      **m_stream;

    HeaderMap			m_common_header;
    FrameHeaderMap		m_frame_headers;
    FrameMap			m_frame_datas;

    mutable Cond		m_cond;
    bool			m_ready_flag;
    long			m_last_frameid_saved;
    bool			m_need_compression;
    FrameCbkCountMap		m_nb_compression_cbk;
    int				m_nb_save_cbk;
    TaskEventCallback	       *m_end_cbk;
    bool			m_has_hwsaving;
    HwSavingCtrlObj*		m_hwsaving;
    _NewFrameSaveCBK*		m_new_frame_save_cbk;

    Stream& getStream(int stream_idx)
    { bool stream_ok = (stream_idx >= 0) && (stream_idx < m_nb_stream);
      return stream_ok ? *m_stream[stream_idx] : getStreamExc(stream_idx); }

    const Stream& getStream(int stream_idx) const
    { bool stream_ok = (stream_idx >= 0) && (stream_idx < m_nb_stream);
      return stream_ok ? *m_stream[stream_idx] : getStreamExc(stream_idx); }

    Stream& getStreamExc(int stream_idx) const;

    SavingMode getAcqSavingMode() const
    { return getStream(0).getParameters(Acq).savingMode; }

    ManagedMode getManagedMode() const
    { return getStream(0).getParameters(Acq).managedMode; }

    // --- from control
    void getSaveCounters(int& first_to_save, int& last_to_save)
    { AutoMutex lock(m_cond.mutex());
      first_to_save = last_to_save = -1;
      FrameMap::const_iterator it, end = m_frame_datas.end();
      for (it = m_frame_datas.begin(); it != end; ++it) {
	if (it->first > last_to_save)
	  last_to_save = it->first;
	if ((first_to_save == -1) || (it->first < first_to_save))
	  first_to_save = it->first;
      }
    }

    // --- internal call
    void _prepare();
    void _getCommonHeader(HeaderMap&);
    void _takeHeader(FrameHeaderMap::iterator&, HeaderMap& header,
		     bool keep_in_map);
    void _getTaskList(TaskType type, long frame_nr, const HeaderMap& header, 
		      TaskList& task_list);
    void _postTaskList(Data&, const TaskList&);
    void _compressionFinished(Data&, Stream&);
    void _saveFinished(Data&, Stream&);
    void _setSavingError(CtControl::ErrorCode);
    void _updateParameters();
    void _synchronousSaving(Data&,HeaderMap&);
    bool _controlIsFault();
    bool _newFrameWrite(int);
  };

  inline std::ostream& operator<<(std::ostream &os,
				  const CtSaving::Parameters::PrefixCnt &prefixCnt)
  {
    os << "<"
       << "prefix=" << prefixCnt.prefix << ", "
       << "timesUsed=" << prefixCnt.timesUsed << ", "
       << "nextNumber=" << prefixCnt.nextNumber << ", "
       << "framesPerFile=" << prefixCnt.framesPerFile
       << ">";
    return os;
  }

  inline std::ostream& operator<<(std::ostream &os,
				  const CtSaving::Parameters::PrefixListType &prefix)
  {
    os << "prefixs=";
    if(prefix.size() > 1)
      os << "<";
    for(CtSaving::Parameters::PrefixListType::const_iterator i = prefix.begin();
	i != prefix.end();++i)
      {
	if(i != prefix.begin())
	  os << ", ";
	os << *i;
      }
    if(prefix.size() > 1)
      os << ">";
    return os;
  }
  
  inline std::ostream& operator<<(std::ostream &os,const CtSaving::Parameters &params)
  {
    const char *aFileFormatHumanPt;
    switch(params.fileFormat)
      {
      case CtSaving::EDF:
	aFileFormatHumanPt = "EDF";break;
      case CtSaving::CBFFormat:
	aFileFormatHumanPt = "CBF";break;
      case CtSaving::NXS:
	aFileFormatHumanPt = "NXS";break;
      default:
	aFileFormatHumanPt = "RAW";break;
      }

    const char *aSavingModeHumanPt;
    switch(params.savingMode)
      {
      case CtSaving::AutoFrame:
	aSavingModeHumanPt = "Auto frame";break;
      case CtSaving::AutoHeader:
	aSavingModeHumanPt = "Auto header";break;
      default: //	Manual
	aSavingModeHumanPt = "Manual";break;
      }

    const char *anOverwritePolicyHumanPt;
    switch(params.overwritePolicy)
      {
      case CtSaving::Overwrite:
	anOverwritePolicyHumanPt = "Overwrite";break;
      case CtSaving::Append:
	anOverwritePolicyHumanPt = "Append";break;
      default:		// Abort
	  anOverwritePolicyHumanPt = "Abort";break;
      }

    os << "<"
       << "directory=" << params.directory << ", "
       << params.prefixs << ", "
       << "suffix=" << params.suffix << ", "
       << "fileFormat=" << params.fileFormat << "," << aFileFormatHumanPt << ", "
       << "savingMode=" << params.savingMode << "," << aSavingModeHumanPt << ", "
       << "overwritePolicy=" << params.overwritePolicy << "," << anOverwritePolicyHumanPt << ", "
       << "nbframes=" << params.nbframes
       << ">";
    return os;
  }

  inline bool operator ==(const CtSaving::Parameters::PrefixCnt& a,
			  const CtSaving::Parameters::PrefixCnt& b)
  {
    return (a.prefix 		== b.prefix 	&&
	    a.timesUsed 	== b.timesUsed 	&&
	    a.nextNumber 	== b.nextNumber &&
	    a.framesPerFile 	== b.framesPerFile);
  }
			  
  
  inline bool operator ==(const CtSaving::Parameters& a,
			  const CtSaving::Parameters& b)
  {
    return ((a.directory       == b.directory)       &&
	    (a.prefixs         == b.prefixs)         &&
	    (a.suffix          == b.suffix)          &&
	    (a.imageType       == b.imageType)       &&
	    (a.fileFormat      == b.fileFormat)      &&
	    (a.savingMode      == b.savingMode)      &&
	    (a.overwritePolicy == b.overwritePolicy) &&
	    (a.indexFormat     == b.indexFormat)     &&
	    (a.nbframes        == b.nbframes));
  }

  inline std::ostream& operator<<(std::ostream &os,const CtSaving::HeaderMap &header)
  {
    os << "< ";
    for(CtSaving::HeaderMap::const_iterator i = header.begin();
	i != header.end();++i)
      os << "(" << i->first << "," << i->second << ") ";
    os << ">";
    return os;
  }
  inline std::ostream& operator<<(std::ostream &os,const CtSaving::HeaderValue &value)
  {
    os << "< (" << value.first << "," << value.second << ") >";
    return os;
  }
} // namespace lima

#endif // CTSAVING_H

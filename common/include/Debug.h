// This code was taken from the Debug.h file (Generig debug in C++)
// in: /segfs/dserver/classes++/extdevice/

#ifndef DEBUG_H
#define DEBUG_H

#include "StreamUtils.h"
#include "ThreadUtils.h"

#include <string>
#include <map>

namespace lima
{

// Do not forget to update the corresponding Type/Format/Module name in
// DebParams::checkInit (Debug.cpp) when you change these enums

enum DebType {
	DebTypeFatal		= 1 << 0,
	DebTypeError		= 1 << 1,
	DebTypeWarning		= 1 << 2,
	DebTypeTrace		= 1 << 3,
	DebTypeFunct		= 1 << 4,
	DebTypeParam		= 1 << 5,
	DebTypeReturn		= 1 << 6,
	DebTypeAlways		= 1 << 7,
};

enum DebFormat {
	DebFmtDateTime		= 1 << 0,
	DebFmtModule		= 1 << 1,
	DebFmtObj		= 1 << 2,
	DebFmtFunct		= 1 << 3,	
	DebFmtFileLine		= 1 << 4,
	DebFmtType		= 1 << 5,
};

enum DebModule {
	DebModNone		= 1 << 0,
	DebModCommon		= 1 << 1,
	DebModHardware		= 1 << 2,
	DebModControl		= 1 << 3,
	DebModSimu		= 1 << 4,
	DebModEspia		= 1 << 5,
	DebModEspiaSerial	= 1 << 6,
	DebModFocla		= 1 << 7,
	DebModFrelon		= 1 << 8,
	DebModFrelonSerial	= 1 << 9,
	DebModMaxipix		= 1 << 10,
};

typedef const char *ConstStr;

/*------------------------------------------------------------------
 *  class DebStream
 *------------------------------------------------------------------*/

class DebStream : public std::ostream
{
 public:
	typedef OCopyStream::StreamList StreamList;
	typedef StreamList::iterator ListIterator;
	enum Selector {
		None, Output, Error, Both
	};

	DebStream();
	
	DebStream& SetStream(Selector new_selector);
	
	bool FindStream(std::ostream *os, Selector buffer);
	void AddOutput(std::ostream *os);
	void RemoveOutput(std::ostream *os);
	void AddError(std::ostream *os);
	void RemoveError(std::ostream *os);

 protected:
	bool Find(std::ostream *os, StreamList& slist, ListIterator& it);

 private:
	StreamList m_out_list;
	StreamList m_err_list;
	StreamList m_all_list;
	
	NullStreamBuf m_null_buf;
	OCopyStream m_out_streams;
	OCopyStream m_err_streams;
	CopyStreamBuf m_all_buf;
	
	Selector m_current;
	std::streambuf *m_buffers[4];
};

inline DebStream& DebStream::SetStream(Selector new_selector)
{
	m_current = new_selector;
	rdbuf(m_buffers[m_current]);
	return *this;
}

/*------------------------------------------------------------------
 *  class DebParams 
 *------------------------------------------------------------------*/

class DebObj;
class DebProxy;

class DebParams
{
 public:
	typedef long long Flags;
	typedef std::vector<std::string> NameList;

	static const Flags AllFlags;

	DebParams(DebModule mod = DebModNone, 
		  ConstStr class_name = NULL, ConstStr name_space = NULL);

	void setModule(DebModule mod);
	DebModule getModule() const;

	void setClassName(ConstStr class_name);
	ConstStr getClassName() const;

	void setNameSpace(ConstStr name_space);
	ConstStr getNameSpace() const;

	bool checkModule() const;
	bool checkType(DebType type) const;

	static void setTypeFlags(Flags type_flags);
	static Flags getTypeFlags();

	static void enableTypeFlags(Flags type_flags);
	static void disableTypeFlags(Flags type_flags);

	static void setFormatFlags(Flags fmt_flags);
	static Flags getFormatFlags();

	static void enableFormatFlags(Flags fmt_flags);
	static void disableFormatFlags(Flags fmt_flags);

	static void setModuleFlags(Flags mod_flags);
	static Flags getModuleFlags();

	static void enableModuleFlags(Flags mod_flags);
	static void disableModuleFlags(Flags mod_flags);

	static void setTypeFlagsNameList(const NameList& type_name_list);
	static NameList getTypeFlagsNameList();

	static void setFormatFlagsNameList(const NameList& fmt_name_list);
	static NameList getFormatFlagsNameList();

	static void setModuleFlagsNameList(const NameList& mod_name_list);
	static NameList getModuleFlagsNameList();

	static DebStream& getDebStream();
	static AutoMutex lock();

	static ConstStr getTypeName(DebType type);
	static ConstStr getFormatName(DebFormat fmt);
	static ConstStr getModuleName(DebModule mod);

private:
	friend class DebProxy;
	friend class DebObj;

	static void checkInit();

	template <class T>
	static void setFlagsNameList(Flags& flags, 
				     const std::map<T, std::string>& name_map,
				     const NameList& name_list);
	template <class T>
	static void getFlagsNameList(Flags flags, 
				     const std::map<T, std::string>& name_map,
				     NameList& name_list);

	static Flags s_type_flags;
	static Flags s_fmt_flags;
	static Flags s_mod_flags;

	static DebStream *s_deb_stream;

	static std::map<DebType,   std::string> *s_type_name_map;
	static std::map<DebFormat, std::string> *s_fmt_name_map;
	static std::map<DebModule, std::string> *s_mod_name_map;

	static Mutex *s_mutex;

	DebModule m_mod;
	ConstStr m_class_name;
	ConstStr m_name_space;
};

std::ostream& operator <<(std::ostream& os, 
			  const DebParams::NameList& name_list);


/*------------------------------------------------------------------
 *  class DebProxy
 *------------------------------------------------------------------*/

class DebProxy
{
 public:
	DebProxy();
	DebProxy(DebObj *deb_obj, DebType type, ConstStr file_name, 
		 int line_nr);
	DebProxy(const DebProxy& p); // should not be called
	~DebProxy();

	template <class T> 
        const DebProxy& operator <<(const T& o) const;

	bool isActive() const;

 private:
	AutoMutex *m_lock;
};


/*------------------------------------------------------------------
 *  class DebObj
 *------------------------------------------------------------------*/

class DebObj
{
 public:
	DebObj(DebParams& deb_params, bool destructor = false,
	       ConstStr funct_name = NULL, ConstStr obj_name = NULL, 
	       ConstStr file_name = NULL, int line_nr = 0);
	~DebObj();

	bool checkOut(DebType type);
	bool checkErr(DebType type);
	bool checkAny(DebType type);

	DebParams& getDebParams();

	DebProxy write(DebType type, 
		       ConstStr file_name = NULL, int line_nr = 0);

 private:
	friend class DebProxy;

	void heading(DebType, ConstStr file_name, int line_nr);

	DebParams *m_deb_params;
	bool m_destructor;
	ConstStr m_funct_name;
	ConstStr m_obj_name;
	ConstStr m_file_name;
	int m_line_nr;
};


/*------------------------------------------------------------------
 *  global inline functions
 *------------------------------------------------------------------*/

inline bool DebHasFlag(DebParams::Flags flags, int val)
{
	return ((flags & val) != 0);
}

/*------------------------------------------------------------------
 *  class DebParams inline functions
 *------------------------------------------------------------------*/

inline DebParams::DebParams(DebModule mod, ConstStr class_name, 
			    ConstStr name_space)
{
	checkInit();
	m_mod = mod;
	m_class_name = class_name;
	m_name_space = name_space;
}

inline void DebParams::setModule(DebModule mod)
{
	m_mod = mod; 
}

inline DebModule DebParams::getModule() const
{ 
	return m_mod; 
}

inline void DebParams::setClassName(ConstStr class_name)
{ 
	m_class_name = class_name; 
}

inline ConstStr DebParams::getClassName() const
{ 
	return m_class_name; 
}

inline void DebParams::setNameSpace(ConstStr name_space)
{ 
	m_name_space = name_space; 
}

inline ConstStr DebParams::getNameSpace() const
{ 
	return m_name_space; 
}

inline bool DebParams::checkModule() const
{ 
	return DebHasFlag(s_mod_flags, m_mod); 
}

inline bool DebParams::checkType(DebType type) const
{ 
	return DebHasFlag(s_type_flags, type); 
}


/*------------------------------------------------------------------
 *  class DebProxy inline functions
 *------------------------------------------------------------------*/

inline DebProxy::DebProxy()
	: m_lock(NULL)
{
}

inline DebProxy::DebProxy(DebObj *deb_obj, DebType type, ConstStr file_name, 
			  int line_nr)
{
	AutoMutex lock(*DebParams::s_mutex);

	deb_obj->heading(type, file_name, line_nr);

	m_lock = new AutoMutex(lock);
}

inline DebProxy::~DebProxy()
{
	if (!m_lock)
		return;

	*DebParams::s_deb_stream << std::endl;
	delete m_lock;
}

inline bool DebProxy::isActive() const
{
	return m_lock;
}

template <class T>
inline const DebProxy& DebProxy::operator <<(const T& o) const
{
	if (isActive()) 
		*DebParams::s_deb_stream << o;
	return *this;
}


/*------------------------------------------------------------------
 *  class DebObj inline functions
 *------------------------------------------------------------------*/

inline DebObj::DebObj(DebParams& deb_params, bool destructor, 
		      ConstStr funct_name, ConstStr obj_name, 
		      ConstStr file_name, int line_nr)
	: m_deb_params(&deb_params), m_destructor(destructor), 
	  m_funct_name(funct_name), m_obj_name(obj_name), 
	  m_file_name(file_name), m_line_nr(line_nr)
{
	write(DebTypeFunct, m_file_name, m_line_nr) << "Enter";
}

inline DebObj::~DebObj()
{
	write(DebTypeFunct, m_file_name, m_line_nr) << "Exit";
}

inline bool DebObj::checkOut(DebType type)
{
	return ((type == DebTypeAlways) || (type == DebTypeFatal) || 
		((type == DebTypeError) && 
		 m_deb_params->checkType(DebTypeError)) ||
		(m_deb_params->checkModule() && 
		 m_deb_params->checkType(type)));
}

inline bool DebObj::checkErr(DebType type)
{
	return ((type == DebTypeFatal) || (type == DebTypeError));
}

inline bool DebObj::checkAny(DebType type)
{
	return checkOut(type) || checkErr(type);
}

inline DebParams& DebObj::getDebParams()
{
	return *m_deb_params;
}

inline DebProxy DebObj::write(DebType type, ConstStr file_name, int line_nr)
{
	if (checkAny(type))
		return DebProxy(this, type, file_name, line_nr);
	else
		return DebProxy();
}

/*------------------------------------------------------------------
 *  debug macros
 *------------------------------------------------------------------*/

#define DEB_CLASS_NAMESPC(mod, class_name, name_space)			\
  private:								\
	static DebParams& getDebParams()				\
	{								\
		static DebParams *deb_params = NULL;			\
		if (!deb_params)					\
			deb_params = new DebParams(mod, class_name,	\
						   name_space);		\
		return *deb_params;					\
	}								\
									\
	void setDebObjName(const std::string& obj_name)			\
	{								\
		m_deb_obj_name = obj_name;				\
	}								\
									\
	ConstStr getDebObjName() const					\
	{								\
		if (m_deb_obj_name.empty())				\
			return NULL;					\
		return m_deb_obj_name.c_str();				\
	}								\
									\
	std::string m_deb_obj_name


#define DEB_MEMBER_FUNCT()						\
	DebObj deb(getDebParams(), false, __FUNCTION__,			\
		   getDebObjName(), __FILE__, __LINE__)

#define DEB_STATIC_FUNCT()						\
	DebObj deb(getDebParams(), false, __FUNCTION__,			\
		   NULL, __FILE__, __LINE__)

#define DEB_CONSTRUCTOR()						\
	DEB_MEMBER_FUNCT()

#define DEB_DESTRUCTOR()						\
	DebObj deb(getDebParams(), true, __FUNCTION__,			\
		   getDebObjName(), __FILE__, __LINE__)

#define DEB_SET_OBJ_NAME(n) \
	setDebObjName(n)

#define DEB_MSG(type)	deb.write(type, __FILE__, __LINE__)

#define DEB_MSG_VAR1(type, v1) \
	DEB_MSG(type) << #v1 "=" << v1

#define DEB_MSG_VAR2(type, v1, v2) \
	DEB_MSG_VAR1(type, v1) << ", " #v2 "=" << v2

#define DEB_MSG_VAR3(type, v1, v2, v3) \
	DEB_MSG_VAR2(type, v1, v2) << ", " #v3 "=" << v3

#define DEB_MSG_VAR4(type, v1, v2, v3, v4) \
	DEB_MSG_VAR3(type, v1, v2, v3) << ", " #v4 "=" << v4

#define DEB_MSG_VAR5(type, v1, v2, v3, v4, v5) \
	DEB_MSG_VAR4(type, v1, v2, v3, v4) << ", " #v5 "=" << v5

#define DEB_MSG_VAR6(type, v1, v2, v3, v4, v5, v6) \
	DEB_MSG_VAR5(type, v1, v2, v3, v4, v5) << ", " #v6 "=" << v6

#define DEB_FATAL()	DEB_MSG(DebTypeFatal)
#define DEB_ERROR()	DEB_MSG(DebTypeError)
#define DEB_WARNING()	DEB_MSG(DebTypeWarning)
#define DEB_TRACE()	DEB_MSG(DebTypeTrace)
#define DEB_PARAM()	DEB_MSG(DebTypeParam)
#define DEB_RETURN()	DEB_MSG(DebTypeReturn)
#define DEB_ALWAYS()	DEB_MSG(DebTypeAlways)

#define DEB_PARAM_VAR1(v1) \
	DEB_MSG_VAR1(DebTypeParam, v1)
#define DEB_PARAM_VAR2(v1, v2) \
	DEB_MSG_VAR2(DebTypeParam, v1, v2)
#define DEB_PARAM_VAR3(v1, v2, v3) \
	DEB_MSG_VAR3(DebTypeParam, v1, v2, v3)
#define DEB_PARAM_VAR4(v1, v2, v3, v4) \
	DEB_MSG_VAR4(DebTypeParam, v1, v2, v3, v4)
#define DEB_PARAM_VAR5(v1, v2, v3, v4, v5) \
	DEB_MSG_VAR5(DebTypeParam, v1, v2, v3, v4, v5)
#define DEB_PARAM_VAR6(v1, v2, v3, v4, v5, v6) \
	DEB_MSG_VAR6(DebTypeParam, v1, v2, v3, v4, v5, v6)

#define DEB_TRACE_VAR1(v1) \
	DEB_MSG_VAR1(DebTypeTrace, v1)
#define DEB_TRACE_VAR2(v1, v2) \
	DEB_MSG_VAR2(DebTypeTrace, v1, v2)
#define DEB_TRACE_VAR3(v1, v2, v3) \
	DEB_MSG_VAR3(DebTypeTrace, v1, v2, v3)
#define DEB_TRACE_VAR4(v1, v2, v3, v4) \
	DEB_MSG_VAR4(DebTypeTrace, v1, v2, v3, v4)
#define DEB_TRACE_VAR5(v1, v2, v3, v4, v5) \
	DEB_MSG_VAR5(DebTypeTrace, v1, v2, v3, v4, v5)
#define DEB_TRACE_VAR6(v1, v2, v3, v4, v5, v6) \
	DEB_MSG_VAR6(DebTypeTrace, v1, v2, v3, v4, v5, v6)

#define DEB_RETURN_VAR1(v1) \
	DEB_MSG_VAR1(DebTypeReturn, v1)
#define DEB_RETURN_VAR2(v1, v2) \
	DEB_MSG_VAR2(DebTypeReturn, v1, v2)
#define DEB_RETURN_VAR3(v1, v2, v3) \
	DEB_MSG_VAR3(DebTypeReturn, v1, v2, v3)
#define DEB_RETURN_VAR4(v1, v2, v3, v4) \
	DEB_MSG_VAR4(DebTypeReturn, v1, v2, v3, v4)
#define DEB_RETURN_VAR5(v1, v2, v3, v4, v5) \
	DEB_MSG_VAR5(DebTypeReturn, v1, v2, v3, v4, v5)
#define DEB_RETURN_VAR6(v1, v2, v3, v4, v5, v6) \
	DEB_MSG_VAR6(DebTypeReturn, v1, v2, v3, v4, v5, v6)

#define DEB_OBJ_NAME(o) \
	((o)->getDebObjName())

} // namespace lima

#endif // DEBUG_H

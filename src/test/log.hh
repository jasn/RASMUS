// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011, The TPIE development team
// 
// This file is part of TPIE.
// 
// TPIE is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// TPIE is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with TPIE.  If not, see <http://www.gnu.org/licenses/>

#ifndef __TEST_LOG_HH__
#define __TEST_LOG_HH__

#include <test/logstream.hh>
#include <fstream>

namespace rasmus {

/** A simple logger that writes messages to stderr */
class stderr_log_target: public log_target {
public:
	log_level m_threshold;

    /** Construct a new stderr logger
	 * \param threshold record messages at or above this severity threshold
	 * */
	stderr_log_target(log_level threshold);
	
	/** Implement \ref log_target virtual method to record message
	 * \param level severity of message
	 * \param message content of message
	 * \param size lenght of message array
	 * */
	void log(log_level level, const char * message, size_t size);
};


///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_init to initialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void init_default_log();

///////////////////////////////////////////////////////////////////////////////
/// \internal \brief Used by tpie_finish to deinitialize the log subsystem.
///////////////////////////////////////////////////////////////////////////////
void finish_default_log();

extern logstream log_singleton;

///////////////////////////////////////////////////////////////////////////
/// \brief Returns the only logstream object. 
///////////////////////////////////////////////////////////////////////////
inline logstream & get_log() {return log_singleton;}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing fatal log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_fatal() {return get_log() << setlevel(LOG_FATAL);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing error log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_error() {return get_log() << setlevel(LOG_ERROR);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing info log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_info() {return get_log() << setlevel(LOG_INFORMATIONAL);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing warning log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_warning() {return get_log() << setlevel(LOG_WARNING);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing app_debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_app_debug() {return get_log() << setlevel(LOG_APP_DEBUG);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_debug() {return get_log() << setlevel(LOG_DEBUG);}

///////////////////////////////////////////////////////////////////////////////
/// \brief Return logstream for writing mem_debug log messages.
///////////////////////////////////////////////////////////////////////////////
inline logstream & log_mem_debug() {return get_log() << setlevel(LOG_MEM_DEBUG);}

}  // namespace rasmus

#endif // __TEST_LOG_HH__

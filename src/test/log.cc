// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2008, 2011 The TPIE development team
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

#include <cstdlib>
#include <test/logstream.hh>
#include <test/log.hh>
#include <iostream>

namespace rasmus {

stderr_log_target::stderr_log_target(log_level threshold): m_threshold(threshold) {}

void stderr_log_target::log(log_level level, const char * message, size_t size) {
	if (level > m_threshold) return;
	fwrite(message, 1, size, stderr);
}	

static stderr_log_target * stderr_target = 0;
logstream log_singleton;
static logstream & log = log_singleton;

void init_default_log() {
	if (stderr_target) return;
	stderr_target = new stderr_log_target(LOG_INFORMATIONAL);
	log.add_target(stderr_target);
}

void finish_default_log() {
	if (!stderr_target) return;
	log.remove_target(stderr_target);
	delete stderr_target;
	stderr_target = 0;
}

} //namespace tpie

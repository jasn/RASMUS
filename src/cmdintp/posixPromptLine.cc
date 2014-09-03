// -*- mode: c++; tab-width: 4; indent-tabs-mode: t; eval: (progn (c-set-style "stroustrup") (c-set-offset 'innamespace 0) (c-set-offset 'inextern-lang 0)); -*-
// vi:set ts=4 sts=4 sw=4 noet :
// Copyright 2014 The pyRASMUS development team
// 
// This file is part of pyRASMUS.
// 
// pyRASMUS is free software: you can redistribute it and/or modify it under
// the terms of the GNU Lesser General Public License as published by the
// Free Software Foundation, either version 3 of the License, or (at your
// option) any later version.
// 
// pyRASMUS is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public
// License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with pyRASMUS.  If not, see <http://www.gnu.org/licenses/>
#include "promptLine.hh"
#include <sys/wait.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <string>
#include <cstdlib>

PromptStatus promptLine(bool complete, std::string & str) {
	int fds[2];
	if(pipe(fds) == -1) {
		perror("pipe");
		exit(EXIT_FAILURE);
	}
	
	pid_t cpid = fork();
	if (cpid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	
	if (cpid == 0) {    /* Child reads from pipe */
		signal(SIGINT, SIG_DFL);
		close(fds[0]);          /* Close unused write end */
		
		char * rl = readline(complete?">>>> ":".... ");
		if (!rl) _exit(3);
		if (rl[0] == '\0') _exit(4); 
		write(fds[1], rl, strlen(rl));
		_exit(EXIT_SUCCESS);
	} 
	
	close(fds[1]);
		
	str="";
	while (true) {
		const int size=1024*10;
		char buff[size+1];
		int i=read(fds[0], buff, size);
		if (i < 1) break;
		buff[i]='\0';
		str+=buff;
	}
	close(fds[0]);
	
	int status=0;
	if (waitpid(cpid, &status, 0) == -1) {
		perror("waitpid");
		exit(EXIT_FAILURE);
	}

	if (WIFEXITED(status)) {
		switch(WEXITSTATUS(status)) {
		case 0: 
		  add_history(str.c_str());
		  return PromptStatus::NORMAL;
		case 3: return PromptStatus::DONE;
		case 4: return PromptStatus::EMPTY;
		default: exit(EXIT_FAILURE);
		}
	} else if (WIFSIGNALED(status)) {
		switch(WTERMSIG(status)) {
		case SIGINT: return PromptStatus::CANCEL;
		default: exit(EXIT_FAILURE);
		}
	} else {
		exit(EXIT_FAILURE);
	}
}

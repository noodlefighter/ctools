/*
 * syscmd.c
 *
 *  Created on: 2020-07-06
 *      Author: ebi
 */
#include "syscmd.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>

int syscmd_get_string_from_pipe(const char* cmd, char* buf, size_t buf_size, int *exit_status)
{
	FILE *fp;
	int rv = 0;

	fflush(NULL);
	fp = popen(cmd, "r");
	if(NULL == fp)
		return -1;

	fgets(buf, buf_size, fp); // note: 若执行的命令无输出，此处会一直阻塞，如必要，应使用select之类的方式实现非阻塞读
	rv = pclose(fp);

	if (exit_status)
		*exit_status = WEXITSTATUS(rv);

	return 0;
}

int syscmd_exec(const char *cmd, int *exit_status)
{
	int rv = 0;

	rv = system(cmd);

	if (exit_status)
		*exit_status = WEXITSTATUS(rv);

	return 0;
}

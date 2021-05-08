/*
 * syscmd.h
 *
 *  Created on: 2020-07-06
 *      Author: ebi
 */

#ifndef SYSCMD_H_
#define SYSCMD_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

int syscmd_get_string_from_pipe(const char* cmd, char* buf, size_t buf_size, int *rv);
int syscmd_exec(const char *cmd, int *exit_status);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SYSCMD_H_ */

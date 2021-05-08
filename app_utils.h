/*
 * app_utils.h
 *
 *  Created on: 2020-05-05
 *      Author: ebi
 */
#ifndef APP_UTILS_H_
#define APP_UTILS_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

uint32_t asc2bcd(uint8_t *bcd, const int8_t *asc, int32_t len);
int8_t *bcd2asc(int8_t *asc, const uint8_t *bcd, uint32_t len);

int remove_recursive(const char *path); // 递归删除文件夹

int check_file_exists(const char *filename);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* APP_UTILS_H_ */


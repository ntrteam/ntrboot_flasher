/*
    elm.h
    Copyright (C) 2009 yellow wood goblin

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __ELM_H__
#define __ELM_H__

#include <stdint.h>
#include <sys/iosupport.h>
#include <sys/types.h>
#include <sys/syslimits.h>

#ifdef __cplusplus
extern "C" {
#endif

int ELM_Mount(void);
void ELM_Unmount(void);
int ELM_ClusterSizeFromHandle(int fildes,uint32_t* size);
int ELM_SectorsPerClusterFromHandle(int fildes,uint32_t* per);
int ELM_ClusterSizeFromDisk(int disk,uint32_t* size);
int ELM_ClustersFromDisk(int disk,uint32_t* clusters);
int ELM_FreeClustersFromDisk(int disk,uint32_t* clusters);
int ELM_SectorsFromDisk(int disk,uint32_t* sectors);
uint32_t ELM_GetFAT(int fildes,uint32_t cluster,uint32_t* sector);
int ELM_DirEntry(int fildes,uint64_t* entry);
int ELM_FormatFAT(uint32_t priv_sectors);
uint32_t ELM_GetSectorCount(unsigned char aDrive);


void NandFast(void);
void NandFlush(void);

int dirnext (DIR_ITER *dirState, char *filename, struct stat *filestat);

#ifdef __cplusplus
}
#endif

#define MAX_FILENAME_LENGTH 768 // 256 UCS-2 characters encoded into UTF-8 can use up to 768 UTF-8 chars

#endif

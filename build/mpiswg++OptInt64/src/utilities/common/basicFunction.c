/* Copyright (C) 
 * 2019 - Hu Ren, rh890127a@163.com
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */
/**
 * @file basicFunction.c
 * @brief basic functionalities that would be used universally
 * @author Hu Ren, rh890127a@163.com
 * @version v0.1
 * @date 2019-08-09
 */
#include "basicFunction.h"
#include "stdio.h"

//--------------------------------------------------------------
// file and directory operations
//--------------------------------------------------------------
/**
* @brief Remove a directory recursively 
*/

void rmtree(const char path[])
{
  size_t path_len;
  char *full_path;
  DIR *dir;
  struct stat stat_path, stat_entry;
  struct dirent *entry;

  // stat for the path
  stat(path, &stat_path);

  // if path does not exists or is not dir - exit with status -1
  if (S_ISDIR(stat_path.st_mode) == 0) 
  {
    fprintf(stderr, "%s: %s\n", "Is not directory", path);
    exit(-1);
  }

  // if not possible to read the directory for this user
  if ((dir = opendir(path)) == NULL) 
  {
    fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
    exit(-1);
  }

  // the length of the path
  path_len = strlen((char*) path);

  // iteration through entries in the directory
  while ((entry = readdir(dir)) != NULL) 
  {

    // skip entries "." and ".."
    if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
      continue;

    // determinate a full path of an entry
    full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
    strcpy(full_path, (char*) path);
    strcat(full_path, "/");
    strcat(full_path, entry->d_name);

    // stat for the entry
    stat(full_path, &stat_entry);

    // recursively remove a nested directory
    if (S_ISDIR(stat_entry.st_mode) != 0) 
    {
      rmtree(full_path);
      continue;
    }

    // remove a file object
    if (unlink(full_path) == 0)
      //printf("Removed a file: %s\n", full_path);
  	  printf("");
    else
      printf("Can`t remove a file: %s\n", full_path);
  }

  // remove the devastated directory and close the object of it
  if (rmdir(path) != 0)
    printf("Can`t remove a directory: %s\n", path);

  closedir(dir);
}

/**
 * @brief create a new directory and move the older one to xxx.old
 *        path should be no longer than 1024-6 byte
 */
int remakeDir(const char* path)
{
  char oldPath[1024], newPath[1024];
  int flag, iter;
  
  // Creat newPath and oldPath from input char pointer
  //for(iter = 0; path[iter] != '\0'; iter++)
  //{
  //	if(iter == 1024-6) {
  //	  printf("**Fatal Error: too long path name!\n");
  //	  printf("\t*path is %s \n", path);
  //	  exit(-1);
  //	}
  //	newPath[iter] = path[iter];
  //}
  //newPath[iter] = '\0';
  memcpy(oldPath, (char*) path, strlen((char*) path)+1 );
  //printf("oldPath is now %s\n", oldPath);
  char suffix[] = ".old";
  memcpy(&oldPath[strlen((char*) path)], suffix, 5);
  //printf("oldPath is now %s\n", oldPath);
  
  // Save new path to old one, and creat the new
  if( access(oldPath,F_OK) == 0 )
  {
  	rmtree(oldPath);
  	flag = access(oldPath,F_OK);
  }
  if( access(path,F_OK) == 0 ){
  	flag = rename( path, oldPath);
  }
  
  flag = mkdir( path, S_IRWXU);
  
  return flag;
}


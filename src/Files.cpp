#ifdef _WIN32
#include "dependencies/dirent/includes/dirent.h"
#else
#include <dirent.h>
#endif


struct dirent* getFilesInFolder(const char* path, u32* fileCount)
{
  DIR* directory = opendir(path);
  if (!directory)
    {
      fprintf(stderr, "WARNING: could not open path: %s. Files.cpp\n", path);
      return NULL;
    }

  //Get total numbero f entries
  u32 count = 0;
  
  struct dirent* entry = readdir(directory);
  while (entry != NULL)
    {
      if (entry->d_name[0] != '.')
	{
	  count++;
	}
      entry = readdir(directory);
    }
  closedir(directory);

  //get files
  struct dirent* files = (dirent*)malloc(sizeof(dirent) * count);
  directory = opendir(path);
  
  u32 i = 0;
  entry = readdir(directory);
  while (entry != NULL)
    {
      if (entry->d_name[0] != '.')
	{
	  files[i] = *entry;
	  i++;
	}
      entry = readdir(directory);
    }
  
  closedir(directory);
  *fileCount = count;
  return files;
  
}

char** getFileNamesFromFiles(struct dirent* files, u32 fileCount)
{
  #ifdef _DARWIN_FEATURE_64_BIT_INODE
  u32 nameLen = 1024;
  #else
  
  u32 nameLen = 256;
  #endif

  char** names = (char**)malloc(sizeof(char*) * fileCount);
  
  for (int i = 0; i < fileCount; i++)
    {
      names[i] = (char*)malloc(nameLen);
      strncpy(names[i], files[i].d_name, files[i].d_namlen + 1);
    }

  return names;
}

char** getFileNamesFromPath(const char* path, u32* fileCount)
{
  struct dirent* files =  getFilesInFolder(path, fileCount);
  char** names = getFileNamesFromFiles(files, *fileCount);
  free(files);
  return names;
}


#include "filelog.h"
#include <sys/time.h>
#include <sys/timeb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "thepublic.h"
//g++ -m64 -fPIC -O2 -g -Wall -Werror -c filelog.cpp 
using namespace std;


int OFileLog::iOpenFile()
{
  if(m_pFile)
  {
    fclose(m_pFile);
  }
  string l_strPath;
  char l_strFileName[64]={0};
  time_t ltmt;
  struct tm l_tm;
  time(&ltmt);
  localtime_r(&ltmt,&l_tm);
  l_strPath.append(m_oLogPath.substr(0,this->m_iPathStringLen));

  time(&ltmt);
  snprintf(l_strFileName,sizeof(l_strFileName)-1,"%02d-%02d-%02d.log",l_tm.tm_hour,l_tm.tm_min,l_tm.tm_sec);
  l_strPath.append(l_strFileName);

  
  int l_iTmpRes;
  m_pFile = fopen(l_strPath.c_str(),"w+");
  if(m_pFile == NULL)
  {
  	ErrorLog("open file %s fail ; errno=%d errmsg=%s",l_strPath.c_str(), errno, strerror(errno));
    return -1;
  }

  return 0;
}



int OFileLog::iLogInfo(const char * format,...)
{
  time_t ltmt;
  struct tm l_tm;
  struct  timeval l_detailTm;
  int l_iFormatLen = 0;
  l_iFormatLen = strlen(format);
  time(&ltmt);
  localtime_r(&ltmt,&l_tm);
  
  gettimeofday(&l_detailTm,NULL);
  fprintf(m_pFile,"[%010d][%02d:%02d:%02d:%04ld]",++(this->m_iCurLine),l_tm.tm_hour,l_tm.tm_min,l_tm.tm_sec,l_detailTm.tv_usec);
  
    va_list vl;
    va_start(vl, format);  
 
    vfprintf(m_pFile,format,vl);
    va_end(vl);
    
    if(l_iFormatLen > 2)
    {
      if(format[l_iFormatLen-1] != 'n' || format[l_iFormatLen-2] != '\\')
       {
         fprintf(m_pFile,"\n");
       }
    }
    else
      fprintf(m_pFile,"\n");
    
    fflush(m_pFile);
    
    return 0;
} 


OFileLog::OFileLog(const char * pthePath)
{
    m_iCurLine=0;
    m_pFile=NULL;
    this->iIndicatePath(pthePath);
}
  
int OFileLog::iIndicatePath(const char * p_thePath)
{
   
    struct stat l_strcTheStat;
  int l_iRet = 0;
  int l_iRet2 = 0;
  int l_ithePathLen = 0;
  
  l_ithePathLen = strlen(p_thePath) ;
 
  if(l_ithePathLen != 0)
  {
    l_iRet = stat(p_thePath,&l_strcTheStat);
    if( l_iRet != 0 )
     {
        if(errno == ENOENT)//是否已经存在该文件夹  
        {  
            l_iRet2 = mkdir(p_thePath, 0775);//创建文件夹  
            printf("creat dir '/%s'/\n", p_thePath);  
            if(l_iRet2 < 0)  
            {  
                printf("Could not create directory \'%s\' \n",  p_thePath);  
                return EXIT_FAILURE;  
            }  
   
        }  
        else  
        {  
            printf("bad file path\n");  
            return EXIT_FAILURE;  
        } 
        
     }
     else
     {
     	l_iRet =  S_ISDIR(l_strcTheStat.st_mode);
       if( l_iRet != 1 )
       {
         printf("it is not a path [err=%d]:%s",errno,strerror(errno));
         return -2;  
       }
     }
     //写入
     
     m_oLogPath.append(p_thePath);
     if(p_thePath[l_ithePathLen-1] != '/');
       m_oLogPath.append("/");
     m_iPathStringLen = m_oLogPath.length();
     
  }//int stat(const char *path, struct stat *buf);
  else
    return -1;
}


#ifndef _FILELOG_H_
#define _FILELOG_H_

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sstream>
#include <string>


#define FILE_MAX_LINE 20000000

class OFileLog
{
  public:
    OFileLog(){m_iCurLine=0;m_pFile=NULL;}
    virtual ~OFileLog(){if(m_pFile){fclose(m_pFile);m_pFile=NULL;}}
    OFileLog(const char * pthePath);
        
    
  public:
    int m_iCurLine;
  
    FILE * m_pFile;
    std::string m_oLogPath;
  private:
    
    int m_iPathStringLen; //
   // char m_chLogPath[64]; //¶¨Î»Â·¾¶
  public:
    int iIndicatePath(const char * p_thePath); 

    int iLogInfo(const char * format,...);  
    int iOpenFile();

};




#endif
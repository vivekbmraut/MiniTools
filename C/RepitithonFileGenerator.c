#include<stdio.h>
#include<stdlib.h>
#include<tchar.h>
#include<strsafe.h>
#include<windows.h>
#include<errhandlingapi.h>
#include<fileapi.h>
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"kernel32.lib")

void DisplayErrorBox(LPTSTR lpszFunctionName);
#define MAX_DIGITS_IN_NUMBER 3

BOOL CheckExtensionC(LPTSTR lpszFileName);
/*fileNamePrefix is not Re Entrant*/
BOOL GetFileNamePrefix(LPSTR FileName,TCHAR* fileNamePrefix);
LPVOID NullHeapRealloc(HANDLE hHeap,LPVOID lpMem,SIZE_T dwBytes);
int _tmain(int argc,TCHAR* argv[])
{
    WIN32_FIND_DATA ffd;
    int i=0, j=0, maxFileNumber=0, k=0, FileNumber=0, numberOfFilesToGenerate = 0;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    static TCHAR szFileNamePrefix[MAX_PATH];
    static TCHAR currentFileNamePrefix[MAX_PATH];
    static TCHAR formattedFileName[MAX_PATH];
    static TCHAR szMaxNumber[MAX_DIGITS_IN_NUMBER+1];
    BOOL repeated_file_present = FALSE;
    HANDLE hHeap = NULL;
    LPTSTR* FileNameArr = NULL;
    SIZE_T FileArrSize = 0;
    int currentFileSize = 0;
    LPTSTR currentFileName = NULL;
    HANDLE createdFileHandle = NULL;

    if(argc != 2){
        _tprintf(":rptfilgen <next number of files to generate>");
        exit(EXIT_FAILURE);
    }
    if((numberOfFilesToGenerate = _tstoi(argv[1])) == 0){
        _tprintf("ERROR: Provide number instead of char");
        exit(EXIT_FAILURE);
    }
    hHeap=HeapCreate(HEAP_NO_SERIALIZE,0,0);
    if(NULL == hHeap){
        DisplayErrorBox(TEXT("HeapCreate"));
        exit(EXIT_FAILURE);
    }
    // _tprintf("Is current directory repeat")
    hFind = FindFirstFile(".\\*",&ffd);
    if(INVALID_HANDLE_VALUE == hFind){
        DisplayErrorBox("FindFirstFile");
        exit(EXIT_FAILURE);
    }
    do{
        if(!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && CheckExtensionC(ffd.cFileName)){
            FileArrSize++;
            currentFileSize =lstrlen(ffd.cFileName);
            FileNameArr = (LPSTR*)NullHeapRealloc(hHeap,FileNameArr,sizeof(LPTSTR)*(FileArrSize));
            FileNameArr[FileArrSize-1] = (LPTSTR)HeapAlloc(hHeap,HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY,
            sizeof(TCHAR)*lstrlen(ffd.cFileName));
            StringCchCopy(FileNameArr[FileArrSize-1],currentFileSize+1,
                          ffd.cFileName);
        }
    }while(FindNextFile(hFind,&ffd)!=0);

    FindClose(hFind);
    hFind = INVALID_HANDLE_VALUE;
    
    for(k=0;k<FileArrSize;++k){
        currentFileName = FileNameArr[k];
        if(GetFileNamePrefix(currentFileName,currentFileNamePrefix) == TRUE){
            for(j=k+1;j<FileArrSize;j++){
                if(GetFileNamePrefix(FileNameArr[j],szFileNamePrefix) == TRUE)
                {
                    if(CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                        NORM_LINGUISTIC_CASING,
                        szFileNamePrefix,lstrlen(szFileNamePrefix),
                        currentFileNamePrefix,
                        lstrlen(currentFileNamePrefix))){
                        repeated_file_present = TRUE;
                        break;
                    }
                }
            }
        }
    }
    //Now currentFileNamePrefix has character part we want the files to be generated
    if(TRUE == repeated_file_present){
        for(k=0;k<FileArrSize;++k){
            currentFileName = FileNameArr[k];
            if(GetFileNamePrefix(currentFileName,szFileNamePrefix) == TRUE && 
                CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                        NORM_LINGUISTIC_CASING,
                        szFileNamePrefix,lstrlen(szFileNamePrefix),
                        currentFileNamePrefix,
                        lstrlen(currentFileNamePrefix))){
                StringCchCopy(szMaxNumber,MAX_DIGITS_IN_NUMBER+1,
                        &currentFileName[lstrlen(currentFileNamePrefix)]);
                FileNumber=_tstoi(szMaxNumber);
                if(FileNumber>maxFileNumber){
                    maxFileNumber=FileNumber;
                }
            }
        }
    }

    if(NULL != FileNameArr){
        for(i=0;i<FileArrSize;++i){
            if(NULL != FileNameArr[i]){
                HeapFree(hHeap,HEAP_NO_SERIALIZE,FileNameArr[i]);
                FileNameArr[i] = NULL;
            }
        }
        HeapFree(hHeap,HEAP_NO_SERIALIZE,FileNameArr);
        FileNameArr = NULL;
    }
    if(NULL != hHeap){
        HeapDestroy(hHeap);
        hHeap = INVALID_HANDLE_VALUE;
    }
    for(i=0;i<numberOfFilesToGenerate;++i){
        StringCchPrintf(formattedFileName,MAX_PATH,"%s%03d.c",currentFileNamePrefix,(maxFileNumber+1)+i);
        createdFileHandle = CreateFile(formattedFileName,
                                        GENERIC_READ|GENERIC_WRITE,
                                        FILE_SHARE_READ,
                                        NULL,
                                        CREATE_NEW,
                                        FILE_ATTRIBUTE_NORMAL,
                                        NULL);
        if(INVALID_HANDLE_VALUE == createdFileHandle){
            DisplayErrorBox("CreateFile");
            exit(EXIT_FAILURE);
        }
    }
    return 0;
}

BOOL CheckExtensionC(LPTSTR lpszFileName){
    int i =0,fileSize=0;
    #define EXTENSION_LENGTH 2
    #define EXTENSION (TEXT(".c"))
    if(!lpszFileName)
        return FALSE;
    fileSize = lstrlen(lpszFileName);
    if(CSTR_EQUAL == CompareString(LOCALE_SYSTEM_DEFAULT,
                      LINGUISTIC_IGNORECASE,
                      EXTENSION,EXTENSION_LENGTH,
                      &lpszFileName[fileSize-EXTENSION_LENGTH],
                      EXTENSION_LENGTH))
        return TRUE;
    return FALSE;
    #undef EXTENSION_LENGTH
}

void DisplayErrorBox(LPTSTR lpszFunctionName){
    DWORD lastErr = GetLastError();
    LPTSTR lpBuffer;
    LPTSTR lpDisplayBuf;

    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                  FORMAT_MESSAGE_FROM_SYSTEM |
                  FORMAT_MESSAGE_IGNORE_INSERTS,
                  NULL,lastErr,0,(LPTSTR)&lpBuffer,
                  0,NULL);
    lpDisplayBuf = (LPTSTR)LocalAlloc(LMEM_ZEROINIT,
                                      lstrlen(lpBuffer)+(lstrlen(lpszFunctionName)+64)*sizeof(TCHAR));
    if(NULL == lpDisplayBuf){
        puts("ERROR IN LocalAlloc");
        exit(EXIT_FAILURE);
    }
    StringCchPrintf(lpDisplayBuf,LocalSize(lpDisplayBuf)/sizeof(TCHAR),
                    TEXT("%s failed with error %d:%s"),lpszFunctionName,lastErr,lpBuffer);
    MessageBox(NULL,lpDisplayBuf,TEXT("ERROR"),MB_OK);

    if(NULL != lpDisplayBuf)
        LocalFree(lpDisplayBuf);
    if(NULL != lpBuffer)
        LocalFree(lpBuffer);

}

BOOL GetFileNamePrefix(const LPSTR FileName,TCHAR* fileNamePrefix){
    int i=0;
    for(i=0;!_istdigit(FileName[i]) && FileName[i]!=(TCHAR)0;++i){}
    if(FileName[i] == (TCHAR)0){
        return(FALSE);
    }
    StringCchCopy(fileNamePrefix,i+1,FileName);
    return(TRUE);
}

LPVOID NullHeapRealloc(HANDLE hHeap,LPVOID lpMem,SIZE_T dwBytes){
    LPVOID pMem = NULL;
    if(NULL == lpMem){
        pMem = HeapAlloc(hHeap,HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY,
                                dwBytes);
        if(NULL == pMem){
            DisplayErrorBox(TEXT("HeapAlloc"));
            exit(EXIT_FAILURE);
        }
        return(pMem);
    }
    pMem = HeapReAlloc(hHeap,HEAP_NO_SERIALIZE|HEAP_ZERO_MEMORY,
                        lpMem,dwBytes);
    if(NULL == pMem){
        DisplayErrorBox(TEXT("HeapReAlloc"));
        exit(EXIT_FAILURE);
    }
    return(pMem);   
}
/**
 * Repetithon file generator
 * Above program creates files for repetithon only and only when we have files present 
 * in current directory with format <filename>000.c 
 * Syntax:
 *      rptfilgen.exe <number of files to create>
 * 
 * limitations:
 *      1.) Current Directory needs atleast 2 files same as the above format
 *      2.) Also it creates repeated files only for first occured file format(<filename>000.c)
 *      3.) It may have more limitations
 */
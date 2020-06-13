#include <stdio.h>
#include <string.h>
#include "curl/curl.h"
#define ERROR_NONE CURLE_OK
#define PX_ERROR -1
#define LW_NULL nullptr;
/***************************************************************************
** 函数名称: write_data
** 功能描述: 回调函数
** 输  入  : ptr		写数据指针
**           size	写入块字节数
**           nmemb  	写数据块数
**           stream 	目标文件指针
** 输  出  : 写入块数
** 返  回  : written
***************************************************************************/
size_t  write_data (void  *ptr, size_t  size, size_t  nmemb, FILE  *stream)
{
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}
/***************************************************************************
** 函数名称: fileDownload
** 功能描述: 下载函数
** 输  入  : pcAddressUrl	下载地址
**           pcFileName  	下载文件名
** 输  出  : NONE
** 返  回  : ERROR_NONE
***************************************************************************/
int fileDownload (const char  *pcAddressUrl, const char  *pcFileName)
{
    CURLcode    curlRet;

    CURL        *curl 	= LW_NULL;
    FILE        *fp      	= LW_NULL;

    const char        *url     	= pcAddressUrl;                    /*  下载网址                    */
    const char        *filename	= pcFileName;                      /*  下载文件名字                */

    curl = curl_easy_init();                                       /*  初始化 curl 会话            */
    if (!curl) {
        printf("curl_easy_init failed\n");
        return  (PX_ERROR);
    }

    fp = fopen(filename, "w+");                                    /*  打开文件                    */
    if (!fp) {
        printf("fopen failed\n");
        return  (PX_ERROR);
    }

    curlRet = curl_easy_setopt(curl, CURLOPT_URL, url);            /*  设置 url                    */
    if (curlRet != CURLE_OK) {
        fclose(fp);
        curl_easy_cleanup(curl);
        return  (PX_ERROR);
    }

    curlRet = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);/*  设置回调函数                */
    if (curlRet != CURLE_OK) {
        fclose(fp);
        curl_easy_cleanup(curl);
        return  (PX_ERROR);
    }

    curlRet = curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);      /*  设置写入文件句柄             */
    if (curlRet != CURLE_OK) {
        fclose(fp);
        curl_easy_cleanup(curl);
        return  (PX_ERROR);
    }

    curlRet = curl_easy_perform(curl);                            /*  完成传输函数                */
    if (curlRet != CURLE_OK) {
        fclose(fp);
        curl_easy_cleanup(curl);
        return  (PX_ERROR);
    }

    fclose(fp);
    curl_easy_cleanup(curl);                  		         /*  释放内存                    */

    return  (ERROR_NONE);
}
/***************************************************************************
** 函数名称: main
** 功能描述: 进程主函数
** 输  入  : argc    入参个数
**           argv    入参数组
** 输  出  : NONE
** 返  回  : ERROR_CODE
***************************************************************************/
int  main ()
{
    int  iRet = -1;

    printf("start download...\n");

    /*
     *  下载文件
     *  UPDATE_URL       下载地址
     *  UPDATE_FILE_NAME 下载保存文件名
     */
    iRet = fileDownload("https://download.microsoft.com/download/4/2/2/42245968-6A79-4DA7-A5FB-08C0AD0AE661/windowssdk/winsdksetup.exe", "a.zip");
    if (iRet != ERROR_NONE) {
        printf("file down failed\n");
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}

/********************************** (C) COPYRIGHT *******************************
* File Name          : Udisk_Func_BasicOp.c
* Author             : WCH
* Version            : V1.0.0
* Date               : 2022/11/22
* Description        : USB full-speed port host operation functions.
*********************************************************************************
* Copyright (c) 2021 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for 
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

/*******************************************************************************/
/* Header File */
#include "Udisk_Operation.h"\

/*******************************************************************************/
/* Variable Definition */
__attribute__((aligned(4)))  uint8_t  MY_DATA_BUF[ DISK_BASE_BUF_LEN ];   /* MY_DATA_BUF指向外部RAM的磁盘数据缓冲区,缓冲区长度为至少一个扇区的长度,用于用户数据缓存 */
uint8_t  *pCodeStr;

/*********************************************************************
 * @fn      UDisk_USBH_ByteOperation
 *
 * @brief   Demo Function For UDisk File Byte-operation
 *          including Create\Modify\Read\Erase (EXAM1)
 *
 * @return  none
 */
void UDisk_USBH_ByteOperation( void )
{
    uint8_t  c, s, i;
    uint16_t TotalCount = 0;

   UDisk_USBH_DiskReady( );
    if( ( CH103DiskStatus >= DISK_MOUNTED )&&( UDisk_Opeation_Flag == 1 ) )
    {
        UDisk_Opeation_Flag = 0;
        printf("CH103DiskStatus:%02x\r\n", CH103DiskStatus);
        /* 读文件 */
        strcpy((char *)mCmdParam.Open.mPathName, "/NEWFILE.C");         //设置将要操作的文件路径和文件名/NEWFILE.C
        s = CH103FileOpen();                                               //打开文件
        if(s == ERR_MISS_DIR || s == ERR_MISS_FILE)                        //没有找到文件
        {
            //创建文件演示
            printf("Find No File And Create\r\n");
            strcpy((char *)mCmdParam.Create.mPathName, "/NEWFILE.C");       //新文件名,在根目录下,中文文件名
            s = CH103FileCreate();                                         //新建文件并打开,如果文件已经存在则先删除后再新建
            mStopIfError(s);
            printf("ByteWrite\r\n");
            //实际应该判断写数据长度和定义缓冲区长度是否相符，如果大于缓冲区长度则需要多次写入
            i = sprintf((char *)Com_Buffer, "Note: \xd\xa这个程序是以字节为单位进行U盘文件读写,简单演示功能。\xd\xa"); //演示
            for(c = 0; c < 10; c++)
            {
                mCmdParam.ByteWrite.mByteCount = i;                          //指定本次写入的字节数
                mCmdParam.ByteWrite.mByteBuffer = Com_Buffer;                       //指向缓冲区
                s = CH103ByteWrite();                                        //以字节为单位向文件写入数据
                mStopIfError(s);
                printf("WriteIn Success %02X times\r\n", (uint16_t)c);
            }
            //演示修改文件属性
            printf("Modify\r\n");
            mCmdParam.Modify.mFileAttr = 0xff;                               //输入参数: 新的文件属性,为0FFH则不修改
            mCmdParam.Modify.mFileTime = 0xffff;                             //输入参数: 新的文件时间,为0FFFFH则不修改,使用新建文件产生的默认时间
            mCmdParam.Modify.mFileDate = MAKE_FILE_DATE(2015, 5, 18);        //输入参数: 新的文件日期: 2015.05.18
            mCmdParam.Modify.mFileSize = 0xffffffff;                         // 输入参数: 新的文件长度,以字节为单位写文件应该由程序库关闭文件时自动更新长度,所以此处不修改
            i = CH103FileModify();                                           //修改当前文件的信息,修改日期
            mStopIfError(i);
            printf("Close\r\n");
            mCmdParam.Close.mUpdateLen = 1;                                  //自动计算文件长度,以字节为单位写文件,建议让程序库关闭文件以便自动更新文件长度
            i = CH103FileClose();
            mStopIfError(i);

            /* 删除某个文件，文件名OLD.C， 需要提前放置在U盘根目录下 */
            printf("Erase\r\n");
            strcpy((char *)mCmdParam.Create.mPathName, "/OLD.C");            //需要删除的文件名，在根目录下
            i = CH103FileErase();                                            //删除文件并关闭
            if(i != ERR_SUCCESS)
            {
                printf("Error: %02X\r\n", (uint16_t)i);                      //显示错误信息，可能是没有找到文件
                if(i == ERR_MISS_FILE)
                {
                    printf("Didn't file the file\r\n");
                }
            }
            else
            {
                printf("Erase Over\r\n");
            }
        }
        else
        {
            //如果希望将新数据添加到原文件的尾部,可以移动文件指针
            //写入文件
            printf("ByteWrite\r\n");
            mCmdParam.ByteLocate.mByteOffset = 0xffffffff;                   //移到文件的尾部
            CH103ByteLocate();
            //实际应该判断写数据长度和定义缓冲区长度是否相符，如果大于缓冲区长度则需要多次写入
            i = sprintf((char *)Com_Buffer, "Note: \xd\xa这个程序是以字节为单位进行U盘文件读写,简单演示功能。\xd\xa");
            for(c = 0; c < 10; c++)
            {
                mCmdParam.ByteWrite.mByteCount = i;                          //指定本次写入的字节数
                mCmdParam.ByteWrite.mByteBuffer = Com_Buffer;                       //指向缓冲区
                s = CH103ByteWrite();                                        //以字节为单位向文件写入数据
                mStopIfError(s);
                printf("WriteIn Success %02X times\r\n", (uint16_t)c);
            }
						
						//演示修改文件属性
            printf("Modify\r\n");
            mCmdParam.Modify.mFileAttr = 0xff;                               //输入参数: 新的文件属性,为0FFH则不修改
            mCmdParam.Modify.mFileTime = 0xffff;                             //输入参数: 新的文件时间,为0FFFFH则不修改,使用新建文件产生的默认时间
            mCmdParam.Modify.mFileDate = MAKE_FILE_DATE(2015, 5, 18);        //输入参数: 新的文件日期: 2015.05.18
            mCmdParam.Modify.mFileSize = 0xffffffff;                         // 输入参数: 新的文件长度,以字节为单位写文件应该由程序库关闭文件时自动更新长度,所以此处不修改
            i = CH103FileModify();                                           //修改当前文件的信息,修改日期
            mStopIfError(i);

            //读取文件前N字节
            TotalCount = 60;                                                 //设置准备读取总长度100字节
            printf("First %d characters read out:\r\n", TotalCount);
            while(TotalCount)                                                //如果文件比较大,一次读不完,可以再调用CH103ByteRead继续读取,文件指针自动向后移动
            {
                if(TotalCount > (MAX_PATH_LEN - 1))
                {
                    c = MAX_PATH_LEN - 1;                                    //剩余数据较多,限制单次读写的长度不能超过 sizeof( mCmdParam.Other.mBuffer ) */
                }
                else
                {
                    c = TotalCount;                                          //最后剩余的字节数
                }
                mCmdParam.ByteRead.mByteCount = c;                           //请求读出几十字节数据
                mCmdParam.ByteRead.mByteBuffer = &Com_Buffer[0];
                s = CH103ByteRead();                                         //以字节为单位读取数据块,单次读写的长度不能超过MAX_BYTE_IO,第二次调用时接着刚才的向后读
                TotalCount -= mCmdParam.ByteRead.mByteCount;                 //计数,减去当前实际已经读出的字符数
                for(i = 0; i != mCmdParam.ByteRead.mByteCount; i++)
                {
                    printf("%c", mCmdParam.ByteRead.mByteBuffer[i]);         //显示读出的字符
                }
                if(mCmdParam.ByteRead.mByteCount < c)                        //实际读出的字符数少于要求读出的字符数,说明已经到文件的结尾
                {
                    printf("\r\n");
                    printf("File end\r\n");
                    break;
                }
            }
            printf("Close\r\n");
            i = CH103FileClose();                                            //关闭文件
            mStopIfError(i);
        }
    }
}

/*********************************************************************
 * @fn      UDisk_USBH_SectorOperation
 *
 * @brief   Demo Function For UDisk File Sector-operation
 *          including Create\Modify\Read\Erase (EXAM6)
 *
 * @return  none
 */
void UDisk_USBH_SectorOperation( void )
{
    uint8_t  SecCount, s, i;
    uint16_t tmp;
    uint8_t  tmpbuf[64];

    UDisk_USBH_DiskReady( );
    if( ( CH103DiskStatus >= DISK_MOUNTED )&&( UDisk_Opeation_Flag == 1 ) )
    {
        UDisk_Opeation_Flag = 0;

#if DISK_BASE_BUF_LEN
        if(DISK_BASE_BUF_LEN < CH103vSectorSize)
        {
            /* 检查磁盘数据缓冲区是否足够大,CHRV3vSectorSize是U盘的实际扇区大小 */
            printf("Too large sector size\n");
            return;
        }
#endif
        /* 读取原文件 */
        printf("Open\r\n");
        strcpy((char *)mCmdParam.Open.mPathName, "/C51/CHRV3HFT.C");    //文件名,该文件在C51子目录下
        s = CH103FileOpen();                                           //打开文件
        if(s == ERR_MISS_DIR || s == ERR_MISS_FILE)                    //没有找到文件
        {
            printf("没有找到文件\r\n");
        }
        else                                                           //找到文件或者出错
        {
            printf("Query\r\n");
            i = CH103FileQuery();                                      //查询当前文件的信息
            mStopIfError(i);
            printf("Read\r\n");
            CH103vFileSize = CH103vFileSize + (sizeof(MY_DATA_BUF) - 1); //原文件的长度
            SecCount = CH103vFileSize / sizeof(MY_DATA_BUF);          //计算文件的扇区数,因为读写是以扇区为单位的,先加CHRV3vSectorSize-1是为了读出文件尾部不足1个扇区的部分
            printf("Size=%ld, Sec=%d\r\n", (long)CH103vFileSize, (uint16_t)SecCount);
            while(SecCount--)
            {
                mCmdParam.Read.mSectorCount = sizeof(MY_DATA_BUF) / 512; //读取全部数据,如果超过2个扇区则只读取2个扇区
                mCmdParam.Read.mDataBuffer = &MY_DATA_BUF[0];            //指向文件数据缓冲区的起始地址
                i = CH103FileRead();                                     //从文件读取数据
                mStopIfError(i);
                if(SecCount == 0) break;
//                for(tmp=0; tmp<sizeof( MY_DATA_BUF ); tmp++)
//                {
//                    printf("%02X ",(uint16_t)MY_DATA_BUF[tmp]);
//                }
//                printf("\n");
            }
            tmp = (CH103vFileSize - (sizeof(MY_DATA_BUF) - 1)) % sizeof(MY_DATA_BUF);
            if((tmp == 0) && (CH103vFileSize != 0)) tmp = sizeof(MY_DATA_BUF);
            CH103vFileSize = CH103vFileSize - (sizeof(MY_DATA_BUF) - 1);  //恢复原文件的长度
//            for(i=0; i<tmp; i++)
//            {
//                printf("%02X ",(uint16_t)MY_DATA_BUF[i]);
//            }
//            printf("\n");

//            //如果文件比较大,一次读不完,可以再调用CHRV3FileRead继续读取,文件指针自动向后移动
//            while ( 1 )
//            {
//                c = 4;   //每次读取4个扇区,缓冲区定义的越大，一次读取的扇区数越多
//                mCmdParam.Read.mSectorCount = c;   //指定读取的扇区数
//                mCmdParam.Read.mDataBuffer = &MY_DATA_BUF[0];  //指向文件数据缓冲区的起始地址
//                CH103FileRead( );   //读完后文件指针自动后移
//                //处理数据
//                if ( mCmdParam.Read.mSectorCount < c ) break;   //实际读出的扇区数较小则说明文件已经结束
//            }
//            //如果希望从指定位置开始读写,可以移动文件指针
//            mCmdParam.Locate.mSectorOffset = 3;  //跳过文件的前3个扇区开始读写
//            i = CH103FileLocate( );
//            mCmdParam.Read.mSectorCount = 10;
//            mCmdParam.Read.mDataBuffer = &MY_DATA_BUF[0];  //指向文件数据缓冲区的起始地址
//            CH103FileRead( );  // 直接读取从文件的第(CHRV3vSectorSizeH*256*3)个字节开始的数据,前3个扇区被跳过
//            //如果希望将新数据添加到原文件的尾部,可以移动文件指针
//            i = CH103FileOpen( );
//            mCmdParam.Locate.mSectorOffset = 0xffffffff;  //移到文件的尾部,以扇区为单位,如果原文件是3字节,则从CHRV3vSectorSizeH个字节处开始添加
//            i = CH103FileLocate( );
//            mCmdParam.Write.mSectorCount = 10;
//            mCmdParam.Write.mDataBuffer = &MY_DATA_BUF[0];
//            CH103FileWrite( );   //在原文件的后面添加数据
//            //使用CHRV3FileRead可以自行定义数据缓冲区的起始地址
//            mCmdParam.Read.mSectorCount = 2;
//            mCmdParam.Read.mDataBuffer = 0x50;  //将读出的数据放到50H开始的缓冲区中，需要指定缓冲区的起始地址
//            CH103FileRead( );   //从文件中读取2个扇区到指定缓冲区
//            //使用CHRV3FileWrite可以自行定义数据缓冲区的起始地址
//            mCmdParam.Write.mSectorCount = 2;
//            mCmdParam.Write.mDataBuffer = 0x50;  //将50H开始的缓冲区中的数据写入
//            CH103FileWrite( );   //将指定缓冲区中的数据写入2个扇区到文件中

            printf("Close\r\n");
            i = CH103FileClose();                                        //关闭文件
            mStopIfError(i);
        }
        printf("Create\r\n");
        strcpy((char *)mCmdParam.Create.mPathName, "/NEWFILE.TXT");       //新文件名,在根目录下,中文文件名
        s = CH103FileCreate();                                           //新建文件并打开,如果文件已经存在则先删除后再新建 */
        mStopIfError(s);
        printf("Write\r\n");
        strcpy((char *)tmpbuf, "000ABCDEFGHIJKLMNOPQRSTUVWXYZ\xd\xa");    //准备写文件数据
        for(i = 0; i < (DISK_BASE_BUF_LEN / sizeof(tmpbuf)); i++)
        {
            tmp = i * sizeof(tmpbuf);
            strcpy((char *)&MY_DATA_BUF[tmp], (char *)tmpbuf);
        }
        for(tmp = 0; tmp < sizeof(MY_DATA_BUF); tmp++)
        {
            printf("%02X", (uint16_t)MY_DATA_BUF[tmp]);
        }
        printf("\r\n");
        for(s = 0; s < 10; s++)
        {
            mCmdParam.Write.mSectorCount = 1;                 //写入所有扇区的数据
            mCmdParam.Write.mDataBuffer = &MY_DATA_BUF[0];    //指向文件数据缓冲区的起始地址
            i = CH103FileWrite();                             //向文件写入数据
            mStopIfError(i);
            printf("成功写入 %02X次\r\n", (uint16_t)s);
        }
//        printf( "Modify\n" );
//        mCmdParam.Modify.mFileAttr = 0xff;                           //输入参数: 新的文件属性,为0FFH则不修改
//        mCmdParam.Modify.mFileTime = 0xffff;                         //输入参数: 新的文件时间,为0FFFFH则不修改,使用新建文件产生的默认时间
//        mCmdParam.Modify.mFileDate = MAKE_FILE_DATE( 2015, 5, 18 );  //输入参数: 新的文件日期: 2015.05.18
//        mCmdParam.Modify.mFileSize = 0xffffffff;                     //输入参数: 新的文件长度,以字节为单位写文件应该由程序库关闭文件时自动更新长度,所以此处不修改
//        i = CH103FileModify( );                                      //修改当前文件的信息,修改日期
//        mStopIfError( i );
        printf("Close\r\n");
        mCmdParam.Close.mUpdateLen = 1;                                    //自动计算文件长度,以字节为单位写文件,建议让程序库关闭文件以便自动更新文件长度
        i = CH103FileClose();
//        mStopIfError(i);
//        /* Delete a file，An OLD.C file needs to be prepared in advance in the root directory of the USB Disk. */
//        /* 删除某个文件，文件名OLD.C， 需要提前放置在U盘根目录下 */
//        printf( "Erase\r\n" );
//        strcpy( (char *)mCmdParam.Create.mPathName, "/OLD.C" );       //file to be delated, under root directory.//需要删除的文件名，在根目录下
//        i = CH103FileErase( );                                       //delate a file and close。//删除文件并关闭
//        if ( i != ERR_SUCCESS )
//        {
//            printf( "Error: %02X\r\n", (uint16_t)i );                  //Display ERR Info , can be not found a file.//显示错误信息，可能是没有找到文件
//            if( i == ERR_MISS_FILE )
//            {
//                printf( "Didn't file the file\r\n" );
//            }
//        }
//        else
//        {
//            printf( "Erase Over\r\n" );
//        }
    }
}


/*********************************************************************
 * @fn      UDisk_USBH_EnumFiles
 *
 * @brief   Demo Function For Enumerating files in UDisk(EXAM11)
 *
 * @return  none
 */
void UDisk_USBH_EnumFiles( void )
{
    uint8_t  i, s;
    uint16_t j;

    UDisk_USBH_DiskReady( );
    if( ( CH103DiskStatus >= DISK_MOUNTED )&&( UDisk_Opeation_Flag == 1 ) )
    {
        UDisk_Opeation_Flag = 0;
        /* 读取原文件 */
        printf( "Open\r\n" );
        strcpy( (char *)mCmdParam.Open.mPathName, "/C51/CHRV3HFT.C" );//文件名,该文件在C51子目录下
        s = CH103FileOpen( );                        //打开文件
        /* 列出文件 */
        if ( s == ERR_MISS_DIR )
        {
            printf("不存在该文件则列出所有文件\r\n");  //C51子目录不存在则列出根目录下的所有文件
            pCodeStr = (uint8_t *)"/*";
        }
        else
        {
            pCodeStr = (uint8_t *)"/C51/*";                     //CHRV3HFT.C文件不存在则列出\C51子目录下的以CHRV3开头的文件
        }
        printf( "List file %s\r\n", pCodeStr );
        for ( j = 0; j < 10000; j ++ )               //最多搜索前10000个文件,实际上没有限制
        {
            strcpy( (char *)mCmdParam.Open.mPathName, (char *)pCodeStr );//搜索文件名,*为通配符,适用于所有文件或者子目录
            i = strlen( (char *)mCmdParam.Open.mPathName );
            mCmdParam.Open.mPathName[ i ] = 0xFF;    //根据字符串长度将结束符替换为搜索的序号,从0到254,如果是0xFF即255则说明搜索序号在CHRV3vFileSize变量中
            CH103vFileSize = j;                      //指定搜索/枚举的序号
            i = CH103FileOpen( );                    //打开文件,如果文件名中含有通配符*,则为搜索文件而不打开
            /* CHRV3FileEnum 与 CHRV3FileOpen 的唯一区别是当后者返回ERR_FOUND_NAME时那么对应于前者返回ERR_SUCCESS */
            if ( i == ERR_MISS_FILE )
            {
                break;                                //再也搜索不到匹配的文件,已经没有匹配的文件名
            }
            if ( i == ERR_FOUND_NAME )
            {
                /* 搜索到与通配符相匹配的文件名,文件名及其完整路径在命令缓冲区中 */
                printf( "  match file %04d#: %s\r\n", (unsigned int)j, mCmdParam.Open.mPathName );//显示序号和搜索到的匹配文件名或者子目录名
                continue;                             //继续搜索下一个匹配的文件名,下次搜索时序号会加1
            }
            else
            {
                /* 出错 */
                mStopIfError( i );
                break;
            }
        }
        printf( "Close\r\n" );
        CH103FileClose( );                            //关闭文件
        printf( "U盘演示完成\r\n" );
    }
}

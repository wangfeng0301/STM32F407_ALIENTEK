#ifndef _SW_SHARE_H_
#define _SW_SHARE_H_



#define SW_HIP_BUFFER_OVERFLOW					0x6A00



#define SW_INS_P3_MISMATCH						0x6700	//P3�ĳ��Ȳ�ƥ��
#define SW_FS_BIN_LEN_OVER_RANGE				0x6700


#define SW_INS_P1P2_MISMATCH					0x6A86	// P1��P2����ȷ��֧��
#define SW_FS_BIN_OFFSET_OVER_RANGE			0x6B00
#define SW_IO_BUFFER_OVERFLOW           0x6A00
#define SW_DATA_LEN_MISMATCH_Le				0x6C00 	// ��Ӧ���ݳ�����SW2����ʾ
#define SW_INS_INSTRUCTION_NO_DEFINE			0x6d00  // ��֧�ֵ�ָ��,���ڸ��������ڽ�ֹ��ָ��

#define SW_SUCCESS							0x9000



typedef unsigned char BYTE;
typedef union
{
  unsigned short Word;
  struct
  {
#ifdef CPU_BIG_ENDIAN  
    BYTE sw2;
    BYTE sw1;
#else
		BYTE sw1;
    BYTE sw2;
#endif
  }Byte;
}StatusWordStruct;


extern StatusWordStruct gStatusWord;

#define gSW  (gStatusWord.Word)
#define gSW1 (gStatusWord.Byte.sw1)
#define gSW2 (gStatusWord.Byte.sw2)

#define WARNING(ErrSW) do{\
    gSW = ErrSW;\
    return FALSE;					\
  }while(0)
#define INS_WARNING(ErrSW) do{\
    gSW = ErrSW;\
    return;\
  }while(0)
#define SET_SW(ErrSW) do{ \
	  gSW = ErrSW;\
	}while(0)

#define goto_lblError	do{\
	goto lblError; \
}while(0)
#define goto_lblException	do{\
	goto lblException; \
}while(0)

#define FatalError()
#endif

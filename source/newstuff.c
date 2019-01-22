#include "newstuff.h"
#include "csvc.h"

Result AM_GetTicketIdCount(u64 titleID, s32 *ticketIDcount)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x81D,2,0);
    cmdbuf[1] = titleID & 0xffffffff;
    cmdbuf[2] = (u32)(titleID >> 32);

    if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

    if(ticketIDcount) *ticketIDcount = (s32)cmdbuf[2];

    return (Result)cmdbuf[1];
}

Result AM_GetTicketIdList(s32 *outticketIDlength, u64 *ticketIDlist, s32 ticketIDlength, u64 titleID, bool unsure)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0] = IPC_MakeHeader(0x81E,4,2);
    cmdbuf[1] = ticketIDlength;
    cmdbuf[2] = titleID & 0xffffffff;
    cmdbuf[3] = (u32)(titleID >> 32);
    cmdbuf[4] = unsure ? 1 : 0;
    cmdbuf[5] = IPC_Desc_Buffer(sizeof(u64)*ticketIDlength,IPC_BUFFER_W);
    cmdbuf[6] = (u32)ticketIDlist;

    if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

    if(outticketIDlength) *outticketIDlength = (s32)cmdbuf[2];

    return (Result)cmdbuf[1];
}

Result AM_ExportTicketWrapped(size_t* CryptedTicketBufferLengthOutput, void *CryptedTicketBuffer, size_t CryptedTicketBufferLength, size_t *WrappedKeydataLengthOutput, void *WrappedKeydata, u64 titleID, u64 ticketID)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0]  = IPC_MakeHeader(0x829,6,4); //0x08290184
    cmdbuf[1]  = CryptedTicketBufferLength;
    cmdbuf[2]  = 256;
    cmdbuf[3]  = titleID & 0xffffffff;
    cmdbuf[4]  = (u32)(titleID >> 32);
    cmdbuf[5]  = ticketID & 0xffffffff;
    cmdbuf[6]  = (u32)(ticketID >> 32);
    cmdbuf[7]  = IPC_Desc_Buffer(CryptedTicketBufferLength,IPC_BUFFER_W);
    cmdbuf[8]  = (u32)CryptedTicketBuffer;
    cmdbuf[9]  = IPC_Desc_Buffer(256,IPC_BUFFER_W);
    cmdbuf[10] = (u32)WrappedKeydata;

    if(R_FAILED(ret = svcSendSyncRequest(*amGetSessionHandle()))) return ret;

    if(CryptedTicketBufferLengthOutput) *CryptedTicketBufferLengthOutput = (size_t)cmdbuf[2];
    if(WrappedKeydataLengthOutput) *WrappedKeydataLengthOutput = (size_t)cmdbuf[3];

    return (Result)cmdbuf[1];
}

//cheating ctrulib since doesnt gives us a ampxiGetSessionHandle, yet
static Handle ampxiHandle;
static int ampxiRefCount;

Result myampxiInit(void)
{
    Result res=0;
    if (AtomicPostIncrement(&ampxiRefCount)) return 0;
    res = svcControlService(SERVICEOP_STEAL_CLIENT_SESSION, (Handle *)&ampxiHandle, (const char *)"pxi:am9");
    if (R_FAILED(res)) AtomicDecrement(&ampxiRefCount);
    return res;
}

void myampxiExit(void)
{
    if (AtomicDecrement(&ampxiRefCount)) return;
    svcCloseHandle(ampxiHandle);
}

Result AMPXI_ExportTicketWrapped(size_t* CryptedTicketBufferLengthOutput, void *CryptedTicketBuffer, size_t CryptedTicketBufferLength, size_t *WrappedKeydataLengthOutput, void *WrappedKeydata, u64 titleID, u64 ticketID)
{
    Result ret = 0;
    u32 *cmdbuf = getThreadCommandBuffer();

    cmdbuf[0]  = IPC_MakeHeader(0x6D,6,4); //0x006D0184
    cmdbuf[1]  = CryptedTicketBufferLength;
    cmdbuf[2]  = 256;
    cmdbuf[3]  = titleID & 0xffffffff;
    cmdbuf[4]  = (u32)(titleID >> 32);
    cmdbuf[5]  = ticketID & 0xffffffff;
    cmdbuf[6]  = (u32)(ticketID >> 32);
    cmdbuf[7]  = IPC_Desc_PXIBuffer(CryptedTicketBufferLength, 0, false);
    cmdbuf[8]  = (u32)CryptedTicketBuffer;
    cmdbuf[9]  = IPC_Desc_PXIBuffer(256, 1, false);
    cmdbuf[10] = (u32)WrappedKeydata;

    if(R_FAILED(ret = svcSendSyncRequest(ampxiHandle))) return ret;

    if(CryptedTicketBufferLengthOutput) *CryptedTicketBufferLengthOutput = (size_t)cmdbuf[2];
    if(WrappedKeydataLengthOutput) *WrappedKeydataLengthOutput = (size_t)cmdbuf[3];

    return (Result)cmdbuf[1];
}
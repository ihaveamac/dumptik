#include <3ds.h>

Result AM_GetTicketIdCount(u64 titleID, s32 *ticketIDcount);
Result AM_GetTicketIdList(s32 *outticketIDlength, u64 *ticketIDlist, s32 ticketIDlength, u64 titleID, bool unsure);
Result AM_ExportTicketWrapped(size_t* CryptedTicketBufferLengthOutput, void *CryptedTicketBuffer, size_t CryptedTicketBufferLength, size_t *WrappedKeydataLengthOutput, void *WrappedKeydata, u64 titleID, u64 ticketID);

Result myampxiInit(void);
void myampxiExit(void);
Result AMPXI_ExportTicketWrapped(size_t* CryptedTicketBufferLengthOutput, void *CryptedTicketBuffer, size_t CryptedTicketBufferLength, size_t *WrappedKeydataLengthOutput, void *WrappedKeydata, u64 titleID, u64 ticketID);

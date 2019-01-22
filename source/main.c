#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <3ds.h>

#include "aes.h"
#include "newstuff.h"

#define UNITINFO_DEBUG (*((vu8*)0x1FF80015) & BIT(0))

void logfmt(const char *format, ...) {
	char buf[512] = {0};
	va_list args;
	va_start(args, format);
	vsprintf(buf, format, args);
	va_end(args);

	fputs(buf, stdout);
	FILE *fp = fopen("/0_OUT.LOG", "a");
	if (fp) {
		fwrite(buf, 1, strlen(buf), fp);
		fclose(fp);
	}
}

void dump_tickets(void) {
	u8 empty[16] = {0};
	Result res;
	u32 i, j;

	u32 ticketsCount, ticketsRead;
	s32 ticketIDcount, ticketIDread;
	u64 *ticketTitleIDs;
	u64 *ticketIDs;

	u8 *tikbuf = malloc(0x2800);
	u8 *keybuf = malloc(256);
	size_t tikbuf_len;
	size_t keybuf_len;

	res = AM_GetTicketCount(&ticketsCount);

	ticketTitleIDs = malloc(ticketsCount * sizeof(u64));

	res = AM_GetTicketList(&ticketsRead, ticketsCount, 0, ticketTitleIDs);

	if (R_FAILED(res)) {
		logfmt("Getting ticket or title information.\nI don't think this should happen...\n  err: 0x%08lx\n", res);
		free(ticketTitleIDs);
		free(tikbuf);
		free(keybuf);
		return;
	}

	for (i = 0; i < ticketsRead; i++) {
		u64 titleID = ticketTitleIDs[i];
		logfmt("Title ID: %016llx\n", titleID);
		
		res = AM_GetTicketIdCount(titleID, &ticketIDcount);
		ticketIDs = malloc(ticketIDcount * sizeof(u64));
		res = AM_GetTicketIdList(&ticketIDread, ticketIDs, ticketIDcount, titleID, true);

		for (j = 0; j < ticketIDread; j++) {
			logfmt(" - Ticket ID: %016llx\n", ticketIDs[j]);
			res = AMPXI_ExportTicketWrapped(&tikbuf_len, tikbuf, 0x2800, &keybuf_len, keybuf, titleID, ticketIDs[j]);
			if (R_FAILED(res)) {
				logfmt("   Failed to export: 0x%08lx\n", res);
				continue;
			}

			if (tikbuf_len) {
				struct AES_ctx ctx;
				AES_init_ctx_iv(&ctx, empty, empty);
				AES_CBC_decrypt_buffer(&ctx, tikbuf, tikbuf_len);

				char namebuf[47] = {0};
				sprintf(namebuf, "/dumptik/%016llx.%016llx.tik", titleID, ticketIDs[j]);
				FILE *fp = fopen(namebuf, "wb");
				if (fp) {
					fwrite(tikbuf, 1, tikbuf_len, fp);
					fclose(fp);
				}
			}
		}

		logfmt("\n");
		free(ticketIDs);
	}

	free(tikbuf);
	free(keybuf);
	free(ticketTitleIDs);
}

bool isOn118(void) {
	Result res;
	u64 titleID[1] = {0x0004013000001502ULL};
	AM_TitleEntry amInfo;

	if (osGetKernelVersion() < SYSTEM_VERSION(2, 55, 0)) return false;

	res = AM_GetTitleInfo(MEDIATYPE_NAND, 1, titleID, &amInfo);
	if (amInfo.version < 10245) return false;

	return true;
}

int main(int argc, char* argv[])
{
	Result res;
	gfxInitDefault();
	amInit();
	consoleInit(GFX_TOP, NULL);

	res = myampxiInit();
	if (R_FAILED(res)) {
		logfmt("Failed to get AMPXI handle.\nAre you using a recent version of Luma3DS?\n");
	} else {
		if (isOn118()) {
			if (UNITINFO_DEBUG) {
				mkdir("/dumptik", 0777);
				dump_tickets();
			} else {
				puts("You must enable developer UNITINFO.");
			}
		} else {
			puts("You must be on 11.8.0 to use dumptik.");
		}
	}

	puts("\nPress START or B to exit.");

	// Main loop
	while (aptMainLoop())
	{
		gspWaitForVBlank();
		gfxSwapBuffers();
		hidScanInput();

		// Your code goes here
		u32 kDown = hidKeysDown();
		if (kDown & KEY_START || kDown & KEY_B)
			break; // break in order to return to hbmenu
	}

	amExit();
	gfxExit();
	return 0;
}

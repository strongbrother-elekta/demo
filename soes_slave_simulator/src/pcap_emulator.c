#ifdef _WIN32
#include <pcap.h>
#else
#include <pcap/pcap.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <pthread.h>
#include "esc_hal.h"

/* Simple pcap-based watcher: when it sees an EtherCAT frame (Ethertype 0x88A4)
 * it will call esc_hal_set_link(1). This is a lightweight helper to detect
 * TwinCAT probing traffic and bring the simulated link up.
 *
 * Build: link with -lpcap (Npcap on Windows). Run as admin.
 */

static void packet_handler(u_char *user, const struct pcap_pkthdr *h, const u_char *bytes)
{
    (void)user;
    if (h->caplen < 14) return; /* too short for Ethernet */
    uint16_t ethertype = (bytes[12] << 8) | bytes[13];
    if (ethertype != 0x88A4) return;

    /* EtherCAT frame seen */
    esc_hal_set_link(1);
    printf("[pcap_emulator] saw EtherCAT frame, len=%u\n", (unsigned)h->caplen);

    /* copy packet into reply buffer and process EtherCAT payload in-place */
    pcap_t *handle = (pcap_t *)user;
    u_char *reply = (u_char *)malloc(h->caplen);
    if (!reply) return;
    memcpy(reply, bytes, h->caplen);

    /* Verbose hexdump of received frame (first 128 bytes) */
    size_t dump_len = (h->caplen < 128) ? h->caplen : 128;
    printf("[pcap_emulator] rx hexdump (%zu bytes):\n", dump_len);
    for (size_t i = 0; i < dump_len; ++i) {
        printf("%02X%s", bytes[i], ((i + 1) % 16 == 0) ? "\n" : " ");
    }
    if (dump_len % 16) printf("\n");

    /* Process EtherCAT payload after Ethernet header (14 bytes) */
    if ((size_t)h->caplen > 14) {
        uint8_t *payload = reply + 14;
        size_t payload_len = h->caplen - 14;
        /* process datagrams and map to DPRAM */
        size_t processed = 0;
        while (processed + 10 <= payload_len) {
            uint8_t *dg = payload + processed;
            /* Simple header parsing (best-effort):
             * [0] = command
             * [1] = idx/reserved
             * [2..3] = address (little-endian)
             * [4..5] = length (little-endian)
             * [6..9] = reserved / mailbox / header extension
             * Data follows at offset 10
             */
            uint8_t cmd = dg[0];
            uint16_t addr = (uint16_t)(dg[2] | (dg[3] << 8));
            uint16_t len = (uint16_t)(dg[4] | (dg[5] << 8));
            size_t hdrlen = 10;
            if (processed + hdrlen + len > payload_len) break; /* malformed */

            uint8_t *data_ptr = dg + hdrlen;

            printf("[pcap_emulator] datagram at off=%zu cmd=0x%02X addr=0x%04X len=%u\n",
                   processed, cmd, addr, len);

            /* Handle common read/write commands (best-effort):
             * cmd 0x01 -> APRD (read)
             * cmd 0x02 -> APWR (write)
             * cmd 0x04 -> FPRD (fast read) treat as read
             * cmd 0x05 -> FPWR (fast write) treat as write
             */
            if (cmd == 0x01 || cmd == 0x04) {
                /* Read: fetch from DPRAM and place into response payload */
                if (len > 0) {
                    uint8_t *tmp = (uint8_t *)malloc(len);
                    if (tmp) {
                        esc_hal_read(addr, tmp, len);
                        memcpy(data_ptr, tmp, len);
                        free(tmp);
                        printf("[pcap_emulator] serviced READ addr=0x%04X len=%u\n", addr, len);
                    }
                }
            } else if (cmd == 0x02 || cmd == 0x05) {
                /* Write: copy into DPRAM */
                if (len > 0) {
                    esc_hal_write(addr, data_ptr, len);
                    printf("[pcap_emulator] serviced WRITE addr=0x%04X len=%u\n", addr, len);
                }
            } else {
                printf("[pcap_emulator] unknown cmd 0x%02X, skipping\n", cmd);
            }

            /* advance to next datagram; header+data aligned to 2 bytes
             * (EtherCAT datagrams are often padded); for simplicity advance by hdrlen+len
             */
            processed += hdrlen + len;
        }
    }

    /* swap MAC addresses in reply and send */
    for (int i = 0; i < 6; ++i) {
        uint8_t t = reply[i];
        reply[i] = reply[6 + i];
        reply[6 + i] = t;
    }
    if (handle != NULL) {
        if (pcap_sendpacket(handle, reply, (int)h->caplen) != 0) {
            fprintf(stderr, "[pcap_emulator] pcap_sendpacket failed: %s\n", pcap_geterr(handle));
        }
    }
    free(reply);
    return;
}

int esc_hal_start_pcap(const char *iface)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t *handle = pcap_open_live(iface, 65536, 1, 1000, errbuf);
    if (!handle) {
        fprintf(stderr, "pcap_open_live failed: %s\n", errbuf);
        return -1;
    }


    /* filter for EtherCAT ethertype 0x88A4 */
    struct bpf_program fp;
    if (pcap_compile(handle, &fp, "ether proto 0x88a4", 1, PCAP_NETMASK_UNKNOWN) == -1) {
        fprintf(stderr, "pcap_compile failed\n");
        pcap_close(handle);
        return -1;
    }
    if (pcap_setfilter(handle, &fp) == -1) {
        fprintf(stderr, "pcap_setfilter failed\n");
        pcap_freecode(&fp);
        pcap_close(handle);
        return -1;
    }
    pcap_freecode(&fp);

    printf("[pcap_emulator] listening on %s for EtherCAT frames\n", iface);
    /* Run capture loop (blocking) - pass handle as user so we can send replies */
    int ret = pcap_loop(handle, 0, packet_handler, (u_char *)handle);
    if (ret == -1) {
        fprintf(stderr, "pcap_loop error: %s\n", pcap_geterr(handle));
    }
    pcap_close(handle);
    return 0;
}

/* Thread function wrapper for async pcap listening */
static void* pcap_thread_func(void *arg)
{
    char *iface = (char *)arg;
    esc_hal_start_pcap(iface);
    free(iface);
    return NULL;
}

/* Async version: starts pcap listening in a background thread and returns immediately */
int esc_hal_start_pcap_async(const char *iface)
{
    if (!iface) {
        fprintf(stderr, "[pcap_emulator] esc_hal_start_pcap_async: iface is NULL\n");
        return -1;
    }

    pthread_t tid;
    char *iface_copy = (char *)malloc(strlen(iface) + 1);
    if (!iface_copy) {
        fprintf(stderr, "[pcap_emulator] malloc failed for iface copy\n");
        return -1;
    }
    strcpy(iface_copy, iface);

    int ret = pthread_create(&tid, NULL, pcap_thread_func, iface_copy);
    if (ret != 0) {
        fprintf(stderr, "[pcap_emulator] pthread_create failed: %d\n", ret);
        free(iface_copy);
        return -1;
    }

    /* Detach the thread so it runs independently */
    pthread_detach(tid);
    printf("[pcap_emulator] started async listener on %s (thread detached)\n", iface);
    return 0;
}

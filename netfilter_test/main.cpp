#include <cstdio>                   // printf()
#include <cstdlib>                  // exit()
#include <unistd.h>                 // uint
#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>        /* for NF_ACCEPT */
#include <cerrno>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <netinet/in_systm.h>
#include <libnet/libnet-macros.h>
#include <win32/config.h>           // #define LIBNET_LIL_ENDIAN 1
#include <libnet/libnet-headers.h>  /* for ip and tcp hdr */
#include <glog/logging.h>           // Glog

#define IPv4            4
#define PROTOCOL_TCP    6
#define PORT_HTTP       80

#define DUMP_FLAG       1
#define DUMP_SIZE       20

void dump(uint8_t *pkt, uint32_t total){
    int i;
    DLOG_EVERY_N(INFO, 1) << google::COUNTER << "/" << total << " HTTP blocked.";
    for(i=0; i<DUMP_SIZE; i++) {
        printf("%02X ", pkt[i]);
        if(i==DUMP_SIZE-1 || (i && i%16==0)) puts("");
    }
}

/* returns packet id */
static uint32_t print_pkt (struct nfq_data *tb, uint8_t *NF_FLAG)
{
    uint32_t id = 0;
    struct nfqnl_msg_packet_hdr *ph;
    struct nfqnl_msg_packet_hw *hwph;
    u_int32_t mark,ifi;
    int ret;
    unsigned char *data;

    ph = nfq_get_msg_packet_hdr(tb);
    if (ph) {
        id = ntohl(ph->packet_id);
        printf("hw_protocol=0x%04x hook=%u id=%u ",
            ntohs(ph->hw_protocol), ph->hook, id);
    }

    hwph = nfq_get_packet_hw(tb);
    if (hwph) {
        int i, hlen = ntohs(hwph->hw_addrlen);

        printf("hw_src_addr=");
        for (i = 0; i < hlen-1; i++)
            printf("%02x:", hwph->hw_addr[i]);
        printf("%02x ", hwph->hw_addr[hlen-1]);
    }

    mark = nfq_get_nfmark(tb);
    if (mark)
        printf("mark=%u ", mark);

    ifi = nfq_get_indev(tb);
    if (ifi)
        printf("indev=%u ", ifi);

    ifi = nfq_get_outdev(tb);
    if (ifi)
        printf("outdev=%u ", ifi);
    ifi = nfq_get_physindev(tb);
    if (ifi)
        printf("physindev=%u ", ifi);

    ifi = nfq_get_physoutdev(tb);
    if (ifi)
        printf("physoutdev=%u ", ifi);

    ret = nfq_get_payload(tb, &data);
    if (ret >= 0)
        printf("payload_len=%d ", ret);
    fputc('\n', stdout);

    struct libnet_ipv4_hdr *ip = (struct libnet_ipv4_hdr*)data;
    if(ip->ip_v == IPv4 && ip->ip_p == PROTOCOL_TCP) {
        struct libnet_tcp_hdr *tcp = (struct libnet_tcp_hdr*)((uint8_t*)ip + (ip->ip_hl<<2));
        if(ntohs(tcp->th_sport)==PORT_HTTP || ntohs(tcp->th_dport)==PORT_HTTP) {
            if(DUMP_FLAG) dump((uint8_t*)data, id);
            *NF_FLAG = NF_DROP;
        }
    }
    return id;
}

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *nfmsg,
          struct nfq_data *nfa, void *data)
{
    uint8_t NF_FLAG = NF_ACCEPT;
    uint32_t id = print_pkt(nfa, &NF_FLAG);
    printf("entering callback\n\n");
    return nfq_set_verdict(qh, id, NF_FLAG, 0, NULL);
}

int main(int argc, char **argv)
{
    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd, rv;
    char buf[4096] __attribute__ ((aligned));

    printf("opening library handle\n");
    h = nfq_open();
    if (!h) { // open err
        fprintf(stderr, "error during nfq_open()\n");
        exit(1);
    }

    printf("unbinding existing nf_queue handler for AF_INET (if any)\n");
    if (nfq_unbind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_unbind_pf()\n");
        exit(1);
    }

    printf("binding nfnetlink_queue as nf_queue handler for AF_INET\n");
    if (nfq_bind_pf(h, AF_INET) < 0) {
        fprintf(stderr, "error during nfq_bind_pf()\n");
        exit(1);
    }

    printf("binding this socket to queue '0'\n");
    qh = nfq_create_queue(h,  0, &cb, NULL);
    if (!qh) {
        fprintf(stderr, "error during nfq_create_queue()\n");
        exit(1);
    }

    printf("setting copy_packet mode\n");
    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        fprintf(stderr, "can't set packet_copy mode\n");
        exit(1);
    }

    fd = nfq_fd(h);

    for (;;) {
        if ((rv = recv(fd, buf, sizeof(buf), 0)) >= 0) {
            printf("pkt received\n");
            nfq_handle_packet(h, buf, rv);
            continue;
        }
        if (rv < 0 && errno == ENOBUFS) {
            printf("losing packets!\n");
            continue;
        }
        perror("recv failed");
        break;
    }

    printf("unbinding from queue 0\n");
    nfq_destroy_queue(qh);

#ifdef INSANE
    printf("unbinding from AF_INET\n");
    nfq_unbind_pf(h, AF_INET);
#endif
    printf("closing library handle\n");
    nfq_close(h);

    exit(0);
}

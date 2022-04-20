#include "blescan.h"

static volatile int signal_received = 0;
static volatile sig_atomic_t e_flag = 0;

static void sig_handler(int signo)
{
    printf("sig handler2\n");
    if (signo == SIGINT)
    {
        e_flag = 1;
    }
}

static int read_flags(uint8_t *flags, const uint8_t *data, size_t size)
{
    size_t offset;

    if (!flags || !data)
        return -EINVAL;

    offset = 0;
    while (offset < size)
    {
        uint8_t len = data[offset];
        uint8_t type;

        /* Check if it is the end of the significant part */
        if (len == 0)
            break;

        if (len + offset > size)
            break;

        type = data[offset + 1];

        if (type == FLAGS_AD_TYPE)
        {
            *flags = data[offset + 2];
            return 0;
        }

        offset += 1 + len;
    }

    return -ENOENT;
}
static int check_report_filter(uint8_t procedure, le_advertising_info *info)
{
    uint8_t flags;

    /* If no discovery procedure is set, all reports are treat as valid */
    if (procedure == 0)
        return 1;

    /* Read flags AD type value from the advertising report if it exists */
    if (read_flags(&flags, info->data, info->length))
        return 0;

    switch (procedure)
    {
    case 'l': /* Limited Discovery Procedure */
        if (flags & FLAGS_LIMITED_MODE_BIT)
            return 1;
        break;
    case 'g': /* General Discovery Procedure */
        if (flags & (FLAGS_LIMITED_MODE_BIT | FLAGS_GENERAL_MODE_BIT))
            return 1;
        break;
    default:
        fprintf(stderr, "Unknown discovery procedure\n");
    }

    return 0;
}

static void sigint_handler(int sig)
{
    signal_received = sig;
}

static int print_advertising_devices(int dd, uint8_t filter_type)
{
    unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
    struct hci_filter nf, of;
    struct sigaction sa;
    socklen_t olen;
    int len = 0;

    olen = sizeof(of);
    if (getsockopt(dd, SOL_HCI, HCI_FILTER, &of, &olen) < 0)
    {
        printf("Could not get socket options\n");
        return -1;
    }

    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);

    if (setsockopt(dd, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0)
    {
        printf("Could not set socket options\n");
        return -1;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_NOCLDSTOP;
    sa.sa_handler = sigint_handler;
    sigaction(SIGINT, &sa, NULL);

    while ((sig_atomic_t *)e_flag == 0)
    {
        evt_le_meta_event *meta;
        le_advertising_info *info;
        char addr[18];
        char prev_addr[18];
        memset(prev_addr, '\0', 18);

        while ((len = read(dd, buf, sizeof(buf))) < 0)
        {
            if (errno == EINTR && signal_received == SIGINT)
            {
                len = 0;
                goto done;
            }

            if (errno == EAGAIN || errno == EINTR)
                continue;
            goto done;
        }

        ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
        len -= (1 + HCI_EVENT_HDR_SIZE);

        meta = (void *)ptr;

        if (meta->subevent != 0x02)
            goto done;

        /* Ignoring multiple reports */
        info = (le_advertising_info *)(meta->data + 1);
        if (check_report_filter(filter_type, info))
        {

            ba2str(&info->bdaddr, addr);
            if (memcmp(addr, prev_addr, 18) != 0)
            {
                printf("%s,%02x\n", addr, info->data[info->length]);
                memcpy(prev_addr, addr, 18);
            }
            fflush(stdout);
        }
    }

done:
    setsockopt(dd, SOL_HCI, HCI_FILTER, &of, sizeof(of));

    if (len < 0)
        return -1;

    return 0;
}

int main(void)
{
    int err, dd;
    uint8_t own_type = 0x00;
    uint8_t scan_type = 0x01; // Passive
    uint8_t filter_type = 0;
    uint8_t filter_policy = 0x00;
    uint16_t interval = htobs(0x0010);
    uint16_t window = htobs(0x0010);
    uint8_t filter_dup = 1;
    int dev_id = 0;

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("\ncan't catch SIGKILL\n");
    }

    if (dev_id < 0)
        dev_id = hci_get_route(NULL);

    dd = hci_open_dev(dev_id);
    if (dd < 0)
    {
        perror("Could not open device");
        exit(1);
    }

    err = hci_le_set_scan_parameters(dd, scan_type, interval, window,
                                     own_type, filter_policy, 1000);
    if (err < 0)
    {
        perror("Set scan parameters failed");
        exit(1);
    }

    err = hci_le_set_scan_enable(dd, 0x01, filter_dup, 1000);
    if (err < 0)
    {
        perror("Enable scan failed");
        exit(1);
    }

    err = print_advertising_devices(dd, filter_type);
    if (err < 0)
    {
        perror("Could not receive advertising events");
        exit(1);
    }

    err = hci_le_set_scan_enable(dd, 0x00, filter_dup, 1000);
    if (err < 0)
    {
        perror("Disable scan failed");
        exit(1);
    }

    hci_close_dev(dd);
    return 0;
}
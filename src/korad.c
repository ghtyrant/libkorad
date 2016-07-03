#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <libserialport.h>

#include "korad.h"

const struct KoradKnownDevice KORAD_KNOWN_DEVICES[] = {
    { "KORADKA3005PV2.0", 0x0416, 0x5011 },
    { NULL, 0, 0}
};

const KoradCommandSettings KORAD_COMMAND_SETTINGS[] = {
    { KORAD_COMMAND_IDN,                "*IDN?",   NULL,           1, 50, KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_STATUS,             "STATUS?", NULL,           1, 1,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_VOLTAGE_USER_GET,   "VSET1?",  NULL,           1, 5,  KORAD_CONVERT_FLOAT },
    { KORAD_COMMAND_VOLTAGE_USER_SET,   "VSET1:",  "VSET1:%02.2f", 0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_VOLTAGE_ACTUAL_GET, "VOUT1?",  NULL,           1, 5,  KORAD_CONVERT_FLOAT },
    { KORAD_COMMAND_CURRENT_USER_GET,   "ISET1?",  NULL,           1, 5,  KORAD_CONVERT_FLOAT },
    { KORAD_COMMAND_CURRENT_USER_SET,   "ISET1:",  "ISET1:%.3f",   0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_CURRENT_ACTUAL_GET, "IOUT1?",  NULL,           1, 5,  KORAD_CONVERT_FLOAT },
    { KORAD_COMMAND_OUTPUT,             "OUT",     "OUT%u",        0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_OVP,                "OVP",     "OVP%u",        0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_OCP,                "OCP",     "OCP%u",        0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_TRACK,              "TRACK",   "TRACK%u",      0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_RECALL,             "RCL",     "RCL%u",        0, 0,  KORAD_CONVERT_NONE  },
    { KORAD_COMMAND_SAVE,               "SAV",     "SAV%u",        0, 0,  KORAD_CONVERT_NONE  },

    // Termination
    {0, NULL, NULL, 0, 0},
};

static const KoradCommandSettings*
korad_find_command_settings(const char* command)
{
    for (const KoradCommandSettings *s = KORAD_COMMAND_SETTINGS; s->command; s++)
    {
        if (strcmp(s->command, command) == 0)
            return s;
    }

    return NULL;
}

static KORAD_ERROR
korad_command_new(KoradCommand **cmd, korad_result_handler handler, const char* command, ...)
{
    const KoradCommandSettings *s = korad_find_command_settings(command);

    // Unknown command?
    if (s == NULL)
    {
        *cmd = NULL;
        return KORAD_UNKNOWN_COMMAND;
    }

    KoradCommand *c = calloc(1, sizeof(KoradCommand));
    c->settings = s;
    c->handler = handler;

    va_list argptr;
    va_start(argptr, command);

    c->command = malloc(50 * sizeof(char));
    vsnprintf(c->command, 50, c->settings->format_string == NULL ? c->settings->command : c->settings->format_string, argptr);
    printf("Command: %s, Len: %d\n", c->command, strlen(c->command));
    va_end(argptr);

    *cmd = c;
    return KORAD_OK;
}

static void
korad_command_free(KoradCommand *c)
{
    free(c->command);
    free(c->result);
    free(c);
}

static void
korad_command_set_result(KoradCommand *c, char *result)
{
    if (c->settings->convert == KORAD_CONVERT_NONE)
    {
        c->result = calloc(1, c->settings->return_length + 1);
        memcpy(c->result, result, c->settings->return_length);
    }
    else if (c->settings->convert == KORAD_CONVERT_FLOAT)
    {
        float *tmp = malloc(sizeof(float));
        *tmp = strtof(result, NULL);
        c->result = (void*) tmp;
    }
    else if (c->settings->convert == KORAD_CONVERT_STATUS)
    {
        char status_byte = *result;
        KoradDeviceStatus *s = calloc(1, sizeof(KoradDeviceStatus));
        s->constant_current = !(status_byte & (1 << 0));
        s->beep             = (status_byte & (1 << 4));
        s->ocp              = (status_byte & (1 << 5));
        s->output           = (status_byte & (1 << 6));

        if (s->output == 1)
            s->ovp          = (status_byte & (1 << 7));

        c->result = (void*) s;
    }
}

static KORAD_ERROR
korad_device_send_command(KoradDevice *d, KoradCommand *c)
{
    if (d->queue_tail == NULL)
        d->queue = c;
    else
        d->queue_tail->next = c;

    d->queue_tail = c;

    return KORAD_OK;
}

static KoradKnownDevice *
korad_find_known_device_by_usb_id(int usb_vid, int usb_pid)
{
    for (const KoradKnownDevice *d = KORAD_KNOWN_DEVICES; d->name; d++)
    {
        if (d->vendor_id = usb_vid && d->product_id = usb_pid)
            return s;
    }

    return NULL;
}

KORAD_ERROR
korad_device_find(KoradDevice **d)
{
    struct sp_port** port_list = malloc(sizeof(struct sp_port**));

    if (sp_list_ports(&port_list) != SP_OK)
        return KORAD_NOT_FOUND;

    struct sp_port* korad_port = malloc(sizeof(struct sp_port*));
    struct sp_port** port_list_item = port_list;
    char found = 0;
    while(*port_list_item)
    {
        if (sp_get_port_transport(*port_list_item) == SP_TRANSPORT_USB)
        {
            int usb_vid, usb_pid;
            sp_get_port_usb_vid_pid(*port_list_item, &usb_vid, &usb_pid);
            printf("USB-Device: Port-Name: %s, Manufacturer: %s, Product: %s, Serial: %s, ID: %04x:%04x\n",
                sp_get_port_name(*port_list_item),
                sp_get_port_usb_manufacturer(*port_list_item),
                sp_get_port_usb_product(*port_list_item),
                sp_get_port_usb_serial(*port_list_item),
                usb_vid,
                usb_pid);

            KoradKnownDevice *kd = korad_find_known_device_by_usb_id(usb_vid, usb_pid);

            if (kd)
            {
                sp_copy_port(*port_list_item, &korad_port);
                found = 1;
                break;
            }
        }

        port_list_item++;
    }
    sp_free_port_list(port_list);

    if (found != 1)
    {
        *d = NULL;
        return KORAD_NOT_FOUND;
    }

    if (sp_open(korad_port, SP_MODE_READ_WRITE) != SP_OK)
    {
        sp_free_port(korad_port);
        return KORAD_COMMUNCATION_ERROR;
    }

    sp_set_flowcontrol(korad_port, SP_FLOWCONTROL_XONXOFF);
    sp_set_baudrate(korad_port, 9600);
    sp_set_bits(korad_port, 8);
    sp_set_parity(korad_port, SP_PARITY_NONE);
    sp_set_stopbits(korad_port, 1);

    const char buf[] = "*IDN?";
    int written = sp_blocking_write(korad_port, buf, strlen(buf), 0);
    printf("Wrote %d bytes to device!\n", written);
    sp_drain(korad_port);

    char in_buf[50];
    int bytes_total = 0;
    int tries = 0;
    while(bytes_total < strlen(KORAD_KA3005PV20) + 1 || tries >= 100)
    {
        bytes_total += sp_blocking_read(korad_port, in_buf + bytes_total, strlen(KORAD_KA3005PV20) + 1, 500);
        tries++;
    }
    printf("Bytes read: %d\n", bytes_total);

    if (bytes_total != strlen(KORAD_KA3005PV20) + 1)
    {
        sp_close(korad_port);
        sp_free_port(korad_port);
        return KORAD_COMMUNCATION_ERROR;
    }

    in_buf[bytes_total] = 0;

    if (strncmp(in_buf, KORAD_KA3005PV20, strlen(KORAD_KA3005PV20)) != 0)
    {
        printf("Did not recognize a supported Korad device: '%s'\n", in_buf);
        sp_close(korad_port);
        sp_free_port(korad_port);
        return -1;
    }

    KoradDevice *device = calloc(1, sizeof(KoradDevice));
    device->port = korad_port;
    device->buffer_pos = 0;

    *d = device;
    return KORAD_OK;
}

static KORAD_ERROR
korad_device_try_parse_buffer(KoradDevice *d)
{
    if (d->queue == NULL)
        return KORAD_NO_COMMAND;

    if (!d->queue->settings->has_return_value)
        return KORAD_NO_COMMAND;

    if (d->buffer_pos < d->queue->settings->return_length)
        return KORAD_READ_AGAIN;

    KoradCommand *c = d->queue;
    korad_command_set_result(d->queue, d->buffer);

    // BUG: ISET1? sends one extra byte, get rid of it
    if (c->settings->id == KORAD_COMMAND_CURRENT_USER_GET)
    {
        char tmp;
        sp_blocking_read(d->port, &tmp, 1, 100);
    }

    d->buffer_pos = 0;

    // Pop off the top command
    d->queue = c->next;

    if (d->queue == NULL)
        d->queue_tail = NULL;

    if (c->handler)
        c->handler(d, c);

    korad_command_free(c);

    return KORAD_OK;
}

static KORAD_ERROR
korad_device_read_command(KoradDevice *d)
{
    int bytes = sp_blocking_read(
        d->port,
        d->buffer + d->buffer_pos,
        d->queue->settings->return_length,
        500);

    // Timeout reached
    if (bytes == 0)
        return KORAD_OK;

    // Error
    if (bytes < 0)
        return KORAD_READ_ERROR;

    d->buffer_pos += bytes;

    if (d->buffer_pos >= sizeof(d->buffer))
        return KORAD_BUFFER_OVERFLOW;

    KORAD_ERROR err = korad_device_try_parse_buffer(d);

    return err;
}

static void
korad_device_send_next(KoradDevice *d)
{
    KoradCommand *c = d->queue;

    int written = sp_blocking_write(d->port, c->command, strlen(c->command), 0);
    sp_drain(d->port);
    printf("Running command '%s' (%d bytes) ...\n", c->command, written);
    c->is_sent = 1;

    // If we are not waiting for a result, pop this command off immediately
    if (c->settings->has_return_value == 0)
    {
        d->queue = c->next;

        if (d->queue == NULL)
            d->queue_tail = NULL;

        korad_command_free(c);
    }
}

void
korad_run(KoradDevice *d)
{
    struct sp_event_set *events = malloc(sizeof(struct sp_event_set*));
    sp_new_event_set(&events);

    sp_add_port_events(events, d->port, SP_EVENT_RX_READY|SP_EVENT_RX_READY|SP_EVENT_ERROR);

    int event_count = 0;
    while(d->quit == 0)
    {
        sp_wait(events, 500);
        event_count++;

        if (event_count >= 20)
            break;

        if (d->queue == NULL)
            continue;

        if (d->queue->is_sent)
            korad_device_read_command(d);
        else
            korad_device_send_next(d);
    }

    sp_free_event_set(events);
}

void
korad_device_stop(KoradDevice *d)
{
    d->quit = 1;
}

void
korad_device_free(KoradDevice *d)
{
    sp_close(d->port);
    sp_free_port(d->port);

    KoradCommand *queue_item = d->queue;

    while (queue_item)
    {
        KoradCommand *next = queue_item->next;
        korad_command_free(queue_item);
        queue_item = next;
    }

    free(d);
}

void
korad_send(KoradDevice *d, korad_result_handler callback, const char *command, ...)
{
    KoradCommand *cmd;
    va_list argptr;
    va_start(argptr, command);
    korad_command_new(&cmd, callback, command, argptr);
    va_end(argptr);

    korad_device_send_command(d, cmd);
}
#include <stdio.h>
#include <korad.h>

void
tmp_handler(KoradDevice *d, KoradCommand *c)
{
    //printf("Result is: %s\n", c->result);
}

int
main (int argc, char *argv[])
{
    KoradDevice *device = NULL;

    if (korad_device_find(&device) != KORAD_OK)
        return -1;

    printf("Found KORAD power supply unit!\n");
    printf("Dev: %s\n", device->known_device->name);

    korad_set_voltage(device, 12.34);

    korad_run(device);

    korad_device_free(device);

    return 0;
}
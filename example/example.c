#include <stdio.h>
#include <korad.h>
#include <unistd.h>
#include <pthread.h>

void
tmp_handler(KoradDevice *d, KoradCommand *c)
{
    //printf("Result is: %s\n", c->result);
}

void *
do_run(void *data)
{
    KoradDevice *device = (KoradDevice*) data;
    korad_run(device);
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

    pthread_t t;
    pthread_create(&t, NULL, do_run, device);

    printf("Before sleep!\n");
    sleep(2);
    printf("After sleep!\n");

    int toggle = 0;

    for (int i = 0; i < 6; ++i)
    {
        toggle = 1 - toggle;
        printf("Before call ...\n");
        korad_ovp(device, toggle);
        printf("After call ...\n");
        sleep(2);
    }

    korad_device_stop(device);

    pthread_join(t, NULL);
    korad_device_free(device);

    return 0;
}

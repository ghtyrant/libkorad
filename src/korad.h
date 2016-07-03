#ifndef KORAD_H_
#define KORAD_H_

/* Library symbol visibilty magic */
#if defined _WIN32 || defined __CYGWIN__
  #define KORAD_DLL_IMPORT __declspec(dllimport)
  #define KORAD_DLL_EXPORT __declspec(dllexport)
  #define KORAD_DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define KORAD_DLL_IMPORT __attribute__ ((visibility ("default")))
    #define KORAD_DLL_EXPORT __attribute__ ((visibility ("default")))
    #define KORAD_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
  #else
    #define KORAD_DLL_IMPORT
    #define KORAD_DLL_EXPORT
    #define KORAD_DLL_LOCAL
  #endif
#endif

#ifdef KORAD_DLL
    #ifdef KORAD_DLL_EXPORTS
        #define KORAD_API KORAD_DLL_EXPORT
    #else
        #define KORAD_API KORAD_DLL_IMPORT
    #endif

    #define KORAD_LOCAL KORAD_DLL_LOCAL
#else
    #define KORAD_API
    #define KORAD_LOCAL
#endif

typedef struct
{
  const char *name;
  int vendor_id;
  int product_id;
} KoradKnownDevice;

extern const KoradKnownDevice KORAD_KNOWN_DEVICES[];

typedef enum
{
    KORAD_OK = 0,
    KORAD_NOT_FOUND,
    KORAD_COMMUNCATION_ERROR,
    KORAD_UNKNOWN_COMMAND,
    KORAD_READ_ERROR,
    KORAD_BUFFER_OVERFLOW,
    KORAD_NO_COMMAND,
    KORAD_READ_AGAIN
} KORAD_ERROR;

typedef enum
{
    KORAD_COMMAND_IDN = 0,
    KORAD_COMMAND_STATUS,
    KORAD_COMMAND_VOLTAGE_USER_GET,
    KORAD_COMMAND_VOLTAGE_USER_SET,
    KORAD_COMMAND_VOLTAGE_ACTUAL_GET,
    KORAD_COMMAND_CURRENT_USER_GET,
    KORAD_COMMAND_CURRENT_USER_SET,
    KORAD_COMMAND_CURRENT_ACTUAL_GET,
    KORAD_COMMAND_OUTPUT,
    KORAD_COMMAND_OVP,
    KORAD_COMMAND_OCP,
    KORAD_COMMAND_TRACK,
    KORAD_COMMAND_RECALL,
    KORAD_COMMAND_SAVE
} KORAD_COMMAND_ID;

typedef enum
{
  KORAD_CONVERT_NONE,
  KORAD_CONVERT_FLOAT,
  KORAD_CONVERT_STATUS
} KORAD_VALUE_CONVERT;

typedef struct _KoradCommandSettings
{
    KORAD_COMMAND_ID id;
    const char* command;
    const char* format_string;
    char has_return_value;
    unsigned int return_length;
    KORAD_VALUE_CONVERT convert;
} KoradCommandSettings;

extern const KoradCommandSettings KORAD_COMMAND_SETTINGS[];

typedef struct
{
  char constant_current;
  char beep;
  char ocp;
  char ovp;
  char output;

} KoradDeviceStatus;

typedef struct _KoradCommand KoradCommand;
typedef struct _KoradDevice KoradDevice;
typedef void (*korad_result_handler)(KoradDevice *device, KoradCommand* command);

struct _KoradCommand
{
    char *command;
    void *result;
    char is_sent;
    const KoradCommandSettings *settings;
    struct _KoradCommand *next;
    korad_result_handler handler;
};

struct _KoradDevice
{
    struct sp_port *port;
    const KoradKnownDevice *known_device;
    char quit;
    char buffer[1024];
    unsigned int buffer_pos;
    KoradCommand *queue;
    KoradCommand *queue_tail;
};

KORAD_API KORAD_ERROR korad_device_find(KoradDevice **d);
KORAD_API void korad_run(KoradDevice *d);
KORAD_API void korad_device_stop(KoradDevice *d);
KORAD_API void korad_device_free(KoradDevice *d);
KORAD_API void korad_send(KoradDevice *d, korad_result_handler callback, const char *command, ...);

#define korad_set_voltage(d, v)              korad_send((d), NULL, "VSET1:", (v))
#define korad_set_current(d, c)              korad_send((d), NULL, "ISET1:", (c))

#define korad_get_maximum_voltage(d, cb)     korad_send((d), (cb), "VSET1?")
#define korad_get_maximum_current(d, cb)     korad_send((d), (cb), "ISET1?")

#define korad_get_actual_voltage(d, cb)      korad_send((d), (cb), "VOUT1?")
#define korad_get_actual_current(d, cb)      korad_send((d), (cb), "IOUT1?")

#define korad_output(d, f)                   korad_send((d), NULL, "OUT", (f))
#define korad_output_on(d)                   korad_output((d), 1)
#define korad_output_off(d)                  korad_output((d), 0)

#define korad_over_voltage_protection(d, f)  korad_send((d), NULL, "OVP", (f))
#define korad_over_voltage_protection_on(d)  korad_over_voltage_protection((d), 1)
#define korad_over_voltage_protection_off(d) korad_over_voltage_protection((d), 0)
#define korad_ovp(d, f)                      korad_over_voltage_protection((d), (f))
#define korad_ovp_on(d)                      korad_over_voltage_protection_on((d))
#define korad_ovp_off(d)                     korad_over_voltage_protection_off((d))

#define korad_over_current_protection(d, f)  korad_send((d), NULL, "OCP", (f))
#define korad_over_current_protection_on(d)  korad_over_current_protection((d), 1)
#define korad_over_current_protection_off(d) korad_over_current_protection((d), 0)
#define korad_ocp(d, f)                      korad_over_current_protection((d), (f))
#define korad_ocp_on(d)                      korad_over_current_protection_on((d))
#define korad_ocp_off(d)                     korad_over_current_protection_off((d))

#define korad_save(d, p)                     korad_send((d), NULL, "SAV", (p))
#define korad_recall(d, p)                   korad_send((d), NULL, "RCL", (p))

#endif // KORAD_H_
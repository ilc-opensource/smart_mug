#include <stdint.h>
#include <linux/types.h>
#include <iohub_client.h>

#ifdef __cplusplus
extern "C" { 
#endif

#ifndef BYTE
typedef uint8_t BOOL;
typedef uint8_t BYTE;
typedef uint32_t DWORD;

#define TRUE  1
#define FALSE 0
#endif

#define BYTES_PER_ROW (16 / 2)

#define MAX_LED_FRAMES 100

#define FRAME_DURATION_MASK (0x10)
#define FRAME_ACTIVE_MASK   (0x02)
#define FRAME_ACTIVE_FLAG   (0x01)

#define FRAME_ENABLE_MASK   (0x08)
#define FRAME_ENABLE_FLAG   (0x04)


struct __attribute__((packed)) LedFrameMesg {
    BYTE type;
    BYTE frame;
    union {
        struct LedFrameRow {
            BYTE index;
            BYTE content[BYTES_PER_ROW];
        } row;

        struct LedFrameSet {
            BYTE flags;
            BYTE reserved;
            DWORD duration;
        } set;
    };
};

#define IOHUB_CMD_FRAME     4

#define FRAME_CMD_ROW       0
#define FRAME_CMD_SET       1


int iohub_write_block_data(int fd, __u8 cmd, __u8 length, const __u8 *data);
int iohub_read_block_data(int fd, __u8 cmd, __u8 length, __u8 *data);
int iohub_user_i2c_init();

handle_t dev_open(device_t type);
error_t  dev_send_command(handle_t handle, cmd_t cmdtype, char *data, int message_len);
void     dev_close(handle_t handle);

int      get_mpu_handle();
int      get_tp_handle();

#ifdef __cplusplus
}
#endif

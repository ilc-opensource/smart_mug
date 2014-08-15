#include <linux/types.h>
#include <iohub_client.h>

#ifdef __cplusplus
extern "C" { 
#endif

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

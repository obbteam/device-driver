#ifndef DEVICE_FILE_H_
#define DEVICE_FILE_H_

int register_device(void);
void unregister_device(void);
void declare_gpio(void);
void free_gpio(void);
void clear_seven_segment(void);
void write_num(int num);
extern struct platform_driver sevenseg_driver;

#endif // DEVICE_FILE_H_
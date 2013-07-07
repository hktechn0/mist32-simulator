#define MONITOR_PORT 1032
#define MONITOR_BUF_SIZE 4096

void monitor_init(void);
void monitor_close(void);

void monitor_method_send(void);
void monitor_method_recv(void);
void monitor_display_draw(unsigned int x, unsigned int y, unsigned int color);


/*
    gcc -o mesure mesure.c -lrt -lpthread
    sudo ./mesure
*/

//XXX: OSが裏で何かしに来た際などにずれが生じている OSが悪さをしないようにする必要がある
//XXX: しっかりと確認したわけではないが数ms単位でずれるため一部波形が欠けてしまう
//XXX: 解決方法としてはbaremetalでの動作などが考えられるがハードルは高い

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define DEV_FB "/dev/fb0"
#define WIDTH 1184
#define HEIGHT 624

#define WHITE   0x00FFFFFF
#define BLACK   0x00000000
#define BLUE    0x000000FF
#define GRAY    0x00A0A0A0
#define YELLOW  0x00FFFF00

#define PERI_ADD 0x3F000000
#define GPIO_ADD (PERI_ADD + 0x00200000)
#define PAGE_SIZE 4096

#define GPIO_INPUT 0x0  //  入力
#define GPIO_OUTPUT 0x1 //  出力

#define MISO 19
#define MOSI 13
#define CLK 26
#define CS 6

#define R_ENC_R 20
#define R_ENC_L 21

static volatile unsigned int *Gpio = NULL;

void gpio_init()
{
    if (Gpio)
        return;

    int fd;
    void *gpio_map;

    fd = open("/dev/mem", O_RDWR | O_SYNC);

    if (fd == -1)
    {
        printf("error: cannot open /dev/mem\n");
        exit(-1);
    }

    gpio_map = mmap(NULL, PAGE_SIZE,
                    PROT_READ | PROT_WRITE, MAP_SHARED,
                    fd, GPIO_ADD);

    if ((int)gpio_map == -1)
    {
        printf("error: cannot map /dev/mem/ on the memory\n");
        exit(-1);
    }

    close(fd);

    Gpio = (unsigned int *)gpio_map;
}

void gpio_configure(int pin, int mode)
{
    if (pin < 0 || pin > 31)
    {
        printf("error: pin number out of range (gpio_configure)\n");
        exit(-1);
    }
    int index = pin / 10;
    unsigned int mask = ~(0x7 << ((pin % 10) * 3));

    Gpio[index] = (Gpio[index] & mask) | ((mode & 0x7) << ((pin % 10) * 3));
}

void gpio_configure_pull (int pin, int pullmode)
{
    if (pin < 0 || pin > 31) {
        printf("error: pin number out of range (gpio_configure_pull)\n");
        exit(-1);
    }
    Gpio[37] = pullmode & 0x3;
    usleep(1);
    Gpio[38] = 0x1 << pin;
    usleep(1);

    Gpio[37] = 0;
    Gpio[38] = 0;
}

void gpio_set(int pin)
{
    Gpio[7] = 0x1 << pin;
}

void gpio_clear(int pin)
{
    Gpio[10] = 0x1 << pin;
}

void gpio_sleep(int miso, int mosi, int clk, int cs, int sleep_time)
{
    int i = 0;
    while (i < sleep_time)
    {
        gpio_clear(mosi);
        gpio_clear(clk);
        gpio_set(cs);
        i++;
    }
}

int gpio_read(int pin)
{
    return (Gpio[13] & (0x1 << pin)) != 0;
}

//mcp3208一つのみのspi
int spi_xfer(int miso, int mosi, int clk, int cs, int tx_buf){
    int data;
    int rx_buf = 0x000000;

    gpio_sleep(miso, mosi, clk, cs, 1);
    gpio_clear(cs);

    for(int k=0; k<48; k++){
        k%2==0 ? gpio_set(clk) : gpio_clear(clk);
        tx_buf&(0x800000>>((k+1)/2)) ? gpio_set(mosi) : gpio_clear(mosi);
        k%2==0 ? rx_buf = rx_buf|gpio_read(miso)<<(23-(k/2)) : gpio_read(miso);
    }

    gpio_set(cs);
    gpio_sleep(miso, mosi, clk, cs, 1);

    data = (rx_buf&(0xFFF0))>>4;

    return data;
}

int flag = 0;
int get_flag = 0;

int buf_num = 50;
int save_num = 90000;
int datas[5000];
int save_data[90000];

static volatile unsigned int *fbptr = NULL;

void drawLine(int x0, int y0, int x1, int y1, int color, volatile unsigned int *canvas){
    if(y0 > y1){
        for(int i=y0; i>y1; i--){
            canvas[i*WIDTH+x0] = color;
        }
    }else if(y0 < y1){
        for(int i=y0; i<y1; i++){
            canvas[i*WIDTH+x0] = color;
        }
    }else{
        canvas[y0*WIDTH+x0] = color;
    }
}

void *get_data(void *arg){
    int miso = MISO;
    int mosi = MOSI;
    int clk = CLK;
    int cs = CS;

    int white = WHITE;
    int gray = GRAY;
    int blue = BLUE;
    int yellow = YELLOW;

    int width = WIDTH;
    int height = HEIGHT;

    int fb = open(DEV_FB, O_RDWR | O_SYNC);
    if(fb < 0) {
        fprintf(stderr, "framebuffer open error\n");
    }

    fbptr = (int *)mmap(0, width*height*4, 
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        fb, 0);

    gpio_init();

    gpio_configure(miso, GPIO_INPUT);
    gpio_configure(mosi, GPIO_OUTPUT);
    gpio_configure(clk, GPIO_OUTPUT);
    gpio_configure(cs, GPIO_OUTPUT);

    int time = 0;
    int count = 0;

    int value;
    int v0 = 0;
    int v1 = 0;

    int i = 0;

    for(;;){
        if(flag == 1){
            printf("stop mesuring\n");
            break;
        }

        value = spi_xfer(miso, mosi, clk, cs, 0x600000);

        if(i%buf_num == 0){
            v1 = v0;
            v0 = value;

            for(int n=i/buf_num%width; n<i/buf_num%width+1; n++){
                for(int m=0; m<height; m++){
                    fbptr[m*width+n] = WHITE;
                }
            }

            drawLine(i/buf_num%width, height-120-v0/8, i/buf_num%width+1, height-120-v1/8, 0xFF0000, fbptr);
        }

        if(flag == 2){
            save_data[count] = v0;
            count++;
            if(count == save_num-1){
                printf("finish mesuring\n");
                flag = 0;
                count = 0;
            }
        }
        i++;
    }

    close(fb);
}

void *check_mode(void *arg){
    int a;
    for(;;){
        printf("type mode >>");
        scanf("%d", &a);
        if(a == 100){
            flag = 1;
            printf("end program\n");
            break;
        }else if(a == 2){
            flag = 2;
            printf("now mesuring\n");
        }
    }
}

void *check_rote(void *arg){
    int r_enc_r = R_ENC_R;
    int r_enc_l = R_ENC_L;

    gpio_init();
    gpio_configure(r_enc_r, GPIO_INPUT);
    gpio_configure(r_enc_l, GPIO_INPUT);
    
    for(;;){
        if(flag == 1){
            printf("end check rote\n");
            break;
        }
        if(gpio_read(r_enc_r) == 0 && gpio_read(r_enc_l) == 1){
            if(buf_num > 10) buf_num -= 10;
            usleep(5000*10);
            printf("left\n");
        }else if(gpio_read(r_enc_l) == 0 && gpio_read(r_enc_r) == 1){
            if(buf_num < 200){
                buf_num += 10;
            }
            usleep(5000*10);
            printf("right\n");
        }
    }
}

void *check_button(void *arg){
    gpio_init();
    gpio_configure(16, GPIO_INPUT);
    gpio_configure_pull(16, 0x2);

    int button_state;

    for(;;){
        button_state = gpio_read(16);
        if(flag == 1){
            printf("end check button\n");
            break;
        }
        if(button_state == 0){
            printf("button pushed!\n");
            flag = 1;
            sleep(1);
        }
    }
}

void graph(){
    int miso = MISO;
    int mosi = MOSI;
    int clk = CLK;
    int cs = CS;

    int white = WHITE;
    int gray = GRAY;
    int blue = BLUE;
    int yellow = YELLOW;

    int width = WIDTH;
    int height = HEIGHT;

    int fb = open(DEV_FB, O_RDWR | O_SYNC);
    if(fb < 0) {
        fprintf(stderr, "framebuffer open error\n");
    }

    fbptr = (int *)mmap(0, width*height*4, 
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        fb, 0);

    gpio_init();

    gpio_configure(miso, GPIO_INPUT);
    gpio_configure(mosi, GPIO_OUTPUT);
    gpio_configure(clk, GPIO_OUTPUT);
    gpio_configure(cs, GPIO_OUTPUT);

    int time = 0;
    int count = 0;

    int value;
    int v0 = 0;
    int v1 = 0;

    int i = 0;

    for(;;){
        if(flag == 1){
            printf("stop mesuring\n");
            break;
        }

        value = spi_xfer(miso, mosi, clk, cs, 0x600000);

        if(i%buf_num == 0){
            v1 = v0;
            v0 = value;

            for(int n=i/buf_num%width; n<i/buf_num%width+1; n++){
                for(int m=0; m<height; m++){
                    fbptr[m*width+n] = BLACK;
                }
            }

            drawLine(i/buf_num%width, height-120-v0/8, i/buf_num%width+1, height-120-v1/8, 0xFF0000, fbptr);
        }

        if(flag == 2){
            save_data[count] = v0;
            count++;
            if(count == save_num-1){
                printf("finish mesuring\n");
                flag = 0;
                count = 0;
            }
        }
        i++;
    }

    close(fb);
}

int main()
{
    /*
    int mem_fd;
    char *map;
    int i;
    int irq1, irq2, irq3;

    volatile unsigned int *irqen1;
    volatile unsigned int *irqen2;
    volatile unsigned int *irqen3;
    volatile unsigned int *irqdi1;
    volatile unsigned int *irqdi2;
    volatile unsigned int *irqdi3;

    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) return -1;
    map = (char*) mmap(NULL,
            4096,
            PROT_READ | PROT_WRITE,
            MAP_SHARED,
            mem_fd,
            PERI_ADD + 0xb000
            );
    
    if (map == MAP_FAILED)  return -1;
    irqen1 = (volatile unsigned int *) (map + 0x210);
    irqen2 = (volatile unsigned int *) (map + 0x214);
    irqen3 = (volatile unsigned int *) (map + 0x218);
    irqdi1 = (volatile unsigned int *) (map + 0x21c);
    irqdi2 = (volatile unsigned int *) (map + 0x220);
    irqdi3 = (volatile unsigned int *) (map + 0x224);

    irq1 = *irqen1;
    irq2 = *irqen2;
    irq3 = *irqen3;
    // 以下3行で全部の割り込みを禁止する
    *irqdi1 = 0xffffffff;
    *irqdi2 = 0xffffffff;
    *irqdi3 = 0xffffffff;
    */

    pthread_t check_thread;
    pthread_t get_data_thread;
    pthread_t check_button_thread;
    pthread_t check_rote_thread;

    pthread_create(&check_thread, NULL, check_mode, NULL);
    pthread_create(&get_data_thread, NULL, get_data, NULL);
    pthread_create(&check_rote_thread, NULL, check_rote, NULL);
    pthread_create(&check_button_thread, NULL, check_button, NULL);

    pthread_join(check_thread, NULL);
    pthread_join(get_data_thread, NULL);
    pthread_join(check_rote_thread, NULL);
    pthread_join(check_button_thread, NULL);

    FILE *file = fopen("/boot/data.csv", "w");

    for(int i=0; i<save_num; i++){
        fprintf(file, "%d\n", save_data[i]);
    }

    fclose(file);

    /*
    *irqen1 = irq1;
    *irqen2 = irq2;
    *irqen3 = irq3;
    */

    return 0;
}

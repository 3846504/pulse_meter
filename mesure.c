
/*
    gcc -o mesure mesure.c -lrt -lpthread
    sudo ./mesure
*/
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
#define GRAY    0xFFA0A0A0

#define PERI_ADD 0x3F000000
#define GPIO_ADD (PERI_ADD + 0x00200000)
#define PAGE_SIZE 4096

#define GPIO_INPUT 0x0  //  入力
#define GPIO_OUTPUT 0x1 //  出力

#define MISO 19
#define MOSI 13
#define CLK 26
#define CS 6

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
int start_flag = 0;
int buf_num = 3000;
int save_num = 90000;
int datas[5000];
int save_data[90000];

static volatile unsigned int *fbptr = NULL;

void *graph(void *arg){
    int hoge = 0;

    int white = WHITE;
    int gray = GRAY;
    int blue = BLUE;

    int width = WIDTH;
    int height = HEIGHT;

    int canvas[width*height];
    int graph[width*height];

    int fb = open(DEV_FB, O_RDWR | O_SYNC);
    if(fb < 0) {
        fprintf(stderr, "framebuffer open error\n");
    }

    fbptr = (int *)mmap(0, width*height*4, 
                        PROT_READ | PROT_WRITE, MAP_SHARED,
                        fb, 0);

    for(int i=0; i<height; i++){
        for(int j=0; j<width; j++){
            canvas[i*width+j] = white;
            if(((i-100)%102 == 0) && (j>150 && (width-150)>j && i<(height-100))) canvas[i*width+j] = gray;
        }
    }

    for(;;){
        if(flag == 1){
            printf("stop graph\n");
            break;
        }
        memcpy(graph, canvas, sizeof(canvas));

        for(int i=100; i<height-100; i++){
            for(int j=150; j<width-150; j++){
                graph[j+width*(height-120-datas[(j-150)*(buf_num/1000)]/8)] = blue;
            }
        }
        memcpy(fbptr, graph, sizeof(graph));
    }

    close(fb);
}

void *get_data(void *arg){
    int miso = MISO;
    int mosi = MOSI;
    int clk = CLK;
    int cs = CS;

    gpio_init();

    gpio_configure(miso, GPIO_INPUT);
    gpio_configure(mosi, GPIO_OUTPUT);
    gpio_configure(clk, GPIO_OUTPUT);
    gpio_configure(cs, GPIO_OUTPUT);

    int i = 0;
    int time = 0;

    for(;;){
        if(flag == 1){
            printf("stop mesuring\n");
            break;
        }
        datas[i%buf_num] = spi_xfer(miso, mosi, clk, cs, 0x600000);
        usleep(1 * 500);
        if(flag == 2 & (i%save_num==0)){
            start_flag = 1;
        }

        if(start_flag == 1){
            save_data[i%save_num] = datas[i%buf_num];
            if(i%save_num == save_num-1){
                printf("finish mesuring\n");
                start_flag = 0;
                flag = 0;
            }
        }
        i++;
    }
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
        }else if(a == 10){
            if(buf_num > 1000) buf_num -= 1000;
            a = 0;
        }else if(a == 20){
            if(buf_num < 4000) buf_num += 1000;
            a = 0;
        }
    }
}

int main()
{
    pthread_t check_thread;
    pthread_t get_data_thread;
    pthread_t graph_thread;

    pthread_create(&check_thread, NULL, check_mode, NULL);
    pthread_create(&get_data_thread, NULL, get_data, NULL);
    pthread_create(&graph_thread, NULL, graph, NULL);

    pthread_join(check_thread, NULL);
    pthread_join(get_data_thread, NULL);
    pthread_join(graph_thread, NULL);

    FILE *file = fopen("/data.csv", "w");

    for(int i=0; i<90000; i++){
        fprintf(file, "%d\n", save_data[i]);
    }

    return 0;
}

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

int last = 0;
void waitNext(int t){
    int next = last + t; 
    do {
        delay(1);
    } while (millis() < next); 
    last = next;
}

void init(void){
    if (wiringPiSetupGpio() == -1) exit(1);
    printf("Initializing PWM signal.\n");
    pinMode(18,PWM_OUTPUT);
    pwmSetMode(PWM_MODE_MS);
    pwmSetClock(2);
    pwmSetRange(140);
    printf("PWM signal initialized successfully.");
}

void marker(){
    pwmWrite(18,70);
    waitNext(999);
}

void zero(){
    pwmWrite(18,2);
    waitNext(100);
    pwmWrite(18,70);
    waitNext(900);
}

void one(){
    pwmWrite(18,2);
    waitNext(200);
    pwmWrite(18,70);
    waitNext(800);
}

void two(){
    pwmWrite(18,2);
    waitNext(300);
    pwmWrite(18,70);
    waitNext(700);
}

void three(){
    pwmWrite(18,2);
    waitNext(400);
    pwmWrite(18,70);
    waitNext(600);
}

int getP3(int frame,int hour,int minute,int weakday){
    int counter = 0;
    int flag = 0;
    if(hour<12){
        flag = 0;
    }
    else{
        flag = 1;
    }
    while(frame != 0){
        counter += frame & 1;
        frame >>= 1;
    }
    while(hour != 0){
        counter += hour & 1;
        hour >>= 1;
    }
    while(minute != 0){
        counter += minute & 1;
        minute >>= 1;
    }
    while(weakday != 0){
        counter += weakday & 1;
        weakday >>= 1;
    }
    switch(counter%2){
        case 0:
        if(flag==0){
            return 0;
        }
        else{
            return 2;
        }
        break;

        case 1:
        if(flag==0){
            return 1;
        }
        else{
            return 3;
        }
        break;
    }
}

int getP4(int day,int month,int year){
    int i=0;
    int counter = 0;
    for(i=0;i<3;i++){
        counter += day & 1;
        counter += month & 1;
        counter += year & 1;
        day >>= 1;
        month >>= 1;
        year >>= 1;
    }
    if(counter%2==0){
        return 0;
    }
    else{
        return 1;
    }
}

void send(int s){
    printf("%d\n",s);
    switch(s){
        case 0:
        zero();
        break;
        case 1:
        one();
        break;
        case 2:
        two();
        break;
        case 3:
        three();
        break;
    }
}

int main(){
    init();
    //
    int P1;
    int P2;
    int P3;
    int P4;
    int hour;
    int minute;
    int weakday;
    int day;
    int month;
    int year;
    //
    time_t current = time(NULL);
    struct tm *tm_struct = gmtime(&current);
    printf("Waiting for next frame...\n");

    do {
        delay(20);
        current = time(NULL);
        tm_struct = localtime(&current);
    } while(tm_struct->tm_sec % 20 != 0);

    while(1){
        marker();
        current = time(NULL);
        tm_struct = localtime(&current);
        P1 = tm_struct->tm_sec / 20;
        P2 = 0;
        hour = tm_struct->tm_hour;
        minute = tm_struct->tm_min;
        weakday = tm_struct->tm_wday;
        P3 = getP3(P1,hour,minute,weakday);
        day = tm_struct->tm_mday;
        month = 1+tm_struct->tm_mon;
        year = tm_struct->tm_year - 100;
        P4 = getP4(day,month,year);
        printf("BPC Frame:%d Started at %d-%d-%d  %d  %d:%d\n",P1,year,month,day,weakday,hour,minute);
        //
        send(P1);
        send(P2);
        send(hour>>2);
        send(hour&3);
        send(minute>>4);
        send((minute&12)>>2);
        send(minute&3);
        send(weakday>>2);
        send(weakday&3);
        send(P3);
        send(day>>4);
        send((day&12)>>2);
        send(day&3);
        send(month>>2);
        send(month&3);
        send(year>>4);
        send((year&12)>>2);
        send(year&3);
        send(P4);
    }
    
}
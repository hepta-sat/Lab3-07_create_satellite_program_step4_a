#include "mbed.h"
#include "HEPTA_CDH.h"
#include "HEPTA_EPS.h"
DigitalOut cond[]={LED1,LED2,LED3,LED4};
RawSerial gs(USBTX,USBRX,9600); // for ground station
HEPTA_CDH cdh(p5, p6, p7, p8, "sd");
HEPTA_EPS eps(p16,p26);
int rcmd = 0,cmdflag = 0; //command variable
 
//getting command and flag
void commandget()
{
    rcmd=gs.getc();
    cmdflag = 1;
}
//interrupting 
void receive(int rcmd, int cmdflag)
{
    gs.attach(commandget,Serial::RxIrq);
}
//initialzing
void initialize()
{
    rcmd = 0;
    cmdflag = 0;
    cond[0] = 0;
}

int main() {
    gs.printf("From Sat : Operation Start...\r\n");
    int flag = 0;     // condition
    float sattime=0.0,btvol,temp; //Voltage, Temerature
    receive(rcmd,cmdflag); //interupting by ground station command
    
    for(int i = 0; i < 100; i++) {
        //Sensing and Transmitting HK data
        eps.vol(&btvol);
        temp = 28.5;
        gs.printf("HEPTASAT::Condition = %d, Time = %f [s], BatVol = %.2f [V],Temp = %.2f [C]\r\n",flag,sattime,btvol,temp);
        //Condition
        cond[0] = 1;
        //Power Saving Mode
        if(btvol <= 3.5) {
            eps.shut_down_regulator();
            flag = 1;
        } else {
            eps.turn_on_regulator();
            flag = 0;
        }
        if(flag == 1) {
            gs.printf("Power saving mode ON\r\n"); 
        }
        //Contents of command
        if (cmdflag == 1) {
            if (rcmd == 'a') {
               for(int j=0;j<5;j++){
                gs.printf("HEPTASAT::Hello World!\r\n");
                cond[0] = 0;
                wait(1);
                cond[0] = 1;
               }
            }
            if (rcmd == 'b') {
                char str[100];
                mkdir("/sd/mydir", 0777);
                FILE *fp = fopen("/sd/mydir/test.txt","w");
                if(fp == NULL) {
                    error("Could not open file for write\r\n");
                }
                else
                {
                    for(int i = 0; i < 10; i++) {
                        eps.vol(&btvol);
                        fprintf(fp,"%f\r\n",btvol);
                        wait(1.0);
                    }
                    fclose(fp);
                    fp = fopen("/sd/mydir/test.txt","r");
                    for(int j = 0; j < 10; j++) {
                        fgets(str,100,fp);
                        puts(str);
                    }
                    fclose(fp);
                }
                cond[0] = 0;
                wait(1);
                cond[0] = 1;                  
            }
            initialize();
        }
        
        //Operation Interval
        wait(1.0);
        sattime = sattime+1.0;
    }
    gs.printf("From Sat : Operation Stop...\r\n");
}
           
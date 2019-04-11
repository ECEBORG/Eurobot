#include "mbed.h"
#include "rtos.h"
#include "QEI.h"
#include "motionCtrl.h"
#include <vector>


int main(){
	//SharpSensor s3=new SharpSensor(SHARP2);
	Serial pc(SERIAL_TX,SERIAL_RX,115200);
	std::vector<Task> v;
	CAN can1(PD_0, PD_1);
	char counter = 'a';
	DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
	Task s1("MOVE_POS",600,0.0,0.0);
	Task s2("MOVE_ANG",0.0,0.0,M_PI/2);
	//Task s3("MOVE_ANG",0.0,0.0,-M_PI/2);
	v.push_back(s1);
	//v.push_back(s2);
	//v.push_back(s2);
	//v.push_back(s3);
	//Task Table[2]= {new Task("MOVE_POS",500,0,0),new Task("MOVE_POS",0,0,0)};
	/*Task t1,t2;
	t1=new Task("MOVE_POS",500,0,0);
	t2= new Task("MOVE_POS",0,0,0)
	Table[0]=t1;
	Table[1]=t2;
float a=200;*/
//motionCtrl test=motionCtrl(0.0,0.0,0.0,v);
CANMessage msg;
    can1.frequency(250000);
   
   
    
    while(1) {
        if(can1.write(CANMessage(3, &counter, 1))) {
            led3 = !led3;
            pc.printf("reussite");
        }
        else
        {
            led2 = !led2;
            pc.printf("erreur");
        }
        counter++;
        wait(1);
    }
while(1)
{
	//test.s1.update();
	//a=test.s1.get_val();
	//pc.printf("x=%f  y=%f angle=%f Pwml= %f Pwmr= %f\n Time: %f\n",test.Posx,test.Posy,test.Angle,test.sPwm_L,test.sPwm_R,test.affiche);
	wait(1);
	

	//		test.addtask(s1);

}

}
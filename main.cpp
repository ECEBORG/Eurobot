#include "mbed.h"
//#include "rtos.h"
//#include "QEI.h"
#include "motionCtrl.h"
#include <vector>

	DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
int main(){
	//SharpSensor s3=new SharpSensor(SHARP2);
	Serial pc(SERIAL_TX,SERIAL_RX,115200);
	//pc.printf("hey");
	std::vector<Task> v;
	led1=1;
	led2=1;
	led3=1;

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
motionCtrl test=motionCtrl(0.0,0.0,0.0,v);

while(1)
{
	//test.s1.update();
	//a=test.s1.get_val();
	//pc.printf("x=%f  y=%f angle=%f Pwml= %f Pwmr= %f\n Time: %f\n",test.Posx,test.Posy,test.Angle,test.sPwm_L,test.sPwm_R,test.affiche);
	//pc.printf("x=%f \n y=%f angle=%f \n",test.Posx,test.Posy,test.Angle);
	pc.printf("x=%f \n y=%f angle=%f \n s1=%d\n s2=%d\n",test.Posx,test.Posy,test.Angle,test.s1.get_val(),test.s2.get_val());
	//pc.printf("x=%f \n y=%f \n s1=%f \n s2= %f\n",test.Posx,test.Posy,test.sPwm_L,test.sPwm_R);
	wait(1);
}

}
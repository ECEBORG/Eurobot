#include "motionCtrl.h"
#include "config.h"
#include "QEI.h"
#include "pinOut.h"
#include "utils.h"
#include <time.h>


motionCtrl::motionCtrl(float m_Posx,float m_Posy,float m_Angle,std::vector<Task> m_Liste) :


    //Motor_l(MOTOR_L_PWM, MOTOR_L_DIR, MOTOR_DIR_LEFT_FORWARD),
   // Motor_r(MOTOR_R_PWM, MOTOR_R_DIR, MOTOR_DIR_RIGHT_FORWARD),
s1(SHARP1),
s2(SHARP2),
Liste(m_Liste),
pid_dist_(PID_DIST_P, PID_DIST_I, PID_DIST_D, ASSERV_DELAY),
pid_angle_(PID_ANGLE_P, PID_ANGLE_I, PID_ANGLE_D, ASSERV_DELAY),
enc_l(ENC_L_DATA1,ENC_L_DATA2,NC,200),
enc_r(ENC_R_DATA1,ENC_R_DATA2,NC,200)
{

    pid_dist_.setInputLimits(-3*1000, 3*1000);  // dist (mm)
    pid_dist_.setOutputLimits(-PID_DIST_MAX_OUPUT, PID_DIST_MAX_OUPUT);  // motor speed (~pwm)
    pid_dist_.setMode(AUTO_MODE);  // AUTO_MODE or MANUAL_MODE
    pid_dist_.setBias(0); // magic *side* effect needed for the pid to work, don't comment this
    pid_dist_.setInterval(ASSERV_DELAY);
    pid_dist_.setSetPoint(0);
    this->pidDistSetGoal(0);  // pid's error

    pid_angle_.setInputLimits(-M_PI, M_PI);  // angle (rad). 0 toward, -pi on right, +pi on left
    pid_angle_.setOutputLimits(-PID_ANGLE_MAX_OUPUT, PID_ANGLE_MAX_OUPUT);  // motor speed (~pwm). -1 right, +1 left, 0 nothing
    pid_angle_.setMode(AUTO_MODE);  // AUTO_MODE or MANUAL_MODE
    pid_angle_.setBias(0); // magic *side* effect needed for the pid to work, don't comment this
    pid_angle_.setSetPoint(0);
    pid_dist_.setInterval(ASSERV_DELAY);
    this->pidAngleSetGoal(0);  // pid's error

    MOTOR_R_PWM=0.0;
    MOTOR_L_PWM=0.0;
    isFinished=false;
        ///avancer
    MOTOR_R_DIR_2= 0;
    MOTOR_R_DIR_1= 1;
    MOTOR_L_DIR_2= 1;
    MOTOR_L_DIR_1= 0;

    enc_l_val=0;
    enc_r_val=0;

    sPwm_L=0;
    sPwm_R=0;
    last_Pwm_l=0.0;
    last_Pwm_r=0.0;
    Dist_last=0.0;
    angl_goal=0.0;
    Dist=0.0;
    Cap=0.0;
    Cap_last=0.0;
    x_goal=Liste.front().x;
    y_goal=Liste.front().y;
    angl_goal=Liste.front().angle;
    Posx=m_Posx;
    Posy=m_Posy;
    Angle=m_Angle;

    timer.start();
    asserv_ticker_ = new Ticker;
    asserv_ticker_->attach(callback(this, &motionCtrl::asserv), ASSERV_DELAY);
}

float recalib(float Cap)
{
   if(Cap>M_PI)Cap-=2*M_PI;
   if(Cap<(-M_PI))Cap+=2*M_PI;
   return Cap;
}
void motionCtrl::DefineDistCap()
{
   float Xerr=x_goal-Posx;
   float Yerr=y_goal-Posy;
   float Aerr=angl_goal-Angle;

   if(Liste.front().type=="MOVE_POS")
   {
    this->Dist=sqrt((Xerr*Xerr)+(Yerr*Yerr));
    this->Cap= (atan2(Yerr,Xerr)-Angle);
    this->Cap=recalib(this->Cap);
    if(this->Cap>(M_PI/2)|| this->Cap<(-M_PI/2))
    {
        this->Dist=-this->Dist;
        this->Cap+=M_PI;
        this->Cap=recalib(this->Cap);
    }
        if ((ABS(this->Dist) < MC_TARGET_TOLERANCE_DIST) )//&& (ABS(cur_speed) < MC_TARGET_TOLERANCE_SPEED))
        {

            this->isFinished = true;
        }
    }
    if(Liste.front().type=="TURN_N_GO")
    {
        if(turning)
        {
            this->Cap = std_rad_angle(Aerr);
            Dist = 0;  //Dist=sqrt((Xerr*Xerr)+(Yerr*Yerr)) * cos(atan2(Yerr,Xerr)-Angle);
            if ((ABS(this->Cap) < MC_TARGET_TOLERANCE_ANGLE) )//&& (ABS(cur_speed_ang) < MC_TARGET_TOLERANCE_ANG_SPEED))
                turning=false;
        }
        else
        {
            Dist=sqrt((Xerr*Xerr)+(Yerr*Yerr));
            Cap= (atan2(Yerr,Xerr)-Angle);
            Cap=recalib(Cap);
            if(Cap>(M_PI/2)|| Cap<(-M_PI/2))
            {
                Dist=-Dist;
                Cap+=M_PI;
                Cap=recalib(Cap);
            }
            if ((ABS(Dist) < MC_TARGET_TOLERANCE_DIST) )//&& (ABS(cur_speed) < MC_TARGET_TOLERANCE_SPEED))
            {

                isFinished = true;
            }
        }
    }
    else if(Liste.front().type=="MOVE_ANG")
    {
        Cap = std_rad_angle(Aerr);
        Dist = Dist=sqrt((Xerr*Xerr)+(Yerr*Yerr)) * cos(atan2(Yerr,Xerr)-Angle);
        if ((ABS(Cap) < MC_TARGET_TOLERANCE_ANGLE) && (ABS(this->Dist) < MC_TARGET_TOLERANCE_DIST) )//&& (ABS(cur_speed_ang) < MC_TARGET_TOLERANCE_ANG_SPEED))
            isFinished=true;

    }
    else if(Liste.front().type=="WAIT")
    {
        Cap=0;
        Dist=0;
        isFinished = true;

    }
    

    
}
void motionCtrl::updateTask()
{
 DefineDistCap();
 this->pidDistSetGoal(Dist);
 this->pidAngleSetGoal(Cap);

}
void motionCtrl::pidDistSetGoal(float goal) {
    pid_dist_goal_ = goal;
}

void motionCtrl::pidAngleSetGoal(float goal) {
    pid_angle_goal_ = goal;
}
void motionCtrl::fetchEncodersValue() {
    enc_l_last = enc_l_val;
    enc_r_last= enc_r_val;

    enc_l_val = enc_l.getPulses();
    enc_r_val = enc_r.getPulses();
}

void motionCtrl::update_Pos()
{
    float cur_x=0.0,cur_y=0.0,cur_angle=0.0;
    float last_x=0.0,last_y=0.0,last_angla=0.0;
    float Speed=0.0;
    float Speed_angle=0.0;
    float S_left=0.0;
    float S_right=0.0;


    S_left=(perim_wheel/reso_encoder)*(enc_l_val- enc_l_last);
    S_right=((perim_wheel)/reso_encoder)*(enc_r_val- enc_r_last);
    Speed=(S_right+S_left)/2;
    Speed_angle=((S_left-S_right)/entraxe);
    cur_angle=(Speed_angle+Angle);
    cur_x=Speed*cosf(cur_angle)+Posx;
    cur_y=Speed*sinf(cur_angle)+Posy;
    Posx=cur_x;
    Posy=cur_y;
    Angle=cur_angle;



}

float  motionCtrl::Dist_Consigne()
{
    //float Dist;
    float VRobot=((Dist_last-Dist)/ASSERV_DELAY);
    float Dfrein=(VRobot*VRobot)/(2*MAX_DECEL);
    float Vconsigne=0;
    if(Dist<Dfrein)
    {
        Vconsigne=VRobot-(MAX_DECEL*ASSERV_DELAY);
    }
    else if(VRobot<MAX_V)
    {
        Vconsigne=VRobot+(MAX_A*ASSERV_DELAY);
    }
    else
    {
        Vconsigne=MAX_V;
    }
    return Vconsigne;
}
float  motionCtrl::Ang_Consigne()
{
    float VRobot=((Cap_last-Cap)/ASSERV_DELAY);
    float Dfrein=(VRobot*VRobot)/(2*MAX_DECEL_A);
    float Vconsigne=0;
    if(Cap<Dfrein)
    {
        Vconsigne=VRobot-(MAX_DECEL_A*ASSERV_DELAY);
    }
    else if(VRobot<MAX_V_A)
    {
        Vconsigne=VRobot+(MAX_A_A*ASSERV_DELAY);
    }
    else
    {
        Vconsigne=MAX_V_A;
    }
    return Vconsigne;
}
float constrain(float val, float min, float max)
{
    if(val<min)return min;
    else if(val >max)return max;
    else return val;
}



void motionCtrl::consigne()
        {
        float VRobot=((Dist_last-Dist)/ASSERV_DELAY);
        //distance_restante = position_consigne - position_courante; 
        //if (abs(distance_restante) < 1000) flag_asservissement_position_fini=1; 
        //if (!flag_asservissement_position_fini)  
        //{ 
        //float vitesse_consigne=0;
        if (this->Dist > 0)
            {
                this->vitesse_consigne=MAX_VITESSE;
            }
        else if (this->Dist < 0) this->vitesse_consigne=-MAX_VITESSE; 
        else if (abs(VRobot*(MAX_VITESSE/(2*ASSERV_DELAY)))>abs(this->Dist/VRobot)) this->vitesse_consigne=1; 
        else this->vitesse_consigne=2;
        //} 
        //else  vitesse_consigne=0; 
        }



float motionCtrl::appConsigne(float pwm)
{

    float VRobot=((Dist_last-Dist)/ASSERV_DELAY);
    if ( VRobot < vitesse_consigne ) pwm+=PWM_STEP; 
    if ( VRobot > vitesse_consigne ) pwm-=PWM_STEP; 
    
    return pwm;
}
float  motionCtrl::update_Motor(float sPwm, char cote)
{

    if(cote=='r')
    {
        float last_sPwm_= last_Pwm_r;
        if (abs(sPwm) < 0.05)//PWM_IS_ALMOST_ZERO peut etre a enlever
        {
            sPwm = 0.0;
            last_sPwm_ = 0.0;
        }
        else
        {
            //float current = last_sPwm_;

            //sPwm = constrain(sPwm, -1, 1);

              // step the raw value
                /*if (abs(sPwm - current) > PWM_STEP)
                {
                    if (sPwm > current)
                        sPwm = current + PWM_STEP;
                    else
                        sPwm = current - PWM_STEP;
                }  */
                
            //sPwm=this->appConsigne(sPwm);

            last_Pwm_r= sPwm;
            sPwm = SIGN(sPwm) * map(ABS(sPwm), 0, 1, PWM_MIN, PWM_MAX);
            //sPwm = SIGN(sPwm) * map(ABS(sPwm), 0, 1, PWM_MIN, PWM_MAX);
            if(sPwm>0.0)
            {
                MOTOR_R_DIR_2= 0;
                MOTOR_R_DIR_1= 1;
            }
            else
                {   sPwm=-sPwm;
                    MOTOR_R_DIR_2= 1;
                    MOTOR_R_DIR_1= 0;
                }
            }
        }
        else 
        {
            float last_sPwm_= last_Pwm_l;
            if (abs(sPwm) < 0.05)//PWM_IS_ALMOST_ZERO
            {
                sPwm = 0.0;
                last_sPwm_ = 0.0;
            }
            else
            {
                //float current = last_sPwm_;

                //sPwm = constrain(sPwm, -1, 1);

                //sPwm=this->appConsigne(sPwm);
                last_Pwm_l= sPwm;
                sPwm = SIGN(sPwm) * map(ABS(sPwm), 0, 1, PWM_MIN, PWM_MAX);
                //sPwm = SIGN(sPwm) * map(ABS(sPwm), 0, 1, PWM_MIN, PWM_MAX);
                if(sPwm>0.0)
                {
                    MOTOR_L_DIR_2= 1;
                    MOTOR_L_DIR_1= 0;
                }
                else
                    {   sPwm=-sPwm;
                        MOTOR_L_DIR_2= 0;
                        MOTOR_L_DIR_1= 1;
                    }
            }
        }

    return sPwm;
}



        
        void motionCtrl::Compute_PID()
        {




            Dconsign=Dist_Consigne();
            AConsign=Ang_Consigne();

            if (isFinished)
            {
                Dconsign = 0;
                AConsign = 0;
            }
            else
            {
                pid_dist_.setProcessValue(-Dconsign);
                pid_dist_out_ = pid_dist_.compute();

                pid_angle_.setProcessValue(AConsign);
                pid_angle_out_ = pid_angle_.compute();

            }
            sPwm_L=pid_dist_out_;
            sPwm_R=pid_angle_out_;

            float mot_l_val = pid_dist_out_ - pid_angle_out_;
            float mot_r_val = pid_dist_out_ + pid_angle_out_;

            Dist_last=Dist;
            Cap_last=Cap;
            last_Pwm_l=update_Motor(mot_l_val,'l');
            last_Pwm_r=update_Motor(mot_r_val,'r');

        }
        void motionCtrl::MAJTask()
        {
            Liste.erase(Liste.begin());
            if(Liste.size()==0)
            {
                //Posy=8888;
                asserv_ticker_->detach();
                MOTOR_L_PWM=0;
                MOTOR_R_PWM=0;

            }
            else if(Liste.front().type=="MOVE_POS")
            {
             x_goal=Liste.front().x;
             y_goal=Liste.front().y;
             angl_goal=Liste.front().angle;
         }
    /*else if(Liste.front().type=="TURN_N_GO")
    {
        if(turning)
        {
             Cap = std_rad_angle(Aerr);
             Dist = Dist=sqrt((Xerr*Xerr)+(Yerr*Yerr)) * cos(atan2(Yerr,Xerr)-Angle);
             if ((ABS(Cap) < MC_TARGET_TOLERANCE_ANGLE) )//&& (ABS(cur_speed_ang) < MC_TARGET_TOLERANCE_ANG_SPEED))
                isFinished=true;
        }
    }*/
         else if(Liste.front().type=="MOVE_ANG")
         {
            x_goal=Posx;
            y_goal=Posy;
            angl_goal=Liste.front().angle;
        }
    }


    void motionCtrl::sharp()
    {
     s2.update();
     s1.update();
        //sPwm_R=s2.get_val();
       //sPwm_L=s1.get_val();
     if(Cap>(-M_PI/4)&&Cap<(M_PI/4))
     {
        if((s2.get_val()<120 && Liste.front().type=="MOVE_POS") || (s2.get_val()<120 && Liste.front().type=="TURN_N_GO" && turning==false))
        {
            cptsharp++;
            if(cptsharp>3)
            {
                x_goal=100*cos(Angle+(-M_PI/2));
                y_goal=100*sin(Angle+(-M_PI/2));
                angl_goal=Angle+(-M_PI/2);
                Task t1("WAIT",0.0,0.0,0.0);
                //turning=true;
                Liste.insert(Liste.begin(),t1);
                cptsharp=0;
            }
        }
        else
        {
            cptsharp=0;
        }

    }


       //   isFinished=true;
}

void motionCtrl::addtask(Task t)
{
    Task s2("WAIT",0.0,0.0,0.0);
    this->Liste.push_back(s2);
    this->Liste.push_back(t);
}

void motionCtrl::adjust_speed_motors()
{
    float vitess_l= this->enc_l_val- this->enc_l_last;
    float vitess_r=this->enc_r_val- this->enc_r_last;

    float pwm_speed_l=vitess_l/this->last_Pwm_l;
    float pwm_speed_r=vitess_r/this->last_Pwm_r;
    this->affiche=(pwm_speed_l-pwm_speed_r)/10000;
    if (vitess_l-vitess_r>0)
    {
        MOTOR_R_PWM=sPwm_R-0.01;
        MOTOR_L_PWM=sPwm_L+0.01;

    }
    else 
    {
        MOTOR_L_PWM=sPwm_L-0.01;
        MOTOR_R_PWM=sPwm_R+0.01;
    }
    
    

}



void motionCtrl::asserv()
{
    //bool ret=false;
    //Posx++;
  this->fetchEncodersValue();
  this->update_Pos();
  this->updateTask();

  
  if ((ABS(this->Dist) < MC_TARGET_TOLERANCE_DIST) )//&& (ABS(cur_speed) < MC_TARGET_TOLERANCE_SPEED))
        {

            this->isFinished = true;
        }


  //this->sharp();
  //sPwm_R=s2.get_val();
  /*if(turning)
  {
    affiche="toto";
}*/

if (isFinished)
{   
    this->MAJTask();
    isFinished=false;
}
/*else
{
    this->adjust_speed_motors();
}*/

}

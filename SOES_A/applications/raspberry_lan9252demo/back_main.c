#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"
#include <bcm2835.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include <stdlib.h>
#include "DobotType.h"
#include "DobotDll.h"

/* Application variables */
_Objects    Obj;

uint32_t ret;
bool isQueued = true;
uint64_t queuedCmdIndex;
EMotor eMotor;
uint64_t currentIndex;
InfraredPort infraredPort = IF_PORT_GP4;
uint8_t value = 0;
time_t start_time, end_time;
Pose pose;

void cb_get_inputs (void)
{
   if (Obj.Buttons.Connection_Start == 1 && Obj.States.Connection == 0)
   {
       if ((ret = ConnectDobot(NULL, 115200, NULL, NULL, NULL)) != DobotConnect_NoError) 
       {
         printf("connect error\n");
       }
       printf("接続開始\n");
       Obj.States.Connection = 1;
   }
/*
   if (Obj.Buttons.Conveyor_Start == 1 && Obj.States.Conveyor == 0  && Obj.States.Connection == 1)
   {
       eMotor.index = 0;
       eMotor.isEnabled = 1;
       eMotor.speed = 5000;
       SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
       printf("コンベアの起動\n");
       Obj.States.Conveyor = 1;
   }

   if (Obj.Buttons.Conveyor_Stop == 1 && Obj.States.Conveyor == 1  && Obj.States.Connection == 1)
   {
       eMotor.isEnabled = 0;
       SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
       printf("コンベアの停止\n");
       Obj.States.Conveyor = 0;
   }
   */

   if (Obj.Buttons.Homing == 1  && Obj.States.Connection == 1)
   {
      HOMEParams homeParams = {200, 0, 50, 0};
      HOMECmd homeCmd;
      SetQueuedCmdClear();
      printf("Clear Queue\n");
      SetHOMEParams(&homeParams, isQueued, &queuedCmdIndex);
      SetHOMECmd(&homeCmd, isQueued, &queuedCmdIndex);
      while(1) 
      {
         GetPose(&pose);
         ret = GetQueuedCmdCurrentIndex(&currentIndex);
         if (ret == DobotCommunicate_NoError && currentIndex >= queuedCmdIndex) 
         {
               break;
         }
      }
      printf("ホーミング\n");
      Obj.Buttons.Homing = 0;
   }

   /*
   if (Obj.Buttons.System_Start == 1 && Obj.States.System == 0  && Obj.States.Connection == 1)
   {
      SetInfraredSensor(1, infraredPort, 1);
      printf("赤外線センサーのON\n");
      Obj.States.System = 1;
   }

   if (Obj.Buttons.System_Stop == 1 && Obj.States.System == 1  && Obj.States.Connection == 1)
   {
      Obj.States.System = 0;
      // thread2でdobotと繋いでいる場合は，connect errorになって，OFFできない
      // 従って，thread2のDisconnectDobot処理まで待つ
      sleep(1);
      SetInfraredSensor(0, infraredPort, 1);
      printf("赤外線センサーのOFF\n");
   }
   */

   if (Obj.Buttons.Connection_Stop == 1 && Obj.States.Connection == 1)
   {
      printf("Disconnect Dobot, Bye!\n");
      DisconnectDobot();
      Obj.States.Connection = 0;
      pose.x = 0;
      pose.y = 0;
      pose.z = 0;
      pose.r = 0;
   }
}

void cb_set_outputs (void)
{
      Obj.Positions.Position_X = pose.x;
      Obj.Positions.Position_Y = pose.y;
      Obj.Positions.Position_Z = pose.z;
      Obj.Positions.Position_R = pose.r;
}

void call_back(void)
{
   printf("Test CallBack. \n");
   ESC_read(ESCREG_ALSTATUS ,(void *)&ESCvar.App.state, sizeof(ESCvar.App.state));
   ESCvar.App.state = etohs(ESCvar.App.state);
   printf("ESCvar.App.state: %u \n", ESCvar.App.state);
}

void main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0",
      .use_interrupt = 0,
      .watchdog_cnt = 150,
      .set_defaults_hook = call_back,
      .pre_state_change_hook = NULL,
      .post_state_change_hook = NULL,
      .application_hook = NULL,
      .safeoutput_override = NULL,
      .pre_object_download_hook = NULL,
      .post_object_download_hook = NULL,
      .rxpdo_override = NULL,
      .txpdo_override = NULL,
      .esc_hw_interrupt_enable = NULL,
      .esc_hw_interrupt_disable = NULL,
      .esc_hw_eep_handler = NULL,
      .esc_check_dc_handler = NULL,
   };

   printf ("Hello Main\n");
   bcm2835_init();
   ecat_slv_init (&config);
   while (1)
   {
      ecat_slv();
   }

}

void easy_system()
{
   while(1)
   {
      if (Obj.States.Connection == 1) GetPose(&pose);
      /*
      if (Obj.States.System == 1)
      {
         // 物体を検知するまで無限ループ
         GetInfraredSensor(infraredPort, &value);
         if (value == 1)
         {
            // アームの速度と加速度:vx,vy,vz,vr,ax,ay,az,ar
            PTPJointParams ptpJointParams={{200, 200, 200, 200}, {200, 200, 200, 200}};
            // 速度比率と加速度比率
            PTPCommonParams ptpCommonParams={100, 100};
            // 移動先の位置
            PTPCmd ptpCmd1={PTPMOVLXYZMode, 200, 0, 50, 0};
            PTPCmd ptpCmd2={PTPMOVLXYZMode, 266.047, -6.515, 14.562, -1.402};
            PTPCmd ptpCmd3={PTPMOVLXYZMode, 200, 50, 50, 0};

            SetPTPJointParams(&ptpJointParams, isQueued, &queuedCmdIndex);
            SetPTPCommonParams(&ptpCommonParams, isQueued, &queuedCmdIndex);
            
            start_time = time(NULL);

            while (SetPTPCmd(&ptpCmd1, isQueued, &queuedCmdIndex) == DobotCommunicate_BufferFull);
            while (SetEndEffectorSuctionCup(1, 1, isQueued, &queuedCmdIndex) == DobotCommunicate_BufferFull);
            while (SetPTPCmd(&ptpCmd2, isQueued, &queuedCmdIndex) == DobotCommunicate_BufferFull);
            while (SetPTPCmd(&ptpCmd3, isQueued, &queuedCmdIndex) == DobotCommunicate_BufferFull);
            while (SetEndEffectorSuctionCup(0, 0, isQueued, &queuedCmdIndex) == DobotCommunicate_BufferFull);

            while(1)
            {
               ret = GetQueuedCmdCurrentIndex(&currentIndex);
               if (ret == DobotCommunicate_NoError && currentIndex >= queuedCmdIndex) {
                  break;
               }
            }

            end_time = time(NULL);
            printf("移動時間:%lf\n", difftime(end_time, start_time));
         }
      }
      else
      {
         sleep(1);
      }
      */
   }
}

int main (void)
{
   printf ("Hello Main\n");
   pthread_t thread1;
   pthread_t thread2;

   pthread_create(&thread1, NULL, (void *)main_run, NULL);
   pthread_create(&thread2, NULL, (void *)easy_system, NULL);

   pthread_join(thread1, NULL);
   pthread_join(thread2, NULL);
   return 0;
}

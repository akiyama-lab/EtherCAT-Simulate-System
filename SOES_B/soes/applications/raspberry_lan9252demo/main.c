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

#define MAX_Seq 1000

/* Application variables */
_Objects    Obj;

uint8_t SeqTable[MAX_Seq];
uint8_t current_index = 1;
uint8_t local_index;
uint32_t ret;
bool isQueued = true;
uint64_t queuedCmdIndex = 0;
uint64_t executedCmdIndex = 0;
EMotor eMotor;
uint64_t currentIndex;
InfraredPort infraredPort = IF_PORT_GP4;
uint8_t value = false;
time_t start_time, end_time;
Pose pose;

void cb_get_inputs (void)
{
   if (Obj.Buttons.System_Start == 1 && Obj.States.System == 0)
   {
       if ((ret = ConnectDobot(NULL, 115200, NULL, NULL, NULL)) != DobotConnect_NoError) 
       {
         printf("connect error\n");
       }
       printf("接続開始\n");
       Obj.States.System = 1;
       for(int i = 0; i <= current_index; i++)
       {
           SeqTable[i] = 0;
       }
       current_index = 1;   // シーケンステーブルの初期化
       local_index = 0;
       SetInfraredSensor(1, infraredPort, 1);
       value = false;
       eMotor.index = 0;
       eMotor.isEnabled = 1;
       eMotor.speed = 5000;
       SetQueuedCmdStartExec();
       SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
       SetQueuedCmdStopExec();
       printf("コンベアの起動\n");
   }

   // SLAVE Bのセンサーが青物体に対する処理が終わって，MASTERによってコンベアを起動する
   if (Obj.Buttons.Conveyor_Start == 1 && Obj.States.System == 1 && Obj.States.Homing == 0)
   {
       eMotor.isEnabled = 1;
       SetQueuedCmdStartExec();
       SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
       SetQueuedCmdStopExec();
       printf("コンベアの起動\n");
   }

   // SLAVE Bのセンサーが青物体を検知し，MASTERによってコンベアを停止する
   if (Obj.Buttons.Conveyor_Stop == 1 && Obj.States.System == 1 && Obj.States.Homing == 0)
   {
       eMotor.isEnabled = 0;
       SetQueuedCmdStartExec();
       SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
       SetQueuedCmdStopExec();
       printf("コンベアの停止\n");
   }
   

   if (Obj.Buttons.Homing == 1  && Obj.States.Homing == 0 && Obj.States.System == 1)
   {
      HOMEParams homeParams = {200, 0, 70, 0};
      HOMECmd homeCmd;
      SetQueuedCmdClear();
      printf("Clear Queue\n");
      SetHOMEParams(&homeParams, isQueued, &queuedCmdIndex);
      SetQueuedCmdStartExec();
      SetHOMECmd(&homeCmd, isQueued, &queuedCmdIndex);
      SetQueuedCmdStopExec();
      printf("ホーミング\n");
      Obj.States.Homing = 1;
   }


   if (Obj.Buttons.System_Stop == 1 && Obj.States.System == 1) 
   {
      eMotor.isEnabled = 0;
      SetQueuedCmdStartExec();
      SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
      printf("コンベア停止\n");
      SetQueuedCmdStopExec();
      SetInfraredSensor(0, infraredPort, 1);
      printf("Disconnect Dobot, Bye!\n");
      DisconnectDobot();
      Obj.States.System = 0;
      pose.x = 0;
      pose.y = 0;
      pose.z = 0;
      pose.r = 0;
      //printf("Connection Stop red counter: %d \n",red_counter);
   }
}

void cb_set_outputs (void)
{
      Obj.Positions.Position_X = pose.x;
      Obj.Positions.Position_Y = pose.y;
      Obj.Positions.Position_Z = pose.z;
      Obj.Positions.Position_R = pose.r;
}

/*
void call_back(void)
{
   printf("Test CallBack. \n");
   ESC_read(ESCREG_ALSTATUS ,(void *)&ESCvar.App.state, sizeof(ESCvar.App.state));
   ESCvar.App.state = etohs(ESCvar.App.state);
   printf("ESCvar.App.state: %u \n", ESCvar.App.state);
}
*/

void main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0",
      .use_interrupt = 0,
      .watchdog_cnt = 150,
      //.set_defaults_hook = call_back,
      .set_defaults_hook = NULL,
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
      if (Obj.States.System == true)
      {
        GetPose(&pose);
        GetQueuedCmdCurrentIndex(&executedCmdIndex);
        if (executedCmdIndex >= queuedCmdIndex && Obj.States.Homing == true) Obj.States.Homing = 0;
        // ローカルのシーケンス表を作る
        if(current_index == Obj.Counters.Sequence_Number)
        {
            // Red:0; Green:1; Blue:2
            if(Obj.Counters.Red == true) SeqTable[current_index] = 1;
            if(Obj.Counters.Green == true) SeqTable[current_index] = 2;
            if(Obj.Counters.Blue == true) SeqTable[current_index] = 3;
            printf("シーケンス番号：%d, 色：%d\n",current_index, SeqTable[current_index]);
            current_index++;
        }
/*
        if ( Obj.Counters.Red == true && Obj.Counters.Sequence_Number != SeqTable[current_index])
        {
            current_index++;
            SeqTable[current_index] = Obj.Counters.Sequence_Number;
            printf("current_index: %d, Sequence_Number:%d\n", current_index, SeqTable[current_index]);
        }
*/
        if(value == false) GetInfraredSensor(infraredPort, &value);
        else if (Obj.States.Homing == false)    // ホーミング中には吸着動作をしない
        {
            if (local_index < Obj.Counters.Sequence_Number) local_index++;
            printf("センサAが検知した物体の個数: %d\n", local_index);
            if (SeqTable[local_index] == 1)
            {
                printf("%d個目に赤色物体を検知しました！\n", SeqTable[local_index]);
            //for(int i = 0; i <= current_index; i++)
            //{
            //if (SeqTable[i] == local_index && local_index != 0)
            //{
                // 物体を検知するまで無限ループ
                eMotor.isEnabled = 0;
                SetQueuedCmdStartExec();
                SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
                SetQueuedCmdStopExec();
                printf("コンベアの停止\n");
                // アームの速度と加速度:vx,vy,vz,vr,ax,ay,az,ar
                PTPJointParams ptpJointParams={{200, 200, 200, 200}, {200, 200, 200, 200}};
                // 速度比率と加速度比率
                PTPCommonParams ptpCommonParams={100, 100};
                // 移動先の位置
                PTPCmd ptpCmd1={PTPMOVLXYZMode, 200, 0, 70, 0};
                PTPCmd ptpCmd2={PTPMOVLXYZMode, 245.553, 46.959, 50, 10.827};
                PTPCmd ptpCmd3={PTPMOVLXYZMode, 245.553, 46.959, 14.125, 10.827};
                PTPCmd ptpCmd4={PTPMOVLXYZMode, 245.553, 46.959, 50, 10.827};
                PTPCmd ptpCmd5={PTPMOVLXYZMode, 155.559, -142.074, 90, -42.406};
                PTPCmd ptpCmd6={PTPMOVLXYZMode, 200, 0, 70, 0};

                start_time = time(NULL);

                SetQueuedCmdStartExec();
                SetPTPJointParams(&ptpJointParams, isQueued, &queuedCmdIndex);
                SetPTPCommonParams(&ptpCommonParams, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd1, isQueued, &queuedCmdIndex);
                SetEndEffectorSuctionCup(1, 1, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd2, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd3, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd4, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd5, isQueued, &queuedCmdIndex);
                SetEndEffectorSuctionCup(0, 0, isQueued, &queuedCmdIndex);
                SetPTPCmd(&ptpCmd6, isQueued, &queuedCmdIndex);

                while (executedCmdIndex < queuedCmdIndex)
                {
                   GetPose(&pose);
                   GetQueuedCmdCurrentIndex(&executedCmdIndex);
                }
                SetQueuedCmdStopExec();

                end_time = time(NULL);
                printf("移動時間:%lf\n", difftime(end_time, start_time));
                eMotor.isEnabled = 1;
                SetQueuedCmdStartExec();
                SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
                SetQueuedCmdStopExec();
                printf("コンベアの起動\n");
            //}
            }
            while(1)
            {
                GetInfraredSensor(infraredPort, &value);
                if (value == false) break;
            }
        }
      }
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

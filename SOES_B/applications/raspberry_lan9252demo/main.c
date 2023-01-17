#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"
#include <bcm2835.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>

// C言語のDOBOT APIライブラリ
#include "DobotType.h"
#include "DobotDll.h"

#define MAX_Seq 1000

// ホストコントローラのアプリケーションオブジェクト
_Objects    Obj;

// ロボットアーム用の各種変数
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

// 出力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_get_inputs (void)
{
    // SLAVE Bのシステム起動処理
    if (Obj.Buttons.System_Start == true && Obj.States.System == false)
    {
        // ロボットアームBとの接続
        if ((ret = ConnectDobot(NULL, 115200, NULL, NULL, NULL)) != DobotConnect_NoError) 
        {
            printf("connect error\n");
        }
        // 変数の処理化
        printf("接続開始\n");
        for(int i = 0; i <= current_index; i++)
        {
            SeqTable[i] = 0;
        }
        current_index = 1;
        local_index = 0;
        // センサBの起動
        SetInfraredSensor(1, infraredPort, 1);
        value = false;
        // コンベアの初期化
        eMotor.index = 0;
        eMotor.isEnabled = true;
        eMotor.speed = 5000;
        // コンベアの起動
        SetQueuedCmdStartExec();
        SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
        SetQueuedCmdStopExec();
        printf("コンベアの起動\n");
        // SLAVE Aのシステム状態をTrueにする
        Obj.States.System = true;
    }

    // MASTERによるコンベアの起動制御
    // SLAVE AのセンサAが青い物体を検知した際に使う
    if (Obj.Buttons.Conveyor_Start == true && Obj.States.System == true && Obj.States.Homing == false)
    {
            eMotor.isEnabled = true;
            SetQueuedCmdStartExec();
            SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
            SetQueuedCmdStopExec();
            printf("コンベアの起動\n");
    }

    // MASTERによるコンベアの停止制御
    // SLAVE AのロボットアームAが青い物体を拾う処理が終わった際に使う
    if (Obj.Buttons.Conveyor_Stop == true && Obj.States.System == true && Obj.States.Homing == false)
    {
            eMotor.isEnabled = false;
            SetQueuedCmdStartExec();
            SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
            SetQueuedCmdStopExec();
            printf("コンベアの停止\n");
    }

    // SLAVE Bのホーミング処理
    if (Obj.Buttons.Homing == true  && Obj.States.Homing == false && Obj.States.System == true)
    {
        // 変数の初期化
        HOMEParams homeParams = {200, 0, 70, 0};
        HOMECmd homeCmd;
        // ロボットアームのコマンドキューをクリアする
        SetQueuedCmdClear();
        printf("Clear Queue\n");
        // ホーミングする位置を設定する
        SetHOMEParams(&homeParams, isQueued, &queuedCmdIndex);
        // ホーミングの実行
        SetQueuedCmdStartExec();
        SetHOMECmd(&homeCmd, isQueued, &queuedCmdIndex);
        SetQueuedCmdStopExec();
        printf("ホーミング\n");
        // ホーミングの状態をTureにする
        Obj.States.Homing = true;
    }

    // SLAVE Bのシステム停止処理
    if (Obj.Buttons.System_Stop == true && Obj.States.System == true) 
    {
        // コンベアの停止
        eMotor.isEnabled = false;
        SetQueuedCmdStartExec();
        SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
        printf("コンベア停止\n");
        SetQueuedCmdStopExec();
        // センサBの停止
        SetInfraredSensor(0, infraredPort, 1);
        printf("Disconnect Dobot, Bye!\n");
        // ロボットアームBとの切断
        DisconnectDobot();
        // ロボットアームB位置の初期化
        pose.x = 0;
        pose.y = 0;
        pose.z = 0;
        pose.r = 0;
        // SLAVE Bのシステム状態をFalseにする
        Obj.States.System = 0;
    }
}

// 入力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_set_outputs (void)
{
    // ロボットアームB位置のフラグの出力
    Obj.Positions.Position_X = pose.x;
    Obj.Positions.Position_Y = pose.y;
    Obj.Positions.Position_Z = pose.z;
    Obj.Positions.Position_R = pose.r;
}

// SOESのプロセスデータをRead/Writeする処理
void main_run (void * arg)
{
   static esc_cfg_t config =
   {
      .user_arg = "rpi3,cs0",
      .use_interrupt = 0,
      .watchdog_cnt = 150,
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

// ロボットアームBが赤い物体を拾う操作
void easy_system()
{
    while(1)
    {
        // SLAVE Bのシステム状態を確認する
        if (Obj.States.System == true)
        {
            // ロボットアームBの位置情報を取得する
            GetPose(&pose);
            // ホーミングが終わると，ホーミング状態をFalseにする
            GetQueuedCmdCurrentIndex(&executedCmdIndex);
            if (executedCmdIndex >= queuedCmdIndex && Obj.States.Homing == true) Obj.States.Homing = false;
            // 物体がカメラの下に通ったら，その際のシーケンス番号と色番号をSeqTableに格納する
            if(current_index == Obj.Counters.Sequence_Number)
            {
                // Red:0; Green:1; Blue:2
                if(Obj.Counters.Red == true) SeqTable[current_index] = 1;
                if(Obj.Counters.Green == true) SeqTable[current_index] = 2;
                if(Obj.Counters.Blue == true) SeqTable[current_index] = 3;
                printf("シーケンス番号：%d, 色：%d\n",current_index, SeqTable[current_index]);
                current_index++;
            }
            // 物体が来るまで，センサBがずっと検知し続ける
            if(value == false) GetInfraredSensor(infraredPort, &value);
            // ホーミング中には物体を拾う動作をしない
            else if (Obj.States.Homing == false)
            {
                // カメラの下に通らず途中でコンベアに置く時，または手などを検知した時はカウントさせない
                if (local_index < Obj.Counters.Sequence_Number) local_index++;
                printf("センサAが検知した物体の個数: %d\n", local_index);
                // 次に来る物体が赤の場合
                if (SeqTable[local_index] == 1)
                {
                    printf("%d個目に赤色物体を検知しました！\n", SeqTable[local_index]);
                    // コンベアを停止する
                    eMotor.isEnabled = false;
                    SetQueuedCmdStartExec();
                    SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
                    SetQueuedCmdStopExec();
                    printf("コンベアの停止\n");

                    // 物体を拾う動作のため変数設定
                    // アームの速度と加速度:vx,vy,vz,vr,ax,ay,az,ar
                    PTPJointParams ptpJointParams={{200, 200, 200, 200}, {200, 200, 200, 200}};
                    // 速度比率と加速度比率
                    PTPCommonParams ptpCommonParams={100, 100};
                    // 物体を拾う時の移動先の位置
                    // 初期位置
                    PTPCmd ptpCmd1={PTPMOVLXYZMode, 200, 0, 70, 0};
                    // 物体の真上の位置
                    PTPCmd ptpCmd2={PTPMOVLXYZMode, 245.553, 46.959, 50, 10.827};
                    // 物体を拾う時の位置
                    PTPCmd ptpCmd3={PTPMOVLXYZMode, 245.553, 46.959, 14.125, 10.827};
                    // 物体を拾い，ロボットアームBが上に上がる位置
                    PTPCmd ptpCmd4={PTPMOVLXYZMode, 245.553, 46.959, 50, 10.827};
                    // 物体を落とす位置
                    PTPCmd ptpCmd5={PTPMOVLXYZMode, 155.559, -142.074, 90, -42.406};
                    // 初期位置
                    PTPCmd ptpCmd6={PTPMOVLXYZMode, 200, 0, 70, 0};

                    // 物体を拾う動作の開始時間を計測する
                    start_time = time(NULL);

                    // 動作を開始する
                    SetQueuedCmdStartExec();
                    // ロボットアームBに設定した変数を反映させる
                    SetPTPJointParams(&ptpJointParams, isQueued, &queuedCmdIndex);
                    SetPTPCommonParams(&ptpCommonParams, isQueued, &queuedCmdIndex);
                    // ロボットアームBを指定した位置に動かせる
                    // ロボットアームBを初期位置に移動させる
                    SetPTPCmd(&ptpCmd1, isQueued, &queuedCmdIndex);
                    // エアポンプを起動させる
                    SetEndEffectorSuctionCup(1, 1, isQueued, &queuedCmdIndex);
                    // ロボットアームBを物体の真上に移動させる
                    SetPTPCmd(&ptpCmd2, isQueued, &queuedCmdIndex);
                    // ロボットアームBを物体の位置に移動させ，物体を拾わせる
                    SetPTPCmd(&ptpCmd3, isQueued, &queuedCmdIndex);
                    // ロボットアームBを上に上げる
                    SetPTPCmd(&ptpCmd4, isQueued, &queuedCmdIndex);
                    // ロボットアームBを物体を落とす位置に移動させる
                    SetPTPCmd(&ptpCmd5, isQueued, &queuedCmdIndex);
                    // エアポンプを停止し，物体を落とす
                    SetEndEffectorSuctionCup(0, 0, isQueued, &queuedCmdIndex);
                    // ロボットアームBを初期位置に移動させる
                    SetPTPCmd(&ptpCmd6, isQueued, &queuedCmdIndex);

                    // 上記の物体を拾う動作をロボットアームBのキューに溜まって，順番ずつ実行する
                    while (executedCmdIndex < queuedCmdIndex)
                    {
                        GetQueuedCmdCurrentIndex(&executedCmdIndex);
                        // ロボットアームAがコマンドを実行する際の位置を取得する
                        GetPose(&pose);
                    }
                    // キューのコマンドを実行終わると，停止する
                    SetQueuedCmdStopExec();

                    // 物体を拾う動作の終了時間を計測する
                    end_time = time(NULL);
                    // 物体を拾う動作が掛かった時間を出力する
                    printf("移動時間:%lf\n", difftime(end_time, start_time));
                    // 拾う動作が終わったら，コンベアを起動する
                    eMotor.isEnabled = true;
                    SetQueuedCmdStartExec();
                    SetEMotor(&eMotor, isQueued, &queuedCmdIndex);
                    SetQueuedCmdStopExec();
                    printf("コンベアの起動\n");
                }
                // 物体実際の位置がptpCmd3変数で設定した物体を拾う位置とずれて，拾えない時，
                // センサBは物体が検知範囲から離れるまで待つ
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

    // EtherCATプロセスデータを処理するスレッド
    pthread_create(&thread1, NULL, (void *)main_run, NULL);
    // ロボットアームが物体を拾うなどの操作を処理するスレッド
    pthread_create(&thread2, NULL, (void *)easy_system, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}

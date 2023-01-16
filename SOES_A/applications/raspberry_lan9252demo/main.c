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
uint64_t queuedCmdIndex;
uint64_t executedCmdIndex = 0;
uint64_t currentIndex;
InfraredPort infraredPort = IF_PORT_GP4;
uint8_t value = false;
time_t start_time, end_time;
Pose pose;
bool blue_local;

// 出力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_get_inputs (void)
{
    // SLAVE Aのシステム起動処理
    if (Obj.Buttons.System_Start == true && Obj.States.System == false)
    {
        // ロボットアームAとの接続
        if ((ret = ConnectDobot(NULL, 115200, NULL, NULL, NULL)) != DobotConnect_NoError) 
        {
            printf("connect error\n");
        }
        printf("接続開始\n");
        // 変数の処理化
        blue_local = false;
        printf("Connection Start blue local: %d \n", blue_local);
        for(int i = 0; i <= current_index; i++)
        {
            SeqTable[i] = 0;
        }
        current_index = 1;
        local_index = 0;
        // センサAの起動
        SetInfraredSensor(1, infraredPort, 1);
        value = false;
        // SLAVE Aのシステム状態をTrueにする
        Obj.States.System = true;
    }

    // SLAVE Aのホーミング処理
    if (Obj.Buttons.Homing == true  && Obj.States.System == true && Obj.States.Homing == false)
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

    // SLAVE Aのシステム停止処理
    if (Obj.Buttons.System_Stop == true && Obj.States.System == true)
    {
        // センサAの停止
        SetInfraredSensor(0, infraredPort, 1);
        printf("Disconnect Dobot, Bye!\n");
        // ロボットアームAとの切断
        DisconnectDobot();
        // ロボットアーム位置の初期化
        pose.x = 0;
        pose.y = 0;
        pose.z = 0;
        pose.r = 0;
        // SLAVE Aのシステム状態をFalseにする
        Obj.States.System = false;
    }
}

// 入力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_set_outputs (void)
{
    // ロボットアーム位置と青い物体を検知した際のフラグの出力
    Obj.Positions.Position_X = pose.x;
    Obj.Positions.Position_Y = pose.y;
    Obj.Positions.Position_Z = pose.z;
    Obj.Positions.Position_R = pose.r;
    Obj.States.Blue_Local = blue_local;
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

// ロボットアームAが青い物体を拾う操作
void easy_system()
{
    while(1)
    {
        // SLAVE Aのシステム状態を確認する
        if (Obj.States.System == true)
        {
            // ロボットアームAの位置情報を取得する
            GetPose(&pose);
            // ホーミングが終わると，ホーミング状態をFalseにする
            GetQueuedCmdCurrentIndex(&executedCmdIndex);
            if(executedCmdIndex >= queuedCmdIndex && Obj.States.Homing == true) Obj.States.Homing = false;
            // 物体がカメラの下に通ったら，その際のシーケンス番号と色番号をSeqTableに格納する
            if(current_index == Obj.Counters.Sequence_Number)
            {
                // Red:1; Green:2; Blue:3
                if(Obj.Counters.Red == true) SeqTable[current_index] = 1;
                if(Obj.Counters.Green == true) SeqTable[current_index] = 2;
                if(Obj.Counters.Blue == true) SeqTable[current_index] = 3;
                printf("シーケンス番号：%d, 色：%d \n", current_index, SeqTable[current_index]);
                current_index++;
            }
            // 赤の物体がロボットアームAの所に通らないため，カウントをインクリメントする
            if(SeqTable[local_index + 1] == 1) local_index++;
            // 次に来る物体が緑の場合
            if(SeqTable[local_index + 1] == 2)
            {
                if (value == false) GetInfraredSensor(infraredPort, &value);
                else 
                {
                    if (local_index < Obj.Counters.Sequence_Number) local_index++;
                    while(1)
                    {
                        GetInfraredSensor(infraredPort, &value);
                        if (value == false) break;
                    }
                }
            }
            if(SeqTable[local_index + 1] == 3)
            {
                // 物体が来るまで，センサーBがずっと検知し続ける
                if (value == false) GetInfraredSensor(infraredPort, &value);
                else if (Obj.States.Homing == false)    // ホーミング中には吸着動作をしない
                {
                    // カメラの下に通らずまたは手などを検知した際にカウントさせないようにする
                    if (local_index < Obj.Counters.Sequence_Number) local_index++;
                    printf("センサーBが検知した物体の個数: %d\n", local_index);
                    printf("%d個目に青色物体を検知しました\n",SeqTable[local_index]);
                    blue_local = true;                  // フラグをtrueにして，コンベアを停止
                    // アームの速度と加速度:vx,vy,vz,vr,ax,ay,az,ar
                    PTPJointParams ptpJointParams={{200, 200, 200, 200}, {200, 200, 200, 200}};
                    // 速度比率と加速度比率
                    PTPCommonParams ptpCommonParams={100, 100};
                    // 移動先の位置
                    PTPCmd ptpCmd1={PTPMOVLXYZMode, 200, 0, 70, 0};
                    PTPCmd ptpCmd2={PTPMOVLXYZMode, 232.316, 203.293, 50, 41.188};
                    PTPCmd ptpCmd3={PTPMOVLXYZMode, 232.316, 203.293, 12.293, 41.188};
                    PTPCmd ptpCmd4={PTPMOVLXYZMode, 232.316, 203.293, 50, 41.188};
                    PTPCmd ptpCmd5={PTPMOVLXYZMode, 179.063, -67.946, 57.675, -20.779};
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
                        GetQueuedCmdCurrentIndex(&executedCmdIndex);
                        GetPose(&pose);
                    }
                    SetQueuedCmdStopExec();

                    end_time = time(NULL);
                    blue_local = false;         // 吸着が終わったら，コンベアを起動する
                    printf("移動時間:%lf\n", difftime(end_time, start_time));
                    // 吸着できないまたは取り扱わない物体がセンサーの検知範囲から離れるまで待つ
                    while(1)
                    {
                        GetInfraredSensor(infraredPort, &value);
                        if (value == false) break;
                    }
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
    // ロボットアームが物体を拾う操作を処理するスレッド
    pthread_create(&thread2, NULL, (void *)easy_system, NULL);

    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
    return 0;
}

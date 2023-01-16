#include <stdio.h>
#include "ecat_slv.h"
#include "utypes.h"
#include <bcm2835.h>

/* Application variables */
_Objects    Obj;
int flag = 0;
FILE *file;

// 出力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_get_inputs (void)
{
   // EtherCATマスタから操作しないため，なし
}

// 入力変数オブジェクトとマッピングしている変数を用いて，処理を行う
void cb_set_outputs (void)
{

   file = fopen("flag.dat", "r");
    flag = fgetc(file);
    printf("%d",flag);
    switch (flag){
       case 48:
         Obj.Colors.Red = 0;
         Obj.Colors.Green = 0;
         Obj.Colors.Blue = 0;
         break;
       case 49:
         Obj.Colors.Red = 1;
         Obj.Colors.Green = 0;
         Obj.Colors.Blue = 0;
         break;
       case 50:
         Obj.Colors.Red = 0;
         Obj.Colors.Green = 1;
         Obj.Colors.Blue = 0;
         break;
       case 51:
         Obj.Colors.Red = 0;
         Obj.Colors.Green = 0;
         Obj.Colors.Blue = 1;
         break;
       case 52:
         Obj.Colors.Red = 1;
         Obj.Colors.Green = 1;
         Obj.Colors.Blue = 0;
         break;
       case 53:
         Obj.Colors.Red = 1;
         Obj.Colors.Green = 0;
         Obj.Colors.Blue = 1;
         break;
       case 54:
         Obj.Colors.Red = 0;
         Obj.Colors.Green = 1;
         Obj.Colors.Blue = 1;
         break;
       case 55:
         Obj.Colors.Red = 1;
         Obj.Colors.Green = 1;
         Obj.Colors.Blue = 1;
         break;

       default:
         printf("error");
         break; 
 }
	fclose(file);
 
}


int main_run (void * arg)
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

   printf ("Hello Main2\n");
   bcm2835_init();
   ecat_slv_init (&config);
   while (1)
   {
      ecat_slv();
   }

   return 0;
}

int main (void)
{
   printf ("Hello Main\n");
   main_run (NULL);
   return 0;
}

# -*- coding: utf-8 -*-
import cv2
import numpy as np
import mmap

def red_detect(img):
    # HSV色空間に変換
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # 赤色のHSVの値域1
    hsv_min = np.array([0,127,0])
    hsv_max = np.array([30,255,255])
    mask1 = cv2.inRange(hsv, hsv_min, hsv_max)

    # 赤色のHSVの値域2
    hsv_min = np.array([150,127,0])
    hsv_max = np.array([179,255,255])
    mask2 = cv2.inRange(hsv, hsv_min, hsv_max)
    
    return mask1 + mask2

def green_detect(img):
    # HSV色空間に変換
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # 赤色のHSVの値域1
    hsv_min = np.array([45,64,0])
    hsv_max = np.array([75,255,255])
    mask = cv2.inRange(hsv, hsv_min, hsv_max)
    
    return mask

def blue_detect(img):
    # HSV色空間に変換
    hsv = cv2.cvtColor(img, cv2.COLOR_BGR2HSV)

    # 赤色のHSVの値域1
    hsv_min = np.array([90,100,0])
    hsv_max = np.array([150,255,255])
    mask = cv2.inRange(hsv, hsv_min, hsv_max)
    
    return mask


def main():
    # カメラのキャプチャ
    cap = cv2.VideoCapture(0)
    last_flag = 0
    send_flag = 0
    
    while(cap.isOpened()):
        # フレームを取得
        ret, frame = cap.read()
        frame = img_median=cv2.medianBlur(frame,5)

        # 赤色検出
        mask = red_detect(frame)
        mask_green = green_detect(frame)
        mask_blue = blue_detect(frame)
        
        cnts1, hierarchy1 = cv2.findContours(mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
        cnts2, hierarchy2 = cv2.findContours(mask_green, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)
        cnts3, hierarchy3 = cv2.findContours(mask_blue, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_NONE)

        if len(cnts1) > 0:
            red_flag = 1
        else:
            red_flag = 0
            
        if len(cnts2) > 0:
            green_flag = 1
        else:
            green_flag = 0
        
        if len(cnts3) > 0:
            blue_flag = 1 
        else:
            blue_flag = 0
        if red_flag == 0 and green_flag == 0 and blue_flag == 0:
            send_flag = 0
        if red_flag == 1 and green_flag == 0 and blue_flag == 0:
            send_flag = 1
        if red_flag == 0 and green_flag == 1 and blue_flag == 0:
            send_flag = 2
        if red_flag == 0 and green_flag == 0 and blue_flag == 1:
            send_flag = 3
        if send_flag != last_flag:
            if send_flag == 1:
                print("red detected")
            if send_flag == 2:
                print("green detected")
            if send_flag == 3:
                print("blue detected")
        last_flag = send_flag
             
             
        # 結果表示
        #cv2.imshow("Frame", frame)
        #cv2.imshow("Mask", mask)
        #cv2.imshow("Mask1", mask_blue)
        #cv2.imshow("Mask2", mask_green)
        with open("flag.dat","w") as f:
            f.write(str(send_flag))
            f.close()
            
                


        # qキーが押されたら途中終了
        if cv2.waitKey(25) & 0xFF == ord('q'):
            break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()

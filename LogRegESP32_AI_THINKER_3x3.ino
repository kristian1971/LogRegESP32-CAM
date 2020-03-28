#include "esp_camera.h"
//#include <math.h>
#define DEBUG

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
// In the plastic box
//#define CAMERA_MODEL_M5STACK_WIDE
// Naked one
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
const int big_areas=3;
const float e=2.718281828459;
int q,row,column,counter,temp,divide_by;
float final_value=0,output100,potencija100;
float x_all[big_areas*big_areas];
float weights[big_areas*big_areas]={-0.83959969,0.85408325,-0.37839335,0.76802856,-0.88139323,0.89086524,-0.74920654,0.79070805,-0.3996645};
uint8_t *p_all;
static double time1,time2,time3,Total_time1,Total_time2;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 2;
  
  } else {
    config.frame_size = FRAMESIZE_QVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_SVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

divide_by=255*(240/big_areas)*(240/big_areas);
}

void loop() {
  time1=micros();
  for(q=0;q<big_areas*big_areas;q++)
  {
    x_all[q]=0;
  }
  
  //x1=x2=x3=x4=x5=x6=x7=x8=x9=0;
  final_value=0;
  potencija100=0;
  counter=0;
  camera_fb_t *fb = esp_camera_fb_get();
  if ( fb ) {

    esp_camera_fb_return(fb);
    time2=micros();

    p_all=fb->buf;
      for(row=0;row<240;row++)
      {
        counter=counter+40;
        for(column=0;column<240;column++)
        {
              temp=(int)((row*big_areas/240)*big_areas+(column*big_areas/240));
              x_all[temp]=x_all[temp]+*(p_all+counter);
              counter=counter+1;             
        }
        counter=counter+40;
      }
      #ifdef DEBUG
      Serial.printf("COUNTER %d\n", counter);
      #endif
      
      for(counter=0;counter<big_areas*big_areas;counter++)
      {
        potencija100=potencija100+(x_all[counter]/divide_by)*weights[counter];
        final_value=final_value+x_all[counter]*weights[counter]/divide_by;
      }
      //-0.18468461+
      output100=pow(e,(potencija100))/(1+pow(e,(potencija100)));
      time3=micros();
      Total_time1=time2-time1;
      Total_time2=time3-time2;
      Serial.printf("FINAL %f\n", output100);     
      Serial.printf("TIME12 %f\n", Total_time1);
      Serial.printf("TIME23 %f\n", Total_time2);


      #ifdef DEBUG
      Serial.printf("x1:x2:x3: %f %f %f\n", x_all[0]/divide_by, x_all[1]/divide_by, x_all[2]/divide_by);
      Serial.printf("x4:x5:x6: %f %f %f\n", x_all[3]/divide_by, x_all[4]/divide_by, x_all[5]/divide_by);
      Serial.printf("x7:x8:x9: %f %f %f\n", x_all[6]/divide_by, x_all[7]/divide_by, x_all[8]/divide_by);
      #endif
    
  }

  delay(1000); 
}

void setup() {

  bool WifiConnected=false;

  ledcSetup(10, 5000/*freq*/, 10 /*resolution*/);
  ledcAttachPin(32, 10);
  analogReadResolution(10);
  ledcWrite(10,768);

  

  Serial.begin(115200); /* prepare for possible serial debug */

  client.setServer(MQTT_SERVER, MQTT_PORT);
  
  Serial.print("Connecting to ");
  Serial.println(ssid);

  while(!WifiConnected)
  {
    WiFi.begin(ssid, password);
    delay(1000);
    Serial.print(".");
    if(WiFi.status() == WL_CONNECTED)
    {
      WifiConnected=true;
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  lv_init();

  #if USE_LV_LOG != 0
    lv_log_register_print(my_print); /* register print function for debugging */
  #endif

  tft.begin(); /* TFT init */
  tft.setRotation(3);

  uint16_t calData[5] = { 275, 3620, 264, 3532, 1 };
  tft.setTouch(calData);

  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

  /*Initialize the display*/
  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.hor_res = screenWidth;
  disp_drv.ver_res = screenHeight;
  disp_drv.flush_cb = my_disp_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

  lv_indev_drv_t indev_drv;
  lv_indev_drv_init(&indev_drv);             /*Descriptor of a input device driver*/
  indev_drv.type = LV_INDEV_TYPE_POINTER;    /*Touch pad is a pointer-like device*/
  indev_drv.read_cb = my_touchpad_read;      /*Set your driver function*/
  lv_indev_drv_register(&indev_drv);         /*Finally register the driver*/


  /*Initialize the touch pad*/
  //lv_indev_drv_t indev_drv;
  //lv_indev_drv_init(&indev_drv);
  //indev_drv.type = LV_INDEV_TYPE_ENCODER;
  //indev_drv.read_cb = read_encoder;
  //lv_indev_drv_register(&indev_drv);

  /*Initialize the graphics library's tick*/
  tick.attach_ms(LVGL_TICK_PERIOD, lv_tick_handler);

  //Set the theme..
 // lv_theme_t * th = lv_theme_night_init(210, NULL);     //Set a HUE value and a Font for the Night Theme
 // lv_theme_set_current(th);

  lv_obj_t * scr = lv_cont_create(NULL, NULL);
  lv_disp_load_scr(scr);

  //lv_obj_t * tv = lv_tabview_create(scr, NULL);
  //lv_obj_set_size(tv, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_t *tabview;
    tabview = lv_tabview_create(lv_scr_act(), NULL);
  /*Add 3 tabs (the tabs are page (lv_page) and can be scrolled*/
    lv_obj_t *tab1 = lv_tabview_add_tab(tabview, "CO2");
    lv_obj_t *tab2 = lv_tabview_add_tab(tabview, "Pressure");
    lv_obj_t *tab3 = lv_tabview_add_tab(tabview, "Status");


    /*Add content to the tabs*/
    /*lv_obj_t * label = lv_label_create(tab1, NULL);
    lv_label_set_text(label, "This the first tab\n\n"
                             "If the content\n"
                             "of a tab\n"
                             "become too long\n"
                             "the it \n"
                             "automatically\n"
                             "become\n"
                             "scrollable.");

    label = lv_label_create(tab2, NULL);
    lv_label_set_text(label, "Second tab");

    label = lv_label_create(tab3, NULL);
    lv_label_set_text(label, "Third tab");*/

    //Chart - CO2
    chrt_co2 = lv_chart_create(tab1, NULL);
    lv_obj_set_size(chrt_co2, 460, 250);
    lv_obj_align(chrt_co2, NULL, LV_ALIGN_CENTER, 0, -5);
    lv_chart_set_type(chrt_co2,  LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_chart_set_series_opa(chrt_co2, LV_OPA_70);                            /*Opacity of the data series*/
    lv_chart_set_series_width(chrt_co2, 4);                                  /*Line width and point radious*/
    lv_chart_set_point_count(chrt_co2, 100);
    lv_chart_set_range(chrt_co2, 400, 1200);
  
    lv_chart_set_update_mode(chrt_co2,LV_CHART_UPDATE_MODE_SHIFT);
  
    /*Add data series*/
    ser_co2_1 = lv_chart_add_series(chrt_co2, LV_COLOR_RED);
    //ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);

    //Chart - Pressure
    chrt_press = lv_chart_create(tab2, NULL);
    lv_obj_set_size(chrt_press, 460, 250);
    lv_obj_align(chrt_press, NULL, LV_ALIGN_CENTER, 0, -5);
    lv_chart_set_type(chrt_press,  LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_chart_set_series_opa(chrt_press, LV_OPA_70);                            /*Opacity of the data series*/
    lv_chart_set_series_width(chrt_press, 4);                                  /*Line width and point radious*/
    lv_chart_set_point_count(chrt_press, 100);
    lv_chart_set_range(chrt_press, 650, 750);
  
    lv_chart_set_update_mode(chrt_press,LV_CHART_UPDATE_MODE_SHIFT);
  
    /*Add data series*/
    ser_press_1 = lv_chart_add_series(chrt_press, LV_COLOR_RED);
    //ser2 = lv_chart_add_series(chart, LV_COLOR_GREEN);
    //sensor labels
    co2_label = lv_label_create(chrt_co2, NULL);
    lv_obj_align(co2_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 5);
    
    pressure_label = lv_label_create(chrt_press, NULL);
    lv_obj_align(pressure_label, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 5);
    
    tvoc_label = lv_label_create(chrt_co2, NULL);
    lv_obj_align(tvoc_label, NULL, LV_ALIGN_IN_TOP_MID, 0, 5);


    //sensors
    Serial.println("CJMCU-8128 CCS811 + HDC1080 + BMP280 Example");
    Wire.begin();
  
    myBME280.settings.commInterface = I2C_MODE;
    myBME280.settings.I2CAddress = 0x76;
    myBME280.settings.runMode = 3; //Normal mode
    myBME280.settings.tStandby = 0;
    myBME280.settings.filter = 4;
    myBME280.settings.tempOverSample = 5;
    myBME280.settings.pressOverSample = 5;
    myBME280.settings.humidOverSample = 5;
  
    myBME280.begin();
    myHDC1080.begin(0x40);
  
    //It is recommended to check return status on .begin(), but it is not required.
  
    if (myCCS811.begin() == false)
    {
      Serial.print("CCS811 error! Please check wiring.");
      
    }

    Serial.print("Manufacturer ID=0x");
    Serial.println(myHDC1080.readManufacturerId(), HEX); // 0x5449 ID of Texas Instruments
    Serial.print("Device ID=0x");
    Serial.println(myHDC1080.readDeviceId(), HEX); // 0x1050 ID of the device

  
}

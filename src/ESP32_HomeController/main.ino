void mqtt_send(char* topic, char* buf)
{
    String s;
    s = String(MQTT_PREFIX)+String(topic);
    Serial.println(s);
    client.publish(s.c_str(), buf,true);
}

void loop() {
  char buf[16];
  static int i=0,seconds=0,minutes=0;
  float press_=0;
  static float last_co2=0,last_tvoc=0;
  static float sht10_temp_c;
  static float sht10_humidity;
  int light_level=0;
  
  lv_task_handler();
  delay(5);

  i=i+5;
  if(i>1000) //every second
  {

    i=0;
    seconds++;
    if(!client.connected()){
      Serial.println("MQTT isn't connected. Trying to reconnect...");
      reconnect();
    }
    client.loop();

    //Read SHT10
    // Read values from the sensor
    sht10_temp_c = sht1x.readTemperatureC();
    sht10_humidity = sht1x.readHumidity();
    
    // Print the values to the serial port
    Serial.print("Temperature: ");
    Serial.print(sht10_temp_c, DEC);
    Serial.print("C ");
    Serial.print(" Humidity: ");
    Serial.print(sht10_humidity);
    Serial.println("%");


    
    //Read HDC1080
    Serial.print("T=");
    Serial.print(myHDC1080.readTemperature());
    Serial.print("C, RH=");
    Serial.print(myHDC1080.readHumidity());
    Serial.println("%");
    
    press_=myBME280.readFloatPressure() * 0.00750062;
    snprintf(buf, 16, "Press: %f", press_);
    lv_label_set_text(pressure_label, buf);

    if (myCCS811.dataAvailable())
      {
        myCCS811.readAlgorithmResults();
        last_co2=myCCS811.getCO2();
        last_tvoc=myCCS811.getTVOC();
        //If so, have the sensor read and calculate the results.
        //Get them later
        
   
        Serial.print("CO2[");
        //Returns calculated CO2 reading
        Serial.print(last_co2);
        Serial.print("] tVOC[");
        //Returns calculated TVOC reading
        Serial.print(last_tvoc);
        Serial.print("] press[");
        Serial.print(press_);
        Serial.print("]");
        //Display the time since program start
        //Serial.print(millis());
        //Serial.print("]");
        Serial.println();
        snprintf(buf, 10, "CO2: %f", last_co2);
        lv_label_set_text(co2_label, buf);
        snprintf(buf, 10, "tVOC: %f", last_tvoc);
        lv_label_set_text(tvoc_label, buf);

        //LightLevel
        light_level=analogRead(35);
        Serial.print("Light level: ");
        Serial.println(light_level); 
        if(client.connected())
        {
          dtostrf(light_level, 5, 3, buf);
          mqtt_send("light_level",buf); 
        }
        
      }
    
    if(seconds>60)
    {
      minutes++;
      seconds=0;
      lv_chart_set_next(chrt_co2, ser_co2_1, last_co2);
      if(client.connected())
      {
      //Post MQTT data
      //SHT10
        dtostrf(sht10_temp_c, 5, 3, buf);
        mqtt_send("sht10_temp",buf);
        dtostrf(sht10_humidity, 5, 3, buf);
        mqtt_send("sht10_humidity",buf);
      //CO2
        dtostrf(last_co2, 5, 3, buf);
        mqtt_send("co2",buf);
        //Pressure
        dtostrf(press_, 5, 3, buf);
        mqtt_send("press",buf);
      }
    }

    if(minutes>60)
    {
      minutes=0;
      lv_chart_set_next(chrt_press, ser_press_1, press_);
    }
    
  }
}

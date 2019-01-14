int wifi=8;
int manual=7;
int load=13;
int load_ip=2;
boolean currentValue,previousValue,wifi_bool;

void setup() {
  pinMode(wifi,OUTPUT);
  pinMode(manual,INPUT_PULLUP);
  pinMode(load,OUTPUT);
  pinMode(load_ip,INPUT);
  Serial.begin(9600);
}

void loop() {
  currentValue=getValue(manual);
  //Serial.print(currentValue);
  delay(100);
  if(Serial.available())
  {
    int val=Serial.read()-'0';
    if(val==1)
    {
      if(wifi_bool==false)
      {
        digitalWrite(wifi,HIGH);
        wifi_bool=true;
      }
      else
      {
        digitalWrite(wifi,LOW);
        wifi_bool=false;
      }
      if(load_fun())
      Serial.print("1");
      else 
      Serial.print("0");
    }
  }
  else if(currentValue != previousValue)
  {
    previousValue=currentValue;
    if(load_fun())
    Serial.print("1");
    else 
    Serial.print("0");
  }
  if(digitalRead(load_ip)==HIGH)
  digitalWrite(load,HIGH);
  else 
  digitalWrite(load,LOW);
}
boolean getValue(int pin)
{
  if(digitalRead(pin)==LOW)
  return true;
  else
  return false;
}
boolean load_fun()
{
  if(currentValue && wifi_bool)
  {
      //Serial.println();
      //Serial.print(currentValue);
      //Serial.println(wifi_bool);
      return true;
  }
  else if(!currentValue && wifi_bool)
  {
    //Serial.println();
    //Serial.print(currentValue);
    //Serial.println(wifi_bool);
    return false;
  }
  else if(currentValue && !wifi_bool)
  {
    //Serial.println();
    //Serial.print(currentValue);
    //Serial.println(wifi_bool);
    return false;
  }
  else if(!currentValue && !wifi_bool)
  {
    //Serial.println();
    //Serial.print(currentValue);
    //Serial.println(wifi_bool);
    return true;
  }

}

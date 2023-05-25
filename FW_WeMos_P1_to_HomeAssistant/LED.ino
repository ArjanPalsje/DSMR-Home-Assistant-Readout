void pulseColor(int pin, uint16_t delayTime)
{
    for(int dutyCycle = 255; dutyCycle > 0; dutyCycle--){
    // changing the LED brightness with PWM
    analogWrite(pin, dutyCycle);
    delayMicroseconds(500);
  }
  delay(delayTime);
 
    for(int dutyCycle = 0; dutyCycle < 255; dutyCycle++){   
    // changing the LED brightness with PWM
    analogWrite(pin, dutyCycle);
    delayMicroseconds(500);
  }
  
  // decrease the LED brightness

  digitalWrite(pin,HIGH);
}

void quickFlash(int pin)
{
analogWrite(pin, 230);
delay(50);
digitalWrite(pin,HIGH);
}


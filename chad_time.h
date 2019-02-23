/* Contains functions created for nightime and 12-hr clock to separate my code from original as much as possible
    BCChad 10/27/2018
*/

bool twelvehr = true;    // Set to false for 24-hour clock
bool nighttime = true;   // Do not change - initialized to true, set later
int nowNight = 2100;     // integer value of 24-hour clock; change as desired
int nowDay = 700;        // integer value of 24-hour clock; change as desired


void turnOFF()  //  Switch display off
{
  timer_ticker.detach();     //  Detach or turn-off tickers to blank display
  display_ticker.detach();
  display.clearDisplay();
}

void turnON()  //  Switch display on
{
  display_ticker.attach(0.001, display_updater);
  timer_ticker.attach(0.1, number_updater);
}

void nightCheck()  // Sets/clears night flag, turns display on/off
{
  String str_checktime = getTimeAsString();
  //   case 0  - If System turns on at night, don't want display
  if (nighttime) turnOFF();

  // case 1 - daytime, becoming night
  if (!nighttime && (str_checktime.toInt() >= nowNight))  {
    turnOFF();
    nighttime = true;
  }
  // case 2 - nighttime - becoming day
  if (nighttime && (str_checktime.toInt() < nowNight) && (str_checktime.toInt() >= nowDay))
  {
    turnON();
    nighttime = false;
  }
}

String fixTime(String cur_Time) {      // Patch to change 24-hr to 12-hr, called if bool flag "twelvehr" set

  String hours = cur_Time.substring(0, 2);  // get hours substr
  cur_Time.remove(0, 2);           // remove it
  int hoursint = hours.toInt();   // change hours substr to integer
  hoursint = hoursint % 12;
  if (hoursint == 0) hoursint = 12;  // 12-hr clock doesn't have "00"
  hours = String(hoursint);  // Convert integer back to string
  if (hoursint < 10) hours = "0" + hours; //  add a leading zero if needed
  String twelveTime;
  return twelveTime = hours + cur_Time;   // Put time string back together
}

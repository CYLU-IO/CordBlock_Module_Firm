Button2 button = Button2(BUTTON_PIN);

void buttonInit() {
  button.setLongClickTime(2000);
  button.setDoubleClickTime(400);

  button.setTapHandler(btnTap);
  button.setLongClickHandler(longClick);
  button.setDoubleClickHandler(doubleClick);
  button.setTripleClickHandler(tripleClick);
}

void buttonLoop() {
  button.loop();
}

void btnTap(Button2& btn) {
  turnSwitch();
}

void longClick(Button2& btn) {
  test.overloading = (!test.overloading) ? true : false;
}

void doubleClick(Button2& btn) {

}

void tripleClick(Button2& btn) {
}

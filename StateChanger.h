
void joystickButtonClick() {

  volatile int currentTime = millis();
  if (currentTime - lastDebounceTime < debounceDelay) { 
    return;
  }

  lastDebounceTime = currentTime;
  shouldRedrawMenu = true;

  switch (systemState) {
    case SYSTEM_STATE_MENU:
      switch (selectedItem) {
        case 0:
          systemState = SYSTEM_STATE_GAME;
          break;
        case 1:
          systemState = SYSTEM_STATE_MENU_SETTINGS;
          isEditingSetting = false;
          break;
        case 2:
          systemState = SYSTEM_STATE_MENU_HIGHSCORES;
          break;
        case 3:
          systemState = SYSTEM_STATE_MENU_ABOUT;
          break;
      }
      selectedItem = 0;
      return;

    case SYSTEM_STATE_MENU_ABOUT:
      systemState = SYSTEM_STATE_MENU;
      return;

    case SYSTEM_STATE_MENU_HIGHSCORES:
      systemState = SYSTEM_STATE_MENU;
      return;

    case SYSTEM_STATE_MENU_SETTINGS:
      doSettingsAction();
      return;
    
  }
}

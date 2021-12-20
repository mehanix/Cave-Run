
void joystickButtonClick() {

  volatile int curTime = millis();
  if (curTime - lastDebounceTime < debounceDelay) { 
    return;
  }

  lastDebounceTime = curTime;
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

    case SYSTEM_STATE_NAME_EDIT:
      if (index == 5) {
        saveSettings();
        index = 0;
        systemState = SYSTEM_STATE_MENU_SETTINGS;
      }
      if (nameEditState == SYSTEM_STATE_NAME_EDIT_UNLOCKED) {
          nameEditState = SYSTEM_STATE_NAME_EDIT_LOCKED;
      } else {
          nameEditState = SYSTEM_STATE_NAME_EDIT_UNLOCKED;
        }
        shouldRedrawNameEdit = true;
        return;

    case SYSTEM_STATE_GAME:
      if (isPowerupAvailable == false) {
        return;
      }
      
      isPowerupAvailable = false;
      isPowerupActive = true;
      powerupStartTime = millis();
      return;
      
    case SYSTEM_STATE_GAME_END:
      setMatrixImage(happyMatrixSymbol);
      gameState = SYSTEM_STATE_GAME_SETUP;
      systemState = SYSTEM_STATE_MENU;
      return;
    
  }
}

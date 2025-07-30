/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2013 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_WINKEY_MINECRAFT_H_
#define XENIA_HID_WINKEY_MINECRAFT_H_

#include "xenia/hid/winkey/hookables/hookable_game.h"

namespace xe {
namespace hid {
namespace winkey {
static int32_t mouse_x_d;
static int32_t mouse_y_d;
class MinecraftGame : public HookableGame {
 public:
  enum class GameBuild { Unknown, TU4, TU75 };

  ~MinecraftGame() override;

  bool IsGameSupported(GameVersion title_version);
  bool DoHooks(uint32_t user_index, RawInputState& input_state,
               X_INPUT_STATE* out_state);
  std::string ChooseBinds();
  bool ModifierKeyHandler(uint32_t user_index, RawInputState& input_state,
                          X_INPUT_STATE* out_state);
  void WeaponSwitchHandler(uint32_t user_index, RawInputState& input_state,
                           X_INPUT_STATE* out_state, int weapon,
                           uint16_t buttons);

  void MidHookInit();

 private:
  GameBuild game_build_ = GameBuild::Unknown;
};

}  // namespace winkey
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_WINKEY_MINECRAFT_H_
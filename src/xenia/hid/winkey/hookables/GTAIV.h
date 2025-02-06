/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Copyright 2023 Ben Vanik. All rights reserved.                             *
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_WINKEY_GTAIV_H_
#define XENIA_HID_WINKEY_GTAIV_H_

#include <chrono>  // Include for chrono timing
#include "xenia/hid/winkey/hookables/hookable_game.h"

namespace xe {
namespace hid {
namespace winkey {

class GTAIVGame : public HookableGame {
 public:
  enum class GameBuild { Unknown, GTAIV_TU0 };

  ~GTAIVGame() override;

  bool IsGameSupported();

  bool DoHooks(uint32_t user_index, RawInputState& input_state,
               X_INPUT_STATE* out_state);

  void HandleCenterFlag(xe::be<uint8_t>* center_flag,
                        const RawInputState& input_state);

  std::string ChooseBinds();

  bool ModifierKeyHandler(uint32_t user_index, RawInputState& input_state,
                          X_INPUT_STATE* out_state);

  void WeaponSwitchHandler(uint32_t user_index, RawInputState& input_state,
                           X_INPUT_STATE* out_state, int weapon,
                           uint16_t buttons);

 private:
  GameBuild game_build_ = GameBuild::Unknown;
  std::chrono::steady_clock::time_point last_movement_time;
};

}  // namespace winkey
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_WINKEY_GTAIV_H_

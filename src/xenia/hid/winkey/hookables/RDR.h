/**
 ******************************************************************************
 * Xenia : Xbox 360 Emulator Research Project                                 *
 ******************************************************************************
 * Released under the BSD license - see LICENSE in the root for more details. *
 ******************************************************************************
 */

#ifndef XENIA_HID_WINKEY_REDEADREDEMPTION_H_
#define XENIA_HID_WINKEY_REDEADREDEMPTION_H_

#include "xenia/base/chrono.h"
#include "xenia/hid/winkey/hookables/hookable_game.h"

namespace xe {
namespace hid {
namespace winkey {

class RedDeadRedemptionGame : public HookableGame {
 public:
  enum class GameBuild {
    Unknown,
    RedDeadRedemption_GOTY_Disk1,
    RedDeadRedemption_GOTY_Disk2,
    RedDeadRedemption_Original_TU0
  };

  ~RedDeadRedemptionGame() override;

  bool IsGameSupported();

  float RadianstoDegree(float radians);
  float DegreetoRadians(float degree);

  bool DoHooks(uint32_t user_index, RawInputState& input_state,
               X_INPUT_STATE* out_state);

  std::string ChooseBinds();

  bool ModifierKeyHandler(uint32_t user_index, RawInputState& input_state,
                          X_INPUT_STATE* out_state);

 private:
  GameBuild game_build_ = GameBuild::Unknown;
  std::chrono::steady_clock::time_point last_movement_time_x_;
  std::chrono::steady_clock::time_point last_movement_time_y_;

  // New helper functions for internal use
  bool CheckPauseFlag();
  float CalculateFOVDivisor();
  void HandleCoverOrMountedCamera(RawInputState& input_state, float divisor);
  void AdjustCameraCover(RawInputState& input_state, float divisor);
  void AdjustMountedCamera(RawInputState& input_state, float divisor);
  void UpdateCoverCenter(RawInputState& input_state);
  void UpdateCameraPosition(RawInputState& input_state, float divisor,
                            xe::be<uint32_t>* base_address);
  void ApplyAutoCentering(RawInputState& input_state);
  void ApplyMountingCenter(RawInputState& input_state);
};

}  // namespace winkey
}  // namespace hid
}  // namespace xe

#endif  // XENIA_HID_WINKEY_REDEADREDEMPTION_H_

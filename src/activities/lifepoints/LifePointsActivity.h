#pragma once

#include "activities/Activity.h"
#include <cstdint>

class LifePointsActivity final : public Activity {
 private:
  static constexpr int START_LIFE = 40;
  static constexpr int MAX_LIFE = 999;
  static constexpr uint32_t LONG_PRESS_MS = 600;
  static constexpr uint32_t RESET_HOLD_MS = 5000;
  static constexpr uint32_t EXIT_SEQUENCE_MS = 3000;

  int life_[2] = {START_LIFE, START_LIFE};

  bool previous_[4] = {false, false, false, false};
  uint32_t pressedAt_[4] = {0, 0, 0, 0};
  bool resetTriggered_ = false;

  uint8_t exitProgress_ = 0;
  uint32_t exitStarted_ = 0;

  GfxRenderer::Orientation previousOrientation_;

  void changeLife(uint8_t player, int amount);
  void processButton(uint8_t index, bool pressed, uint32_t now);
  void processExitSequence(uint8_t index, uint32_t now);

  void drawDigit(int x, int y, int size, int digit, bool clockwise);
  void drawNumber(int value, int centerX, int centerY, bool clockwise);
  void drawControls();

 public:
  LifePointsActivity(GfxRenderer& renderer, MappedInputManager& input)
      : Activity("LifePoints", renderer, input),
        previousOrientation_(GfxRenderer::Orientation::Portrait) {}

  void onEnter() override;
  void onExit() override;
  void loop() override;
  void render(RenderLock&& lock) override;

  bool preventAutoSleep() override { return true; }
  bool handleHomeGesture() override { return true; }
};

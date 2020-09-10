#pragma once

#include "linear.h"

struct Renderer {
  r3::Matrix4f camFrustum;
  r3::Matrix4f camPose;
  r3::Vec3f intLoc;
  r3::Vec2d prevPos;
  r3::Vec2d diffPos;
  float theta = 0.0;
  bool intersect = false;
  int iterate = 0;

  float worldScale = 2.0;

  bool drawLeft = false;
  bool drawRight = false;
  r3::Posef leftPose;
  r3::Posef rightPose;

  virtual void Init() = 0;
  virtual void Draw() = 0;
  virtual void SetWindowSize(int w, int h) = 0;
  virtual void SetCursorPos(r3::Vec2d cursorPos) = 0;
  virtual void ResetSim() = 0;
  virtual void RayInWorld(r3::Vec3f& nIW3, r3::Vec3f& fIW3) = 0;
  virtual void RayInWorld(int w, int h, r3::Vec3f& nIW3, r3::Vec3f& fIW3) = 0;
  virtual void Intersect(r3::Vec3f nIW3, r3::Vec3f fIW3) = 0;
  virtual void Drag(r3::Vec3f newPos) = 0;
  virtual void ReScale(float scale) = 0;
};
Renderer* CreateRenderer();

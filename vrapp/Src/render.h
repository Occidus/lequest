#pragma once

#include "linear.h"

struct Renderer {
  r3::Matrix4f camFrustum;
  r3::Matrix4f camPose;
  r3::Posef headPoseInTracking;
  r3::Vec3f intLoc;
  r3::Vec2d prevPos;
  r3::Vec2d diffPos;
  float theta = 0.0;
  bool intersect = false;
  int iterate = 0;

  bool drawCenterAxis = false;
  r3::Posef leftPose;
  r3::Posef rightPose;

  virtual void Init() = 0;
  virtual void Draw() = 0;
  virtual void SetWindowSize(int w, int h) = 0;
  virtual void SetCursorPos(r3::Vec2d cursorPos) = 0;
  virtual void ResetSim() = 0;
  virtual void RayInWorld(r3::Vec3f& nIW3, r3::Vec3f& fIW3) = 0;
  virtual void RayInWorld(int w, int h, r3::Vec3f& nIW3, r3::Vec3f& fIW3) = 0;
  virtual bool BalisticProj(r3::Vec3f& pos, r3::Vec3f velo) = 0;
  virtual void Intersect(r3::Vec3f nIW3, r3::Vec3f fIW3) = 0;
  virtual void Drag(r3::Vec3f newPos) = 0;
  virtual void SetScale(float scale, r3::Vec3f scaleOriginInTracking) = 0;
  virtual void TeleportInApp(r3::Vec3f newPos) = 0;
};
Renderer* CreateRenderer();

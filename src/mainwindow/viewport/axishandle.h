#pragma once

#include "mainwindow/viewport/subhandle.h"

namespace omm
{

class AxisHandle : public SubHandle
{
public:
  struct Trait
  {
    Style base_style;
    Style hover_style;
    Style active_style;
    arma::vec2 direction;
    enum class Action { Scale, Translate };
    Action action;
  };

  AxisHandle(Handle& handle, Trait&& style);
  void draw(AbstractRenderer& renderer) const override;
  bool contains(const arma::vec2& point) const override;
  void mouse_move(const arma::vec2& delta, const arma::vec2& pos, const bool allow_hover) override;

private:
  Trait m_trait;
};

class XTranslateHandle : public AxisHandle
{
public:
  XTranslateHandle(Handle& handle);
private:
  static Trait trait();
};

class YTranslateHandle : public AxisHandle
{
public:
  YTranslateHandle(Handle& handle);
private:
  static Trait trait();
};

class XScaleHandle : public AxisHandle
{
public:
  XScaleHandle(Handle& handle);
private:
  static Trait trait();
};

class YScaleHandle : public AxisHandle
{
public:
  YScaleHandle(Handle& handle);
private:
  static Trait trait();
};

class ScaleHandle : public AxisHandle
{
public:
  ScaleHandle(Handle& handle);
private:
  static Trait trait();
};



}  // namespace omm

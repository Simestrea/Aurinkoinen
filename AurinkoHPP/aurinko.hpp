#pragma once

// ==========================================
// Aurinko 库总入口
//
// 一行引入所有基础设施 + 组件：
//   #include <aurinko/aurinko.hpp>
//
// 只需要其中一部分时，可以单独 include 具体文件，
// 例如只用到 Character：
//   #include <aurinko/components/character/character.hpp>
// ==========================================

// ---------- 基础设施 ----------
#include <aurinko/init.hpp>
#include <aurinko/coroutine.hpp>
#include <aurinko/easing.hpp>
#include <aurinko/draw.hpp>
#include <aurinko/input.hpp>
#include <aurinko/scene.hpp>
#include <aurinko/context.hpp>

// ---------- 基础组件 ----------
#include <aurinko/components/text.hpp>
#include <aurinko/components/ui_root.hpp>

// ---------- UI 组件 ----------
#include <aurinko/components/button.hpp>
#include <aurinko/components/panel.hpp>
#include <aurinko/components/progress_bar.hpp>
#include <aurinko/components/textbox.hpp>
#include <aurinko/components/slider.hpp>
#include <aurinko/components/table.hpp>

// ---------- 图表 ----------
#include <aurinko/components/plot/plot.hpp>

// ---------- 布局 ----------
#include <aurinko/layout.hpp>

// ---------- 角色 ----------
#include <aurinko/components/character/character.hpp>

// ---------- 样式系统 ----------
#include <aurinko/style/style.hpp>

#pragma once
// ==========================================
// aurinko 样式系统（CSS 风格）
//
// 用法:
//   aur::style::StyleSheet styles;
//   styles.Load("styles/default.style");
//
//   aur::Button btn("OK", 100, 100, 200, 40, 18, FONT);
//   styles.Apply(btn, "button primary");
//
// 文件格式 (.style):
//   [button]
//   bg = #2196F3
//   hover-bg = #1565C0
//   text = white
//
//   [button.primary]
//   bg = #E53935
// ==========================================

#include "value.hpp"
#include "stylesheet.hpp"
#include "apply.hpp"

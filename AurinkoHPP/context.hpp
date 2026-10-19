#pragma once
#include "scene.hpp"
#include "coroutine.hpp"
#include "init.hpp"
#include "character.hpp"
#include "text.hpp"

namespace aur {

// ==========================================
// Context：角色脚本的"依赖包"
//
// 把可安全暴露给脚本的全局单例打包在一起，
// 通过函数参数传给角色剧本，避免脚本直接碰
// aur::detail:: 里的内部单例。
//
// 用法（在角色 hpp 里）：
//   inline aur::Task AliceFun(aur::Character& c, aur::Context& ctx) {
//       auto* bob = ctx.scene.Find("Bob");
//       ctx.scene.Switch(1);
//       int vw = ctx.screen.GetVirtualWidth();
//   }
// ==========================================
struct Context {
    SceneManager&       scene;      // 场景管理器
    CoroutineScheduler& scheduler;  // 协程调度器（一般不用直接碰）
    VirtualScreen&      screen;     // 虚拟屏幕（查尺寸、坐标换算）
    TexturePool&        textures;   // 纹理池
    FontPool&           fonts;      // 字体池
};

// ==========================================
// 内部单例
//
// 用函数局部 static 而不是 inline 变量，避免跨 TU 的
// 初始化顺序问题（五个引用单例分布在不同头文件里，
// inline 变量的构造顺序未定义）。
// 角色脚本禁止直接写 aur::detail::context()
// ==========================================
namespace detail {
    inline Context& context() {
        static Context c{
            sceneManager,
            scheduler,
            virtualScreen,
            texturePool,
            fontPool
        };
        return c;
    }
}

}
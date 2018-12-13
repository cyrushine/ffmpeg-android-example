#### ffmpeglib
在 android 上运行 ffmpeg lib，演示了如何使用 ffmpeg api 进行视频播放


#### sdl_android_project
演示了如何在 android app 上运行 SDL2(2.0.8)，按照 sdl 文档的说明
1. 把 sdl/android-project 拷贝一份出来，然后把整个 sdl 文件夹放入 android-project/app/jni
2. 在 android-project/app/jni/src 文件夹下，新建 main.c，并修改 Android.mk
```
LOCAL_SRC_FILES := main.c
```
3. 写入约定好的 main 方法
```cpp
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

int SDL_main(int argc, char *argv[]) {
    for(;;) {
        sleep(1000 * 10);
    }
    return 0;
}
```
4. 在 jni 下执行 ndk-build 编译出 so 文件



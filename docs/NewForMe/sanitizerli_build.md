cmake \
-S /home/selviniah/Documents/ssh/EnterTheGungeonClone \
-B /home/selviniah/Documents/ssh/EnterTheGungeonClone/cmake-build-debug \
-DCMAKE_BUILD_TYPE=Debug \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer -g"

Ardından build ve çalıştır:

cmake --build /home/selviniah/Documents/ssh/EnterTheGungeonClone/cmake-build-debug --target ETG --parallel

cd /home/selviniah/Documents/ssh/EnterTheGungeonClone/cmake-build-debug/bin

ASAN_OPTIONS=abort_on_error=1:halt_on_error=1:symbolize=1 \
UBSAN_OPTIONS=print_stacktrace=1:halt_on_error=1 \
./ETG 2> sanitizer.log 



Sanitizer    Yakaladığı ana problem
━━━━━━━━━━━  ━━━━━━━━━━━━━━━━━━━━━━━━━━━━
ASan         Geçersiz bellek erişimi
───────────  ────────────────────────────
UBSan        Tanımsız C++ davranışı
───────────  ────────────────────────────
LSan         Memory leak
───────────  ────────────────────────────
TSan         Thread data race
───────────  ────────────────────────────
MSan         Initialize edilmemiş değer
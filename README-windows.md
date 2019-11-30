# Building on Windows

1. Install gcc and gmp. We give detailed instructions below for getting these through MSYS2, but you can get it any way you like.
    1. Install [MSYS2](https://www.msys2.org).
    2. Open the MSYS2 prompt by running the newly-created "MSYS2 MSYS" shortcut in your start menu.
    3. Inside the prompt, run `pacman -Syu`, then close the window when it prompts you to.
    4. Reopen the MSYS2 prompt and run:

            pacman -Syu
            pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-gmp mingw-w64-x86_64-cmake make

        (Yes, you should run `pacman -Syu` a second time.)

    5. Modify your `%Path%` to include the newly-installed software. You should include these two paths:

            C:\msys64\mingw64\bin
            C:\msys64\usr\bin
2. Build the ElectionGuard library.
    1. Open a command prompt and navigate to the directory with the vendor-sdk repo.
    2. Run the following commands:

            cmake -S . -B build -G "MSYS Makefiles" ..
            cmake --build build

    3. You should now have a `electionguard.a` or `electionguard.dll` (depending on the how cmake was configured).
3. (Optional) Build the simple example election driver.
    1. Open a command prompt and navigate to the directory with the vendor-sdk repo.
    2. Run the following commands:

            $env:CMAKE_PREFIX_PATH="C:\path\to\vendor-sdk\build\ElectionGuard"
            cmake -S examples/simple -B simple_build -G "MSYS Makefiles"
            cmake --build simple_build --target simple

    3. You should now have a `simple.exe` that simulates some random voters and generates election record artifacts.

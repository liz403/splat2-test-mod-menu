Basic mod menu for Splatoon 2. This doesn't bypass anticheat or do anything special, basically a runtime pchtxt.

Thanks Shadów and Haxxie for exlaunch and imgui-nvn. Uses [oead](https://github.com/shadowninja108/slide) implementation from Shadów.

Uses patches based on SHUT 5.5.0 public patches

**Building:**
1. Install DevKitPro and all Switch dependencies into your favorite Linux distro (or WSL)
2. Download this repo into a folder named `imgui-nvn`
3. ```bash
   #Example
   mkdir -p ~/Documents/Projects/splat2-test-mod-menu/imgui-nvn/
   git clone https://github.com/liz403/splat2-test-mod-menu.git ~/Documents/Projects/splat2-test-mod-menu/imgui-nvn/
   ```
4. Change whatever you want in the configs or code
5. Run `./exlaunch.sh clean && ./exlaunch.sh build`
